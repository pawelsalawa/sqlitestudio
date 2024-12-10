#ifndef ERDENTITY_H
#define ERDENTITY_H

#include "erditem.h"
#include "parser/ast/sqlitecreatetable.h"
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
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    private:
        struct Row
        {
            Row(QGraphicsItem* parent);
            ~Row();

            QGraphicsRectItem* topRect = nullptr;
            QGraphicsSimpleTextItem* text = nullptr;
            QGraphicsSimpleTextItem* datatype = nullptr;
            QGraphicsLineItem* bottomLine = nullptr;
            QList<QGraphicsItem*> icons;
            bool isHeader = false;

            qreal calcWidth(qreal iconColumn, qreal nameColumn) const;
            qreal height() const;
            qreal calcIconsWidth() const;
            qreal calcNameWidth() const;
            qreal updateLayout(qreal iconColumn, qreal nameColumn, qreal globalWidth, qreal globalY);
            void disableChildSelection();
        };

        void rebuild();
        void addColumn(SqliteCreateTable::Column* column, bool isLast);
        void addTableTitle();

        static constexpr qreal CELL_PADDING = 7.0;
        static constexpr qreal TEXT_GAP = 8.0;
        static constexpr qreal ICON_GAP = 4.0;

        QSharedPointer<SqliteCreateTable> tableModel;
        QList<ErdConnection*> connections;

        QList<Row*> rows;
};

#endif // ERDENTITY_H
