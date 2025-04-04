#ifndef ERDCHANGE_H
#define ERDCHANGE_H

#include <QStringList>

class ErdChange
{
    public:
        enum class Category
        {
            LAYOUT,
            ENTITY_CHANGE,
            ENTITY_NEW
        };

        ErdChange() = delete;

        /**
         * @param skipSaveoints Pass true if committing all changes to actual database, as there is no need to keep savepoint marks in that case.
         */
        QStringList toDdl(bool skipSaveoints = false);

        /**
         * @return Savepoint rollback statement according to this change transactionId.
         */
        QStringList getUndoDdl();

        Category getCategory() const;
        QString getTransactionId() const;

    protected:
        ErdChange(Category category, bool generateTransactionId = false);

        virtual QStringList getChangeDdl() = 0;

        Category category;
        QString transactionId;
};

#endif // ERDCHANGE_H
