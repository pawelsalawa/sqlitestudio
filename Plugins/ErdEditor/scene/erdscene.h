#ifndef ERDSCENE_H
#define ERDSCENE_H

#include "common/strhash.h"
#include "parser/ast/sqlitecreatetable.h"
#include "erdarrowitem.h"
#include "schemaresolver.h"
#include <QGraphicsScene>
#include <QSet>

class ErdEntity;
class ErdChange;
class ErdChangeModifyEntity;
class ErdChangeNewEntity;
class ErdChangeDeleteEntity;
class ErdChangeDeleteConnection;
class ErdChangeComposite;
class ErdConnection;
class Db;
class ChainExecutor;

class ErdScene : public QGraphicsScene
{
    Q_OBJECT

    public:
        /**
         * @brief API provided to ErdChange implementations to apply changes on the scene.
         */
        class SceneChangeApi {
            public:
                virtual ~SceneChangeApi() = default;

                /**
                 * @brief Refreshes contents of given entity by parsing DDL of existing table.
                 * @param entityName Current name of the entity on the scene.
                 * @param actualTableName Actual table name in the database to parse DDL from. May (but does not must) differ from entityName.
                 */
                virtual void refreshEntity(const QString& entityName, const QString& actualTableName) = 0;

                /**
                 * @brief Refreshes all entities which correspond to given table names.
                 * @param tables List of table names to refresh entities for.
                 *
                 * It expects entity names on the scene to match table names provided.
                 * If certain entity does not exist on the scene yet, it will be created.
                 * If certain table does not exist in the database, the corresponding entity will be removed from the scene.
                 */
                virtual void refreshEntitiesByTableNames(const QStringList& tables) = 0;

                /**
                 * @brief Removes entity with given name from the scene.
                 * @param entityName Name of the entity to remove.
                 *
                 * It given entity does not exist on the scene already, the method does nothing.
                 */
                virtual void removeEntityFromScene(const QString& entityName) = 0;

                /**
                 * @brief Sets position of given entity on the scene.
                 * @param entityName Name of the entity existing on the scene to set position for.
                 * @param pos New position to set.
                 */
                virtual void setEntityPosition(const QString& entityName, const QPointF& pos) = 0;

                /**
                 * @brief Gets position of given entity on the scene.
                 * @param entityName Name of the entity existing on the scene to get position for.
                 * @return Current position of the entity.
                 */
                virtual QPointF getEntityPosition(const QString& entityName) = 0;

                /**
                 * @brief Sets custom color of given entity on the scene.
                 * @param entityName Name of the entity existing on the scene to set color for.
                 * @param color New color to set. Invalid color removes custom color.
                 */
                virtual void setEntityColor(const QString& entityName, const QColor& color) = 0;

                /**
                 * @brief Gets custom color of given entity on the scene.
                 * @param entityName Name of the entity existing on the scene to get color for.
                 * @return Current custom color of the entity. Invalid color if no custom color is set.
                 */
                virtual QColor getEntityColor(const QString& entityName) = 0;

                /**
                 * @brief Provides schema resolver used by the scene.
                 * @return Reference to schema resolver.
                 */
                virtual SchemaResolver& schemaResolver() = 0;

                /**
                 * @brief Requests scene to update its contents visually.
                 */
                virtual void updateScene() = 0;
        };

        ErdScene(ErdArrowItem::Type arrowType, QObject *parent = nullptr);
        ~ErdScene();

        void setDb(Db* db);
        Db* getDb() const;
        QSet<QString> parseSchema();
        QList<ErdEntity*> getAllEntities() const;
        QList<ErdEntity*> getSelectedEntities() const;
        void setArrowType(ErdArrowItem::Type arrowType);
        ErdArrowItem::Type getArrowType() const;

        /**
         * @return Entities for which position was not restored from the session.
         */
        void applyConfig(const QHash<QString, QVariant>& erdLayout);
        QHash<QString, QVariant> getConfig();
        void placeNewEntity(ErdEntity* entity, const QPointF& pos);
        ErdConnection* getConnectionForArrow(ErdArrowItem* arrow);
        bool undoChange(ErdChange* change);
        bool redoChange(ErdChange* change);
        QList<ErdEntity*> applyColorToSelectedEntities(const QColor& color);
        QString getNewEntityName(const QString& prefix, int startIdx) const;
        void editEntityColumn(ErdEntity* entity, const QPointF& pos);

        static QHash<QString, QVariant> createEntityConfigEntry(const QPointF& pos,
                                                                const QColor& bgColor = QColor(),
                                                                const QColor& fgColor = QColor());

