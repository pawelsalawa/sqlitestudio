#ifndef ERDSCENE_H
#define ERDSCENE_H

#include "common/strhash.h"
#include "parser/ast/sqlitecreatetable.h"
#include <QGraphicsScene>

class ErdEntity;
class Db;

class ErdScene : public QGraphicsScene
{
    Q_OBJECT

    public:
        explicit ErdScene(QObject *parent = nullptr);

        void parseSchema(Db* db);

    private:
        void setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable);
        void setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity);
        void setupEntityConnections(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity, SqliteCreateTable::Column* srcColumn);
        void setupEntityConnection(const StrHash<ErdEntity*>& entitiesByTable, ErdEntity* srcEntity, const QString& srcColumn,
                                   int sourceReferenceIdx, SqliteForeignKey* fk);

        int lastCreatedX = -600;
        QList<ErdEntity*> entities;

    public slots:
        void newTable();
};

#endif // ERDSCENE_H
