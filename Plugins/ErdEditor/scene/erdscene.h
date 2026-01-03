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
class ErdChangeEntity;
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
        ErdScene(ErdArrowItem::Type arrowType, QObject *parent = nullptr);

        void setDb(Db* db);
        Db* getDb() const;
        QSet<QString> parseSchema();
        QList<ErdEntity*> getAllEntities() const;
        void setArrowType(ErdArrowItem::Type arrowType);
        ErdArrowItem::Type getArrowType() const;
        void applyConfig(const QHash<QString, QVariant>& erdLayout);
        QHash<QString, QVariant> getConfig();
        void placeNewEntity(ErdEntity* entity, const QPointF& pos);
        ErdConnection* getConnectionForArrow(ErdArrowItem* arrow);
        bool undoChange(ErdChange* change);
        bool redoChange(ErdChange* change);

        static constexpr const char* CFG_KEY_ENTITIES = "entities";
        static constexpr const char* CFG_KEY_VIEW_RECT = "viewRect";
        static constexpr const char* CFG_KEY_ARROW_TYPE = "arrowType";
        static constexpr const char* CFG_KEY_POS = "pos";
        static constexpr const char* CFG_KEY_COLOR = "color";

    private:
        void setupEntityConnections();
        void setupEntityConnections(ErdEntity* srcEntity);
        void setupEntityConnections(ErdEntity* srcEntity, SqliteCreateTable::Column* srcColumn);
        ErdConnection* setupEntityConnection(ErdEntity* srcEntity, const QString& srcColumn,
                                             int sourceReferenceIdx, SqliteForeignKey* fk);
        void arrangeEntities(int algo);
        QPointF getPosForNewEntity() const;
        QSet<ErdConnection*> getConnections() const;
        bool confirmLayoutChange() const;
        void refreshConnections(const QList<ErdEntity*>& forEntities = {});
        void refreshEntityFromTableName(SchemaResolver& resolver, ErdEntity* entity, const QString& tableName);
        ErdChange* deleteEntity(ErdEntity*& entity);
        ErdChange* deleteConnection(ErdConnection*& connection);
        void refreshSchemaForTableNames(const QStringList& tables);
        void entityCreated(ErdEntity* entity);
        void entityToBeDeleted(ErdEntity* entity);
        void refreshScheduledConnections();

        void handleChangeByType(ErdChange* change);
        void handleSingleChange(ErdChangeEntity* entityChange);
        void handleSingleChange(ErdChangeNewEntity* newEntityChange);
        void handleSingleChange(ErdChangeDeleteEntity* change);
        void handleSingleChange(ErdChangeDeleteConnection* change);

        void handleChangeUndo(ErdChange* change);
        void handleChangeUndoByType(ErdChange* change);
        void handleSingleChangeUndo(ErdChangeEntity* change);
        void handleSingleChangeUndo(ErdChangeNewEntity* change);
        void handleSingleChangeUndo(ErdChangeDeleteEntity* change);
        void handleSingleChangeUndo(ErdChangeDeleteConnection* change);

        /**
         * @param forwardExecution true if executing new change/redoing change, or false if undoing change.
         */
        void handleSingleChange(ErdChangeEntity* change, bool forwardExecution);
        void handleSingleChange(ErdChangeDeleteConnection* change, bool forwardExecution);

        void restoreEntityPosition(const QString& tableName, const QPointF& pos);

        QList<ErdEntity*> entities;
        QList<ErdEntity*> connectionRefreshScheduled;
        StrHash<ErdEntity*> entityMap;
        ErdArrowItem::Type arrowType;
        Db* db = nullptr;
        ChainExecutor* ddlExecutor = nullptr;

        static constexpr qreal sceneMargin = 200;

    public slots:
        void arrangeEntitiesNeato(bool skipConfirm = false);
        void arrangeEntitiesFdp(bool skipConfirm = false);
        void refreshSceneRect();
        void notify(ErdChange* change);
        void deleteItems(const QList<QGraphicsItem *> &items);
        void handleChange(ErdChange* change);

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
        void changeReceived(ErdChange* change);
        void sidePanelAbortRequested();
        void sidePanelRefreshRequested();
        void entityNameEditedInline(ErdEntity* entity, const QString& newName);
        void entityFieldEditedInline(ErdEntity* entity, int colIdx, const QString& newName);
};

#endif // ERDSCENE_H
