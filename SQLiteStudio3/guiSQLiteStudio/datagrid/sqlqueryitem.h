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
                LIMITED_VALUE = 1003,
                COLUMN = 1004,
                UNCOMMITTED = 1005,
                COMMITTING_ERROR = 1006,
                NEW_ROW = 1007,
                DELETED = 1008,
                OLD_VALUE = 1009,
                JUST_INSERTED_WITHOUT_ROWID = 1010,
                VALUE_FOR_DISPLAY = 1011
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

        bool isNewRow() const;
        void setNewRow(bool isNew);

        bool isJustInsertedWithOutRowId() const;
        void setJustInsertedWithOutRowId(bool justInsertedWithOutRowId);

        bool isDeletedRow() const;
        void setDeletedRow(bool isDeleted);

        QVariant getValue() const;
        void setValue(const QVariant& value, bool limited = false, bool loadedFromDb = false);
        bool isLimitedValue() const;

        QVariant getOldValue() const;
        void setOldValue(const QVariant& value);

        QVariant getValueForDisplay() const;
        void setValueForDisplay(const QVariant& value);

        /**
         * @brief loadFullData Reloads entire value of the cell from database.
         * @return QString::null on sucess, or error string on failure.
         */
        QString loadFullData();

        /**
         * @brief getFullValue Loads and returns full value from database, but keeps the original value.
         * @return Full value, reloaded from database.
         * Calls loadFullData(), then getValue() for the result,
         * but just before returning - restores initial, limited value.
         */
        QVariant getFullValue();

        SqlQueryModelColumn* getColumn() const;
        void setColumn(SqlQueryModelColumn* column);

        SqlQueryModel* getModel() const;

        void setData(const QVariant& value, int role = Qt::UserRole + 1);
        QVariant data(int role = Qt::UserRole + 1) const;

    private:
        void setLimitedValue(bool limited);
        QVariant adjustVariantType(const QVariant& value);
        QString getToolTip() const;
};

#endif // SQLQUERYITEM_H
