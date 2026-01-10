#ifndef ERDCHANGE_H
#define ERDCHANGE_H

#include "scene/erdscene.h"
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

        /**
         * @brief Performs necessary scene updates according to the particular ErdChange implementation.
         * @param api Specialized API provided by the ErdScene needed to perform scene updates.
         *
         * Called after the DDL for this change has been applied in the current context.
         */
        virtual void apply(ErdScene::SceneChangeApi& api) = 0;

        /**
         * @brief Perdorms necessary scene updates after undoing this change.
         * @param api Specialized API provided by the ErdScene needed to perform scene updates.
         *
         * Called after the Undo DDL for this change has been applied in the current context.
         */
        virtual void applyUndo(ErdScene::SceneChangeApi& api) = 0;

        /**
         * @brief Perdorms necessary scene updates after redoing this change.
         * @param api Specialized API provided by the ErdScene needed to perform scene updates.
         *
         * Default implementation just executes the apply() method, but some changes may require
         * customized behavior in case of Redo vs First Time execution.
         */
        virtual void applyRedo(ErdScene::SceneChangeApi& api);

        Category getCategory() const;
        virtual QString getTransactionId() const;
        QString getDescription() const;

    protected:
        ErdChange(Category category, const QString& description, bool generateTransactionId = false);

        virtual QStringList getChangeDdl() = 0;

        Category category;
        /**
         * @brief transactionId Savepoint name marked just before executing this change,
         *        so undoing this change can be one by rolling back to this savepoint.
         */
        QString transactionId;
        QString description;
};

#endif // ERDCHANGE_H
