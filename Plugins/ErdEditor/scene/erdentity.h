#ifndef ERDENTITY_H
#define ERDENTITY_H

#include "erditem.h"
#include "parser/ast/sqlitecreatetable.h"
#include <QGraphicsRectItem>

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
class QGraphicsProxyWidget;
class QLineEdit;

class ErdEntity : public QObject, public QGraphicsRectItem, public ErdItem
{
        Q_OBJECT

    public:
        ErdEntity(SqliteCreateTable* tableModel);
        ErdEntity(const SqliteCreateTablePtr& tableModel);

        SqliteCreateTablePtr getTableModel() const;
        SqliteStatement* getStatementAtRowIndex(int rowIdx) const;
        void setTableModel(const SqliteCreateTablePtr& tableModel);
        void modelUpdated();
        int rowIndexAt(const QPointF& point);
        QRectF rowRect(int rowIndex);
        bool isClickable();
        void updateConnectionsGeometry();
        void addConnection(ErdConnection* conn);
        void removeConnection(ErdConnection* conn);
        void clearConnections();
        QList<ErdConnection*> getConnections() const;
        QList<ErdConnection*> getOwningConnections() const;
        QList<ErdConnection*> getForeignConnections() const;
        QString getTableName() const;
        void updateConnectionIndexes();
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
        bool isExistingTable() const;
        void setExistingTable(bool newExistingTable);
        bool edit(const QPointF& point);
        void editName();
        bool eventFilter(QObject *obj, QEvent *event);
        void stopInlineEditing();
        void setCustomColor(const QColor& bg, const QColor& fg);
        QPair<QColor, QColor> getCustomColor() const;
        bool usesCustomColor() const;

    protected:
        void keyPressEvent(QKeyEvent* event);
        void keyReleaseEvent(QKeyEvent* event);

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
            SqliteStatement* sqliteStatement = nullptr;
            bool emptyTextValue = false;

            qreal calcWidth(qreal iconColumn, qreal nameColumn) const;
            qreal height() const;
            qreal calcIconsWidth() const;
            qreal calcNameWidth() const;
            qreal updateLayout(qreal iconColumn, qreal nameColumn, qreal globalWidth, qreal globalY);
            void notLastAnymore();
            void becomeLast();
            void setText(const QString& value);
            QString getText() const;
            void applyColors(const QColor& bg, const QColor& fg);
        };

        void editRow(int rowIdx);
        void rebuild();
        void addColumn(SqliteCreateTable::Column* column, QList<SqliteCreateTable::Constraint*> tableConstraints, bool isLast);
        Row* addColumn(const QString& columnName, const QString& typeName, bool isLast);
        void addTableTitle();
        void disableChildSelection(QGraphicsItem* parent);
        void enableChildFocusing(QGraphicsItem* parent);
        void updateGeometry();
        void inlineEditTabKeyPressed(bool backward = false);
        void inlineEditEnterKeyPressed();
        void updateInlineEditorGeometry(Row* row, QGraphicsProxyWidget* inlineProxy);
        void requestRowVisibility(Row* row);
        bool inlineEditionCheckIfFieldDeleted(bool indexAutocorrection = true);
        void handleFieldEditedInline(int rowIdx, const QString& newName);
        void handleFieldDeleted(int rowIdx);

        static constexpr qreal CELL_PADDING = 7.0;
        static constexpr qreal TEXT_GAP = 8.0;
        static constexpr qreal ICON_GAP = 4.0;

        SqliteCreateTablePtr tableModel;
        QList<ErdConnection*> connections;
        QList<Row*> rows;
        QGraphicsPixmapItem* cornerIcon = nullptr;
        bool existingTable = true;
        int lastInlineEditedRow = -1;
        QColor customBgColor;
        QColor customFgColor;

    private slots:
        void applyRowEdition(int rowIdx, const QString& value, QGraphicsProxyWidget* inlineProxy);

    signals:
        void nameEdited(const QString& newName);
        void fieldEdited(int rowIdx, const QString& newName);
        void fieldDeleted(int rowIdx);
        void requestVisibilityOf(const QRectF& rect);
        void requestSceneGeomUpdate();
};

#endif // ERDENTITY_H
