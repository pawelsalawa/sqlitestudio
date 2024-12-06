#ifndef ERDSCENE_H
#define ERDSCENE_H

#include "common/strhash.h"
#include "parser/ast/sqlitecreatetable.h"
#include "erdarrowitem.h"
#include <QGraphicsScene>
#include <QSet>

class ErdEntity;
class ErdConnection;
class Db;

class ErdScene : public QGraphicsScene
{
    Q_OBJECT

    public:
        ErdScene(ErdArrowItem::Type arrowType, QObject *parent = nullptr);

        void parseSchema(Db* db);
        QList<ErdEntity*> getAllEntities() const;
        void setArrowType(ErdArrowItem::Type arrowType);
        ErdArrowItem::Type getArrowType() const;

    private:
        void setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable);
        void setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity);
        void setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity, SqliteCreateTable::Column* srcColumn);
        void setupEntityConnection(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity, const QString& srcColumn,
                                   int sourceReferenceIdx, SqliteForeignKey* fk);
        void arrangeEntities(int algo);
        QPointF getPosForNewEntity() const;
        QSet<ErdConnection*> getConnections() const;

        qreal lastCreatedX = 0;
        QList<ErdEntity*> entities;
        ErdArrowItem::Type arrowType;

        static constexpr qreal sceneMargin = 200;

    public slots:
        void newTable();
        void arrangeEntitiesNeato();
        void arrangeEntitiesFdp();
        void refreshSceneRect();
};

#endif // ERDSCENE_H
