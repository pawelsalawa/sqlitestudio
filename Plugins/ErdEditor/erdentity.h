#ifndef ERDENTITY_H
#define ERDENTITY_H

#include "erditem.h"
#include <QGraphicsRectItem>

class SqliteCreateTable;
class QGridLayout;
class QFrame;
class QLabel;
class QLayoutItem;
class Icon;
class ErdConnection;
class QGraphicsSipmleTextItem;
class QGraphicsTextItem;
class QGraphicsLineItem;
class QGraphicsItem;

class ErdEntity : public QGraphicsRectItem, public ErdItem
{
    public:
        ErdEntity(SqliteCreateTable* tableModel);
        ErdEntity(QSharedPointer<SqliteCreateTable> tableModel);

        QSharedPointer<SqliteCreateTable> getTableModel() const;
        void modelUpdated();
        int rowIndexAt(const QPointF& point);
        QRectF rowRect(int rowIndex);
        bool isClickable();
        void updateConnectionsGeometry();
        void addConnection(ErdConnection* conn);
        void removeConnection(ErdConnection* conn);
        QList<ErdConnection*> getConnections() const;
        QString getTableName() const;

    private:
        struct Row
        {
            Row(QGraphicsItem* parent);
            ~Row();

            QGraphicsRectItem* topRect = nullptr;
            QGraphicsSimpleTextItem* text = nullptr;
            QGraphicsLineItem* bottomLine = nullptr;
            QList<QGraphicsItem*> icons;

            qreal calcWidth(qreal iconColumn) const;
            qreal height() const;
            qreal calcIconsWidth() const;
            qreal updateLayout(qreal iconColumn, qreal globalWidth, qreal globalY);
            void disableChildSelection();
        };

        void rebuild();
        void addColumn(const QString& text, bool isLast);
        void addTableTitle(const QString& text);

        static constexpr qreal CELL_PADDING = 6.0;
        static constexpr qreal TEXT_GAP = 5.0;
        static constexpr qreal ICON_GAP = 2.0;

        QSharedPointer<SqliteCreateTable> tableModel;
        QList<ErdConnection*> connections;

        QList<Row*> rows;
};

#endif // ERDENTITY_H
