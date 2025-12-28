#ifndef ERDCHANGE_H
#define ERDCHANGE_H

#include <QStringList>

class ErdChangeComposite;

class ErdChange
{
    public:
        enum class Category
        {
            LAYOUT,
            COMPOSITE,
            ENTITY_CHANGE,
            ENTITY_NEW,
            ENTITY_DELETE,
            CONNECTION_DELETE,
        };

        friend class ErdChangeComposite;

        ErdChange() = delete;
        virtual ~ErdChange() {}

        /**
         * @param skipSaveoints Pass true if committing all changes to actual database, as there is no need to keep savepoint marks in that case.
         */
        QStringList toDdl(bool skipSaveoints = false);

        /**
         * @return Savepoint rollback statement according to this change transactionId.
         */
        virtual QStringList getUndoDdl();

        QStringList getEntitiesToRefreshAfterUndo() const;

        Category getCategory() const;
        QString getTransactionId() const;
        QString getDescription() const;

    protected:
        ErdChange(Category category, const QString& description, bool generateTransactionId = false);

        virtual QStringList getChangeDdl() = 0;
        virtual QStringList provideUndoEntitiesToRefresh() const = 0;

        Category category;
        /**
         * @brief transactionId Savepoint name marked just before executing this change,
         *        so undoing this change can be one by rolling back to this savepoint.
         */
        QString transactionId;
        QString description;
};

#endif // ERDCHANGE_H
