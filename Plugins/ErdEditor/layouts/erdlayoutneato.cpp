#include "erdlayoutneato.h"
#include "scene/erdentity.h"
#include "scene/erdconnection.h"
#include <QRandomGenerator>

namespace
{
    struct NaetoParams
    {
        int iterations = 900;

        // Force-directed (neato-like)
        double repulsion = 80000.0;       // Coulomb
        double springK = 0.07;            // Hooke
        double idealEdgeLength = 240.0;   // px
        double gravity = 0.014;           // very low
        double damping = 0.85;
        double maxStep = 25.0;

        // Anti-overlap
        double overlapPadding = 60.0;
        double overlapPush = 0.65;

        // Annealing
        double temperature0 = 45.0;
        double temperatureMin = 0.6;

        // UI behavior
        bool pinSelected = true;
        unsigned int randomSeed = 0; // 0 = random
    };

    struct Edge
    {
        int a;
        int b;
    };

    static inline double lengthSafe(const QPointF& v)
    {
        return qSqrt(v.x()*v.x() + v.y()*v.y()) + 1e-9;
    }

    static inline QPointF clampLen(const QPointF& v, double maxLen)
    {
        const double L = lengthSafe(v);
        if (L <= maxLen) return v;
        return v * (maxLen / L);
    }

    static QList<Edge> collectEdges(const QList<ErdEntity*>& ents,
                                    const QHash<ErdEntity*, int>& index)
    {
        QSet<qulonglong> seen;
        QList<Edge> edges;

        for (auto* e : ents)
        {
            for (auto* c : e->getConnections())
            {
                if (!c || !c->isFinalized())
                    continue;

                auto* s = c->getStartEntity();
                auto* t = c->getEndEntity();
                if (!s || !t || s == t)
                    continue;

                if (!index.contains(s) || !index.contains(t))
                    continue;

                int a = index[s];
                int b = index[t];
                if (a > b) std::swap(a, b);

                qulonglong key = (qulonglong(a) << 32) | qulonglong(b);
                if (seen.contains(key))
                    continue;

                seen.insert(key);

                edges.push_back({a, b});
            }
        }
        return edges;
    }

    static double minEdgeLength(int a, int b,
                                const QVector<QRectF>& rect,
                                double padding)
    {
        const QRectF& ra = rect[a];
        const QRectF& rb = rect[b];

        const double wa = ra.width();
        const double wb = rb.width();
        const double ha = ra.height();
        const double hb = rb.height();

        const double da = qSqrt(wa*wa + ha*ha) * 0.5;
        const double db = qSqrt(wb*wb + hb*hb) * 0.5;

        return da + db + padding;
    }

