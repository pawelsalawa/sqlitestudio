#include "erdlayoutfdp.h"
#include "scene/erdconnection.h"
#include "scene/erdentity.h"
#include <QHash>
#include <QQueue>
#include <QRandomGenerator>
#include <QSet>

namespace
{
    struct ErdFdpLayoutParams
    {
        int iterations = 600;           // typically 300..1200
        qreal idealEdgeLength = 260.0;
        qreal initialTemperature = 150.0;
        qreal temperatureDecay = 0.985; // 0.97..0.995
        qreal repulsion = 1.8;
        qreal attraction = 1.0;
        qreal gravity = 0.04;
        qreal overlapPenalty = 1.8;
        qreal jitter = 1e-3;            // little noise, to avoid symetry/zeros
        qreal maxStep = 30.0;
        qreal margin = 40.0;            // final margin for the whole setup
    };

    struct Node
    {
        ErdEntity* e = nullptr;
        QPointF pos;
        QPointF disp;
        QSizeF  size;
        qreal   mass = 1.0;
    };

    struct Edge
    {
        int a = -1;
        int b = -1;
    };

    static QPointF centerOf(const Node& n)
    {
        return n.pos + QPointF(n.size.width() * 0.5, n.size.height() * 0.5);
    }

    static qreal lengthSafe(const QPointF& v, qreal eps = 1e-9)
    {
        const qreal l2 = v.x()*v.x() + v.y()*v.y();
        if (l2 < eps)
            return 0.0;

        return std::sqrt(l2);
    }

    static QSizeF halfExtents(const Node& n)
    {
        return QSizeF(n.size.width() * 0.5, n.size.height() * 0.5);
    }

    static bool rectsOverlapApprox(const QPointF& ca, const QSizeF& ha,
                                   const QPointF& cb, const QSizeF& hb)
    {
        return (std::abs(ca.x() - cb.x()) < (ha.width()  + hb.width())) &&
               (std::abs(ca.y() - cb.y()) < (ha.height() + hb.height()));
    }

    static QPointF overlapPush(const QPointF& ca, const QSizeF& ha,
                               const QPointF& cb, const QSizeF& hb)
    {
        const qreal dx = ca.x() - cb.x();
        const qreal dy = ca.y() - cb.y();

        const qreal px = (ha.width()  + hb.width())  - std::abs(dx);
        const qreal py = (ha.height() + hb.height()) - std::abs(dy);

        if (px < py)
            return QPointF((dx >= 0 ? 1 : -1) * px, 0);
        else
            return QPointF(0, (dy >= 0 ? 1 : -1) * py);
    }

    static bool hasAnyEdge(const QList<ErdEntity*>& comp)
    {
        for (ErdEntity* e : comp) {
            for (ErdConnection* c : e->getConnections()) {
                if (c && c->isFinalized())
                    return true;
            }
        }
        return false;
    }

    static void layoutIsolatedComponent(QList<ErdEntity*>& comp)
    {
        constexpr int COLS = 4;
        constexpr qreal GAP_X = 100.0;
        constexpr qreal GAP_Y = 100.0;

        for (int i = 0; i < comp.size(); ++i) {
            int row = i / COLS;
            int col = i % COLS;
            comp[i]->setPos(
                QPointF(col * GAP_X, row * GAP_Y)
            );
        }
    }

    static QVector<Edge> collectEdges(const QList<ErdEntity*>& entities)
    {
        QHash<ErdEntity*, int> idx;
        idx.reserve(entities.size());
        for (int i = 0; i < entities.size(); ++i) idx.insert(entities[i], i);

        QSet<quint64> seen;
        QVector<Edge> edges;
        edges.reserve(entities.size() * 2);

        for (int i = 0; i < entities.size(); ++i)
        {
            const auto conns = entities[i]->getConnections();
            for (ErdConnection* c : conns)
            {
                if (!c || !c->isFinalized())
                    continue;

                ErdEntity* s = c->getStartEntity();
                ErdEntity* t = c->getEndEntity();
                if (!s || !t) continue;
                if (!idx.contains(s) || !idx.contains(t))
                    continue;

                int a = idx.value(s);
                int b = idx.value(t);
                if (a == b)
                    continue;

                const int lo = std::min(a,b);
                const int hi = std::max(a,b);
                const quint64 key = (quint64(lo) << 32) | quint64(hi);
                if (seen.contains(key))
                    continue;

                seen.insert(key);

                edges.push_back(Edge{lo, hi});
            }
        }
        return edges;
    }

    static QList<QList<ErdEntity*>> findConnectedComponents(const QList<ErdEntity*>& entities)
    {
        QSet<ErdEntity*> visited;
        QList<QList<ErdEntity*>> components;

        for (ErdEntity* e : entities)
        {
            if (!e || visited.contains(e))
                continue;

            QList<ErdEntity*> component;
            QQueue<ErdEntity*> queue;

            visited.insert(e);
            queue.enqueue(e);

            while (!queue.isEmpty())
            {
                ErdEntity* cur = queue.dequeue();
                component.append(cur);

                for (ErdConnection* c : cur->getConnections())
                {
                    if (!c || !c->isFinalized())
                        continue;

                    ErdEntity* other = (c->getStartEntity() == cur)
                            ? c->getEndEntity()
                            : c->getStartEntity();

                    if (other && !visited.contains(other))
                    {
                        visited.insert(other);
                        queue.enqueue(other);
                    }
                }
            }
            components.append(component);
        }
        return components;
    }

