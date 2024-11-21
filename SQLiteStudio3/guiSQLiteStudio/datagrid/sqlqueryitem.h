#ifndef SQLQUERYITEM_H
#define SQLQUERYITEM_H

#include "sqlquerymodelcolumn.h"
#include "db/sqlquery.h"
#include "guiSQLiteStudio_global.h"
#include <QStandardItem>

class SqlQueryModel;

class GUI_API_EXPORT SqlQueryItem : public QObject, public QStandardItem
{
    Q_OBJECT

    public:
        struct GUI_API_EXPORT DataRole // not 'enum class' because we need autocasting to int for this one
        {
            enum Enum
            {
                ROWID = 1001,
                VALUE = 1002,
                COLUMN = 1003,
                UNCOMMITTED = 1004,
                COMMITTING_ERROR = 1005,
                NEW_ROW = 1006,
                DELETED = 1007,
                OLD_VALUE = 1008,
                JUST_INSERTED_WITHOUT_ROWID = 1009,
                COMMITTING_ERROR_MESSAGE = 1010,
                EDIT_SKIP_INITIAL_SELECT = 1011 // to prevent content selection initially when editing with double-click
            };
        };

        explicit SqlQueryItem(QObject *parent = 0);
        SqlQueryItem(const SqlQueryItem& item);

        QStandardItem* clone() const;

        RowId getRowId() const;
        void setRowId(const RowId& rowId);

        bool isUncommitted() const;
        void setUncommitted(bool uncommitted);
        void rollback();

        bool isCommittingError() const;
        void setCommittingError(bool isError);
        void setCommittingError(bool isError, const QString& msg);

        QString getCommittingErrorMessage() const;
        void setCommittingErrorMessage(const QString& value);

        bool isNewRow() const;
        void setNewRow(bool isNew);

        bool isJustInsertedWithOutRowId() const;
        void setJustInsertedWithOutRowId(bool justInsertedWithOutRowId);

        bool isDeletedRow() const;
        void setDeletedRow(bool isDeleted);

        QVariant getValue() const;
        void setValue(const QVariant& value, bool loadedFromDb = false);

        QVariant getOldValue() const;
        void setOldValue(const QVariant& value);

        SqlQueryModelColumn* getColumn() const;
        void setColumn(SqlQueryModelColumn* column);

        SqlQueryModel* getModel() const;

        void setData(const QVariant& value, int role = Qt::UserRole + 1);
        QVariant data(int role = Qt::UserRole + 1) const;

        void resetInitialFocusSelection();
        void skipInitialFocusSelection();
        bool shoulSkipInitialFocusSelection() const;

    private:
        QVariant adjustVariantType(const QVariant& value);
        QString getToolTip() const;
        void rememberOldValue();
        void clearOldValue();

        QMutex valueSettingLock;
};

#endif // SQLQUERYITEM_H
