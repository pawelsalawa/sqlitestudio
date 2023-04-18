#ifndef ERDENTITY_H
#define ERDENTITY_H

#include "erditem.h"

#include <QGraphicsProxyWidget>

class SqliteCreateTable;
class QGridLayout;
class QFrame;
class QLabel;
class QLayoutItem;
class Icon;
class ErdConnection;

class ErdEntity : public QGraphicsProxyWidget, public ErdItem
{
    Q_OBJECT

    public:
        ErdEntity(SqliteCreateTable* tableModel);

        SqliteCreateTable* getTableModel() const;
        int rowIndexAt(const QPointF& point);
        QRectF rowRect(int rowIndex);
        bool isClickable();
        void updateConnectionsGeometry();
        void addConnection(ErdConnection* conn);
        void removeConnection(ErdConnection* conn);

    private:
        void clearLayout();
        void rebuild();
        void addColumn(const QString& text);
        void addTableTitle(const QString& text);
        QLayoutItem* setIcon(Icon* icon);
        QLayoutItem* setBlankIcon();
        QLayoutItem* setLabel(const QString& text);

        SqliteCreateTable* tableModel = nullptr;
        QGridLayout* layout = nullptr;
        QFrame* frame = nullptr;
        int currRow = 0;
        QList<ErdConnection*> connections;

    public slots:
        void modelUpdated();
};

#endif // ERDENTITY_H