    static void resolveOverlaps(QList<ErdEntity*>& component)
    {
        constexpr int MAX_PASSES = 30;
        constexpr qreal PUSH_STEP = 20.0;
        constexpr qreal MARGIN = 20.0;

        for (int pass = 0; pass < MAX_PASSES; ++pass)
        {
            bool anyOverlap = false;
            for (int i = 0; i < component.size(); ++i)
            {
                for (int j = i + 1; j < component.size(); ++j)
                {
                    ErdEntity* a = component[i];
                    ErdEntity* b = component[j];
                    QRectF ra = a->mapToScene(a->boundingRect()).boundingRect().adjusted(-MARGIN, -MARGIN, MARGIN, MARGIN);
                    QRectF rb = b->mapToScene(b->boundingRect()).boundingRect().adjusted(-MARGIN, -MARGIN, MARGIN, MARGIN);
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

    static QRectF computeBoundingBox(const QList<ErdEntity*>& entities)
    {
        QRectF bbox;
        for (ErdEntity* e : entities)
        {
            if (!e)
                continue;

            bbox |= e->mapToScene(e->boundingRect()).boundingRect();
        }
        return bbox;
    }
}

ErdLayoutNeato::ErdLayoutNeato()
{

}

void ErdLayoutNeato::arrange(QList<ErdEntity*> entities)
{
    NaetoParams params; // always use default params

    const int n = entities.size();
    if (n == 0)
        return;

    QHash<ErdEntity*, int> index;
    for (int i = 0; i < n; ++i)
        index.insert(entities[i], i);

    QList<Edge> edges = collectEdges(entities, index);
    QVector<QVector<bool>> hasEdge(n, QVector<bool>(n, false));
    for (const auto& e : edges)
    {
        hasEdge[e.a][e.b] = true;
        hasEdge[e.b][e.a] = true;
    }

    QRandomGenerator rng(params.randomSeed ? params.randomSeed : QRandomGenerator::global()->generate());

    QVector<QPointF> pos(n);
    QVector<QPointF> vel(n, QPointF(0,0));
    QVector<bool> pinned(n, false);

    // start: current or random
    for (int i = 0; i < n; ++i)
    {
        pos[i] = entities[i]->pos();
        if (params.pinSelected && entities[i]->isSelected())
            pinned[i] = true;
        else if (pos[i].isNull())
            pos[i] = QPointF(rng.bounded(-300,300),
                             rng.bounded(-300,300));
    }

    // Local entity rects
    QVector<QRectF> rect(n);
    for (int i = 0; i < n; ++i)
    {
        QRectF r = entities[i]->sceneBoundingRect();
        r.moveCenter(QPointF(0,0));
        rect[i] = r;
    }

    QVector<QPointF> force(n);
    double T = params.temperature0;
    auto cool = [&](int i)
    {
        double a = qPow(params.temperatureMin / params.temperature0,
                        1.0 / qMax(1, params.iterations));
        T = qMax(params.temperatureMin,
                 params.temperature0 * qPow(a, i));
    };

    // Iterations
    for (int it = 0; it < params.iterations; ++it)
    {
        force.fill(QPointF(0,0));

        // repulsion
        for (int i = 0; i < n; ++i)
        {
            for (int j = i+1; j < n; ++j)
            {
                double minDist = (rect[i].width() + rect[j].width()) * 0.5
                  + params.overlapPadding * 2.0;

                QPointF d = pos[i] - pos[j];
                double dist = lengthSafe(d);
                QPointF dir = d / dist;
                double f = params.repulsion / (dist * dist);
                if (dist < minDist)
                {
                    double extra = (minDist - dist) / minDist;
                    f *= (1.0 + 2.0 * extra);
                }

                if (!pinned[i])
                    force[i] += dir * f;

                if (!pinned[j])
                    force[j] -= dir * f;
            }
        }

        // springs
        for (const auto& e : edges)
        {
            QPointF d = pos[e.b] - pos[e.a];
            double dist = lengthSafe(d);
            QPointF dir = d / dist;
            double L = qMax(
                params.idealEdgeLength,
                minEdgeLength(e.a, e.b, rect, params.overlapPadding * 2.0)
            );
            double f = params.springK * (dist - L);
            if (!pinned[e.a])
                force[e.a] += dir * f;

            if (!pinned[e.b])
                force[e.b] -= dir * f;
        }

        // gravity
        QPointF center(0,0);
        for (int i = 0; i < n; ++i)
            center += pos[i];

        center /= double(n);
        for (int i = 0; i < n; ++i)
        {
            if (!pinned[i])
                force[i] += (center - pos[i]) * params.gravity;
        }

        // integration
        cool(it);
        for (int i = 0; i < n; ++i)
        {
            if (pinned[i])
                continue;

            vel[i] = vel[i] * params.damping + force[i];
            QPointF step = clampLen(vel[i], params.maxStep);
            step = clampLen(step, T);
            pos[i] += step;
        }

        // Anti-overlap
        for (int pass = 0; pass < 2; ++pass)
        {
            for (int i = 0; i < n; ++i)
            {
                QRectF ri = rect[i];
                ri.moveCenter(pos[i]);
                ri.adjust(-params.overlapPadding, -params.overlapPadding,
                          params.overlapPadding,  params.overlapPadding);

                for (int j = i+1; j < n; ++j)
                {
                    QRectF rj = rect[j];
                    rj.moveCenter(pos[j]);
                    rj.adjust(-params.overlapPadding, -params.overlapPadding,
                              params.overlapPadding,  params.overlapPadding);

                    if (!ri.intersects(rj))
                        continue;

                    QRectF is = ri.intersected(rj);
                    QPointF dir = pos[i] - pos[j];
                    if (dir.isNull())
                        dir = QPointF(rng.generateDouble()-0.5,
                                      rng.generateDouble()-0.5);

                    bool related = hasEdge[i][j];
                    double pushFactor = related ? 1.4 : 1.0;
                    if (is.width() < is.height())
                    {
                        double dx = (is.width()+1) * params.overlapPush * pushFactor;
                        dx *= (dir.x() >= 0 ? 1 : -1);
                        if (!pinned[i])
                            pos[i].rx() += dx;

                        if (!pinned[j])
                            pos[j].rx() -= dx;
                    }
                    else
                    {
                        double dy = (is.height()+1) * params.overlapPush * pushFactor;
                        dy *= (dir.y() >= 0 ? 1 : -1);
                        if (!pinned[i])
                            pos[i].ry() += dy;

                        if (!pinned[j])
                            pos[j].ry() -= dy;
                    }

                    ri = rect[i];
                    ri.moveCenter(pos[i]);
                }
            }
        }
    }

    // Boundig box in shifted positioning
    QRectF bb;
    bool first = true;
    for (int i = 0; i < n; ++i)
    {
        QRectF r = rect[i];
        r.moveCenter(pos[i]);
        if (first)
        {
            bb = r;
            first = false;
        }
        else
            bb = bb.united(r);
    }

    // Normalize & apply
    QPointF shift = -bb.topLeft() + QPointF(40,40);
    for (int i = 0; i < n; ++i)
    {
        pos[i] += shift;
        entities[i]->setPos(pos[i]);
    }
    resolveOverlaps(entities);

    for (auto* e : entities)
        e->updateConnectionsGeometry();
}