        static constexpr auto CFG_KEY_ENTITIES = "entities";
        static constexpr auto CFG_KEY_VIEW_RECT = "viewRect";
        static constexpr auto CFG_KEY_ARROW_TYPE = "arrowType";
        static constexpr auto CFG_KEY_POS = "pos";
        static constexpr auto CFG_KEY_COLOR = "color";

    private:
        class SceneChangeApiImpl : public SceneChangeApi {
            public:
                explicit SceneChangeApiImpl(ErdScene& scene);

                void refreshEntity(const QString& entityName, const QString& actualTableName) override;
                void refreshEntitiesByTableNames(const QStringList& tables) override;
                void removeEntityFromScene(const QString& entityName) override;
                void setEntityPosition(const QString& entityName, const QPointF& pos) override;
                void setEntityColor(const QString& entityName, const QColor& color) override;
                QPointF getEntityPosition(const QString& entityName) override;
                QColor getEntityColor(const QString& entityName) override;
                SchemaResolver& schemaResolver() override;
                void updateScene() override;

            private:
                ErdScene& scene;
        };

        void setupEntityConnections();
        void setupEntityConnections(ErdEntity* srcEntity);
        void setupEntityConnections(ErdEntity* srcEntity, SqliteCreateTable::Column* srcColumn);
        ErdConnection* setupEntityConnection(ErdEntity* srcEntity, const QString& srcColumn,
                                             int sourceReferenceIdx, SqliteForeignKey* fk);
        void arrangeEntities(int algo);
        void arrangeEntities(int algo, QSet<ErdEntity*> pinnedEntities);
        QPointF getPosForNewEntity(ErdEntity* entity, const QSet<ErdEntity*>& excludeFromCalculations = {}) const;
        QPointF getPosForNewEntitySpiral(ErdEntity* entity, const QSet<ErdEntity*>& excludeFromCalculations = {}) const;
        bool collides(const QRectF& candidate, const QSet<ErdEntity*>& exclude) const;
        QSet<ErdConnection*> getConnections() const;
        bool confirmLayoutChange() const;
        void refreshEntityFromTableName(ErdEntity* entity, const QString& tableName);
        void refreshSchemaForTableNames(const QStringList& tables);
        void setEntityPosition(ErdEntity* entity, const QPointF& pos);
        ErdChange* deleteEntity(ErdEntity*& entity);
        ErdChange* deleteConnection(ErdConnection*& connection);
        void entityCreated(ErdEntity* entity);
        void entityToBeDeleted(ErdEntity* entity);
        void refreshScheduledConnections();

        void handleChangeUndo(ErdChange* change);
        void handleChangeRedo(ErdChange* change);
        void changeApplied();

        QList<ErdEntity*> entities;
        QList<ErdEntity*> connectionRefreshScheduled;
        StrHash<ErdEntity*> entityMap;
        ErdArrowItem::Type arrowType;
        Db* db = nullptr;
        ChainExecutor* ddlExecutor = nullptr;
        SchemaResolver* schemaResolver = nullptr;
        SceneChangeApi* sceneChangeApi = nullptr;

        static constexpr qreal sceneMargin = 200;

    public slots:
        void arrangeEntitiesNeato(bool skipConfirm = false);
        void arrangeEntitiesFdp(bool skipConfirm = false);
        void refreshSceneRect();
        void notify(ErdChange* change);
        void deleteItems(const QList<QGraphicsItem *> &items);
        void handleChange(ErdChange* change);
        void applyItemFiltering(const QString& value);
        void connectionFinalizationFailed();
        void selectAll();
        void clearScene();

        /**
         * Removes entity from the scene only. No db changes nor ChangeRegistry shifts are made.
         * Any connections to/from this entity are ignored. This should be a final step
         * of entity remove - once there are no other object dependencies to this entity item.
         */
        void removeEntityFromScene(ErdEntity* entity);
        void removeEntityFromSceneByName(const QString& tableName);

    signals:
        void showEntityToUser(ErdEntity* entity);
        void requiresImmediateViewUpdate();
        void changeCreated(ErdChange* change);
        void sidePanelAbortRequested();
        void sidePanelRefreshRequested();
        void connectionEditAbortRequested();
        void entityNameEditedInline(ErdEntity* entity, const QString& newName);
        void entityFieldEditedInline(ErdEntity* entity, int colIdx, const QString& newName);
        void entityFieldDeletedInline(ErdEntity* entity, int colIdx);
        void requestVisibilityOf(const QRectF& rect);
        void requestToEditColumn(ErdEntity* entity, const QString& columnName);
};

#endif // ERDSCENE_H