    static void layoutFdpLike(QList<ErdEntity*> entities, const ErdFdpLayoutParams& params)
    {
        if (entities.size() <= 1)
            return;

        QVector<Node> nodes;
        nodes.reserve(entities.size());

        // Initialize positions
        QRectF existingBounds;
        bool anyPlaced = false;
        for (ErdEntity* e : entities)
        {
            if (!e)
                continue;

            const QPointF pos = e->pos();
            const QSizeF  sz  = e->boundingRect().size();
            nodes.push_back(Node{e, pos, QPointF(0,0), sz, 1.0});
            if (!qFuzzyIsNull(pos.x()) || !qFuzzyIsNull(pos.y()))
            {
                existingBounds |= QRectF(pos, sz);
                anyPlaced = true;
            }
        }
        if (nodes.size() <= 1)
            return;

        const QVector<Edge> edges = collectEdges(entities);

        // If there are many of (0,0), put them on a circle
        if (!anyPlaced || existingBounds.isNull() || existingBounds.width() < 1.0)
        {
            const qreal R = params.idealEdgeLength * std::sqrt(nodes.size());
            const QPointF C(0, 0);
            for (int i = 0; i < nodes.size(); ++i)
            {
                const qreal ang = (2.0 * M_PI * i) / qreal(nodes.size());
                const qreal rx = R * std::cos(ang);
                const qreal ry = R * std::sin(ang);
                nodes[i].pos = C + QPointF(rx, ry);
            }
        }

        const qreal k = params.idealEdgeLength;
        qreal t = params.initialTemperature;

        QRandomGenerator* rng = QRandomGenerator::global();

        for (int it = 0; it < params.iterations; ++it)
        {
            // reset disp
            for (Node& n : nodes) n.disp = QPointF(0,0);

            // Repulsion
            for (int i = 0; i < nodes.size(); ++i)
            {
                for (int j = i + 1; j < nodes.size(); ++j)
                {
                    Node& a = nodes[i];
                    Node& b = nodes[j];

                    const QPointF ca = centerOf(a);
                    const QPointF cb = centerOf(b);
                    QPointF delta = ca - cb;

                    // Jitter
                    if (qFuzzyIsNull(delta.x()) && qFuzzyIsNull(delta.y()))
                    {
                        delta.setX((rng->generateDouble() - 0.5) * 2.0 * params.jitter);
                        delta.setY((rng->generateDouble() - 0.5) * 2.0 * params.jitter);
                    }

                    const qreal dist = std::max<qreal>(1e-3, lengthSafe(delta));
                    const QPointF dir = delta / dist;

                    qreal force = (k * k) / dist;
                    force *= params.repulsion;

                    const qreal sizeBoost = 0.5 * (a.size.width() + a.size.height() + b.size.width() + b.size.height()) / (2.0 * k);
                    force *= (1.0 + 0.15 * sizeBoost);

                    QPointF fvec = dir * force;

                    a.disp += fvec;
                    b.disp -= fvec;

                    // Overlap penalty
                    const QSizeF ha = halfExtents(a);
                    const QSizeF hb = halfExtents(b);
                    if (rectsOverlapApprox(ca, ha, cb, hb)) {
                        const QPointF push = overlapPush(ca, ha, cb, hb);
                        const QPointF pvec = push * params.overlapPenalty;
                        a.disp += pvec;
                        b.disp -= pvec;
                    }
                }
            }

            // Edge attraction
            for (const Edge& e : edges)
            {
                Node& a = nodes[e.a];
                Node& b = nodes[e.b];

                const QPointF ca = centerOf(a);
                const QPointF cb = centerOf(b);
                QPointF delta = ca - cb;

                if (qFuzzyIsNull(delta.x()) && qFuzzyIsNull(delta.y()))
                {
                    delta.setX((rng->generateDouble() - 0.5) * 2.0 * params.jitter);
                    delta.setY((rng->generateDouble() - 0.5) * 2.0 * params.jitter);
                }

                const qreal dist = std::max<qreal>(1e-3, lengthSafe(delta));
                const QPointF dir = delta / dist;

                qreal force = (dist * dist) / k;
                force *= params.attraction;

                const qreal stretch = (dist - params.idealEdgeLength) / std::max<qreal>(params.idealEdgeLength, 1.0);
                force *= (1.0 + 0.35 * std::abs(stretch));

                const QPointF fvec = dir * force;

                a.disp -= fvec;
                b.disp += fvec;
            }

            // Gravity
            QPointF centroid(0,0);
            for (const Node& n : nodes) centroid += centerOf(n);
            centroid /= qreal(nodes.size());

            for (Node& n : nodes)
            {
                const QPointF c = centerOf(n);
                const QPointF toCenter = centroid - c;
                n.disp += toCenter * params.gravity;
            }

            // Temperature & maxStep
            for (Node& n : nodes)
            {
                const qreal dispLen = lengthSafe(n.disp);
                if (dispLen <= 1e-9)
                    continue;

                const qreal step = std::min<qreal>(params.maxStep, std::min<qreal>(dispLen, t));
                QPointF delta = (n.disp / dispLen) * step;

                if (!qIsFinite(delta.x()) || !qIsFinite(delta.y()))
                    continue;

                n.pos += delta;
            }

            // Cooling
            t *= params.temperatureDecay;
            if (t < 0.5)
                break;
        }

        // Normalize
        QRectF bounds;
        for (const Node& n : nodes)
            bounds |= QRectF(n.pos, n.size);

        QPointF shift(0,0);
        shift.setX(params.margin - bounds.left());
        shift.setY(params.margin - bounds.top());

        for (Node& n : nodes)
            n.pos += shift;

        // Apply
        for (Node& n : nodes)
        {
            if (!n.e)
                continue;

            n.e->setPos(n.pos);
        }

        for (Node& n : nodes)
        {
            if (!n.e)
                continue;

            n.e->updateConnectionsGeometry();
        }
    }

