#ifndef ERDCHANGE_H
#define ERDCHANGE_H

#include "scene/erdscene.h"
#include <QStringList>

class ErdEffectiveChange;
class ErdChangeComposite;

/**
 * @brief Base class representing a single change made in the ERD scene.
 */
class ErdChange
{
    public:
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

        /**
         * @brief Provides transaction ID (savepoint name) for this change.
         * @return Transaction ID string.
         */
        virtual QString getTransactionId() const;

        /**
         * @brief Provides human-readable description of this change.
         * @return Description string.
         */
        QString getDescription() const;

        /**
         * @brief Determines whether this change affects the database schema (DDL) or is just a diagram/layout change.
         * @return True if this change produces DDL statements, false otherwise.
         */
        bool isDdlChange();

        /**
         * @brief Converts this change into an effective change representation.
         * @return Effective change object, or nullptr if not applicable.
         */
        virtual ErdEffectiveChange toEffectiveChange() const;

        /**
         * @brief If multiple changes are provided, they will be wrapped into a composite change.
         * @param changes List of changes to normalize.
         * @param compositeDescription Description to set on the composite change if multiple changes are provided.
         * @return Single change if only one change is provided, composite change if multiple changes are provided,
         *         nullptr if no changes are provided.
         */
        static ErdChange* normalizeChanges(const QList<ErdChange*>& changes, const QString& compositeDescription);

    protected:
        ErdChange(const QString& description, bool generateTransactionId = false);

        /**
         * @brief Provides DDL statements representing this change.
         * @return List of DDL statements.
         *
         * This should provide essential DDL statements, excluding savepoint - that is handled by the ErdChange.
         */
        virtual QStringList getChangeDdl() = 0;

        /**
         * @brief Provides cached DDL statements for this change.
         * @return List of DDL statements.
         *
         * This method caches the result of getChangeDdl() to avoid multiple computations.
         */
        QStringList getCachedChangeDdl();

        /**
         * @brief transactionId Savepoint name marked just before executing this change,
         *        so undoing this change can be one by rolling back to this savepoint.
         */
        QString transactionId;
        QString description;
        QStringList cachedDdl;
};

#endif // ERDCHANGE_H
