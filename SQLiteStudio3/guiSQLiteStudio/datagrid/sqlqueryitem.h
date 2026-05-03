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
                ROWID = 1001,               /**< RowId of the cell, used for identifying the row in database, so we can update it with new value if needed.
                                                 This is loaded from DB and not modified by user, because ROWID doesn't change when value is updated. */
                VALUE = 1002,               /**< Cell value, used for both display and editing.
                                                 This is the main value of the cell, which is loaded
                                                 from DB and modified by user. */
                COLUMN = 1003,              /**< Pointer to SqlQueryModelColumn, used for easy access to column info
                                                 like name, type, etc. when needed. */
                UNCOMMITTED = 1004,         /**< True if the value was modified by user and is not yet committed
                                                 to database, false otherwise. */
                COMMITTING_ERROR = 1005,    /**< True if there was an error during commit of the value, false otherwise.
                                                 This is used to mark cells with errors after commit attempt,
                                                 so user can easily spot them and fix. */
                NEW_ROW = 1006,             /**< Row being inserted */
                DELETED = 1007,             /**< Row marked for deletion */
                OLD_VALUE = 1008,           /**< Cell value before modification, used for rollback */
                UNTOUCHED = 1009,           /**< New row's cell marked to be filled with default value,
                                                 ie. when user leaves it without modifying it at all.
                                                 Not the same as NULL, cause null can be set explicitly by user,
                                                 while untouched means that user didn't change the value at all,
                                                 so it should be loaded from DB as default value for the column.
                                                 This info is useful if the column has DEFAULT, PK+AUTOINCR
                                                 or GENERATED constraint. */
                COMMITTING_ERROR_MESSAGE = 1010, /**< In case of error during commit, this role can be used
                                                      to store error message from database, so it can be shown
                                                      in tooltip for the cell. */
                EDIT_SKIP_INITIAL_SELECT = 1011  /**< To prevent content selection initially when editing with double-click */
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

        bool isUntouched() const;
        void setUntouched(bool isUntouched);

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

        static QString formatToolTip(SqlQueryModelColumn* col, const QString& footer);
        static QString formatToolTip(SqlQueryModelColumn* col, bool withRowId, RowId rowId, bool withCommittingError, const QString& committingErrorMsg, const QString& footer = QString());

    private:
        QString getNullDisplayString() const;
        QVariant adjustVariantType(const QVariant& value);
        QString getToolTip() const;
        void rememberOldValue();
        void clearOldValue();

        static constexpr int DISPLAY_LEN_LIMIT = 1000;

        QMutex valueSettingLock;
};

#endif // SQLQUERYITEM_H