    static void layoutSingleComponent(QList<ErdEntity*>& component)
    {
        if (component.size() <= 1)
            return;

        const qreal R = 300.0;
        for (int i = 0; i < component.size(); ++i)
        {
            const qreal a = 2.0 * M_PI * i / component.size();
            component[i]->setPos(QPointF(R * std::cos(a), R * std::sin(a)));
        }

        ErdFdpLayoutParams p;
        p.iterations = 600;
        p.idealEdgeLength = 260.0;
        p.repulsion = 1.0;
        p.attraction = 1.0;
        p.gravity = 0.03;

        layoutFdpLike(component, p);

        QRectF bbox = ErdLayout::computeBoundingBox(component);
        for (ErdEntity* e : component)
            e->setPos(e->pos() - bbox.topLeft());
    }

    static void packComponents(const QList<QList<ErdEntity*>>& components)
    {
        QPointF cursor(0, 0);
        qreal rowHeight = 0;

        constexpr qreal GAP = 200.0;
        constexpr qreal MAX_ROW_WIDTH = 2800.0;

        for (const QList<ErdEntity*>& comp : components)
        {
            QRectF bbox = ErdLayout::computeBoundingBox(comp);

            if (cursor.x() + bbox.width() > MAX_ROW_WIDTH)
            {
                cursor.setX(0);
                cursor.setY(cursor.y() + rowHeight + GAP);
                rowHeight = 0;
            }

            for (ErdEntity* e : comp)
                e->setPos(e->pos() + cursor);

            cursor.rx() += bbox.width() + GAP;
            rowHeight = std::max(rowHeight, bbox.height());
        }
    }

    static void resolveOverlaps(QList<ErdEntity*>& component)
    {
        constexpr int MAX_PASSES = 20;
        constexpr qreal PUSH_STEP = 8.0;

        for (int pass = 0; pass < MAX_PASSES; ++pass)
        {
            bool anyOverlap = false;
            for (int i = 0; i < component.size(); ++i)
            {
                for (int j = i + 1; j < component.size(); ++j)
                {
                    ErdEntity* a = component[i];
                    ErdEntity* b = component[j];
                    QRectF ra = a->mapToScene(a->boundingRect()).boundingRect();
                    QRectF rb = b->mapToScene(b->boundingRect()).boundingRect();
                    if (!ra.intersects(rb))
                        continue;

                    anyOverlap = true;
                    QRectF inter = ra.intersected(rb);
                    if (inter.width() < inter.height())
                    {
                        qreal dx = inter.width() / 2.0 + PUSH_STEP;
                        if (ra.center().x() < rb.center().x())
                            dx = -dx;

                        a->setPos(a->pos() + QPointF(dx, 0));
                        b->setPos(b->pos() - QPointF(dx, 0));
                    }
                    else
                    {
                        qreal dy = inter.height() / 2.0 + PUSH_STEP;
                        if (ra.center().y() < rb.center().y())
                            dy = -dy;

                        a->setPos(a->pos() + QPointF(0, dy));
                        b->setPos(b->pos() - QPointF(0, dy));
                    }
                }
            }

            if (!anyOverlap)
                break;
        }
    }
}

ErdLayoutFdp::ErdLayoutFdp()
{

}

void ErdLayoutFdp::arrange(QList<ErdEntity*> entities)
{
    if (entities.isEmpty())
        return;

    QList<QList<ErdEntity*>> components = findConnectedComponents(entities);

    for (QList<ErdEntity*>& comp : components)
    {
        if (comp.size() == 1 || !hasAnyEdge(comp))
        {
            layoutIsolatedComponent(comp);
        }
        else
        {
            layoutSingleComponent(comp);
            resolveOverlaps(comp);
        }
    }

    packComponents(components);

    for (ErdEntity* e : entities)
        e->updateConnectionsGeometry();
}
