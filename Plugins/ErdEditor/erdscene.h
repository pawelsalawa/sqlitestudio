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
class ErdConnection;
class Db;

class ErdScene : public QGraphicsScene
{
    Q_OBJECT

    public:
        ErdScene(ErdArrowItem::Type arrowType, QObject *parent = nullptr);

        QSet<QString> parseSchema(Db* db);
        void refreshSchema(Db* db, ErdChangeEntity* entityChange);
        void refreshSchema(Db* db, ErdChangeNewEntity* newEntityChange);
        QList<ErdEntity*> getAllEntities() const;
        void setArrowType(ErdArrowItem::Type arrowType);
        ErdArrowItem::Type getArrowType() const;
        void applyConfig(const QHash<QString, QVariant>& erdLayout);
        QHash<QString, QVariant> getConfig();
        void placeNewEntity(ErdEntity* entity);
        ErdConnection* getConnectionForArrow(ErdArrowItem* arrow);

        static constexpr const char* CFG_KEY_ENTITIES = "entities";
        static constexpr const char* CFG_KEY_VIEW_RECT = "viewRect";
        static constexpr const char* CFG_KEY_ARROW_TYPE = "arrowType";
        static constexpr const char* CFG_KEY_POS = "pos";
        static constexpr const char* CFG_KEY_COLOR = "color";

    private:
        void setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable);
        void setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity);
        void setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity, SqliteCreateTable::Column* srcColumn);
        void setupEntityConnection(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity, const QString& srcColumn,
                                   int sourceReferenceIdx, SqliteForeignKey* fk);
        void arrangeEntities(int algo);
        QPointF getPosForNewEntity() const;
        QSet<ErdConnection*> getConnections() const;
        bool confirmLayoutChange() const;
        StrHash<ErdEntity*> collectEntitiesByTable() const;
        void refreshConnections(const QList<ErdEntity*>& forEntities = {});
        void refreshEntityFromTableName(SchemaResolver& resolver, StrHash<ErdEntity*>& entitiesByTable, ErdEntity* entity, const QString& tableName);

        QList<ErdEntity*> entities;
        ErdArrowItem::Type arrowType;

        static constexpr qreal sceneMargin = 200;

    public slots:
        void arrangeEntitiesNeato(bool skipConfirm = false);
        void arrangeEntitiesFdp(bool skipConfirm = false);
        void refreshSceneRect();

    signals:
        void showEntityToUser(ErdEntity* entity);
};

#endif // ERDSCENE_H
