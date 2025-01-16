#ifndef DBTREEMODEL_H
#define DBTREEMODEL_H

#include "db/db.h"
#include "dbtreeitem.h"
#include "services/config.h"
#include "guiSQLiteStudio_global.h"
#include "common/strhash.h"
#include <QStandardItemModel>
#include <QObject>

class DbManager;
class DbTreeView;
class DbPlugin;
class DbObjectOrganizer;
class QMenu;
class QCheckBox;

class GUI_API_EXPORT DbTreeModel : public QStandardItemModel
{
    Q_OBJECT

    public:
        DbTreeModel();
        ~DbTreeModel();

        void connectDbManagerSignals();
        DbTreeItem* findItem(DbTreeItem::Type type, const QString &name);
        DbTreeItem* findItem(DbTreeItem::Type type, Db* db);
        DbTreeItem* findFirstItemOfType(DbTreeItem::Type type);
        DbTreeItem* findItemBySignature(const QStringList &signature);
        QList<DbTreeItem*> findItems(DbTreeItem::Type type);
        void move(QStandardItem* itemToMove, QStandardItem* newParentItem, int newRow = -1);
        void move(QStandardItem* itemToMove, int newRow);
        DbTreeItem *createGroup(const QString& name, QStandardItem *parent = nullptr);
        void deleteGroup(QStandardItem* groupItem);
        QStandardItem *root() const;
        QStringList getGroupFor(QStandardItem* item);
        void storeGroups();
        void refreshSchema(Db* db);
        QList<DbTreeItem*> getAllItemsAsFlatList() const;
        void setTreeView(DbTreeView *value);
        QVariant data(const QModelIndex &index, int role) const;
        QStringList mimeTypes() const;
        QMimeData* mimeData(const QModelIndexList &indexes) const;
        bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
        bool pasteData(const QMimeData* data, int row, int column, const QModelIndex& parent, Qt::DropAction defaultAction = Qt::IgnoreAction,
                       bool *invokeStdAction = nullptr);
        void interruptableStarted(Interruptable* obj);
        void interruptableFinished(Interruptable* obj);
        bool getIgnoreDbLoadedSignal() const;
        void setIgnoreDbLoadedSignal(bool value);
        bool hasDbTreeItem(const QMimeData* data);
        QList<DbTreeItem*> getDragItems(const QMimeData* data);
        QList<DbTreeItem*> getItemsForIndexes(const QModelIndexList& indexes) const;
        QHash<QString, QVariant> collectSelectionState();
        void restoreSelectionState(const QHash<QString, QVariant>& selectionState);

        static DbTreeItem* findItem(QStandardItem *parentItem, DbTreeItem::Type type, const QString &name);
        static DbTreeItem* findItem(QStandardItem* parentItem, DbTreeItem::Type type, Db* db);
        static QList<DbTreeItem*> findItems(QStandardItem* parentItem, DbTreeItem::Type type);
        static DbTreeItem* findFirstItem(QStandardItem* parentItem, DbTreeItem::Type type);
        static void staticInit();

        static const constexpr char* MIMETYPE = "application/x-sqlitestudio-dbtreeitem";

    private:
        void readGroups(QList<Db*> dbList);
        QList<Config::DbGroupPtr> childsToConfig(QStandardItem* item);
        void restoreGroup(const Config::DbGroupPtr& group, QList<Db*>* dbList = nullptr, QStandardItem *parent = nullptr);
        bool applyFilter(QStandardItem* parentItem, const QString& filter);
        void refreshSchema(Db* db, QStandardItem* item);
        void collectExpandedState(QHash<QString, bool>& state, QStandardItem* parentItem = nullptr);
        QStandardItem* refreshSchemaDb(Db* db);
        QList<QStandardItem*> refreshSchemaTables(const QStringList &tables, const QSet<QString>& virtualTables, bool sort);
        QList<QStandardItem*> refreshSchemaTableColumns(const QStringList& columns);
        QList<QStandardItem*> refreshSchemaIndexes(const QStringList& indexes, bool sort);
        QList<QStandardItem*> refreshSchemaTriggers(const QStringList& triggers, bool sort);
        QList<QStandardItem*> refreshSchemaViews(const QStringList &views, bool sort);
        void populateChildItemsWithDb(QStandardItem* parentItem, Db* db);
        void loadTableSchema(DbTreeItem* tableItem);
        void loadViewSchema(DbTreeItem* viewItem);
        void refreshSchemaBuild(QStandardItem* dbItem, QList<QStandardItem*> tables, QList<QStandardItem*> views);
        void restoreExpandedState(const QHash<QString, bool>& expandedState, QStandardItem* parentItem);
        QString getToolTip(DbTreeItem *item) const;
        QString getDbToolTip(DbTreeItem *item) const;
        QString getTableToolTip(DbTreeItem *item) const;
        QList<DbTreeItem*> getChildsAsFlatList(QStandardItem* item) const;
        bool dropDbTreeItem(const QList<DbTreeItem*>& srcItems, DbTreeItem* dstItem, Qt::DropAction defaultAction, bool* invokeStdDropAction);
        bool dropDbObjectItem(const QList<DbTreeItem*>& srcItems, DbTreeItem* dstItem, Qt::DropAction defaultAction);
        QCheckBox* createCopyOrMoveMenuCheckBox(QMenu* menu, const QString& label);
        bool dropUrls(const QList<QUrl>& urls);
        bool quickAddDroppedDb(const QString& filePath);
        void moveOrCopyDbObjects(const QList<DbTreeItem*>& srcItems, DbTreeItem* dstItem, bool move, bool includeData, bool includeIndexes, bool includeTriggers);
        QHash<QStringList, DbTreeItem*> getAllItemsWithSignatures() const;
        DbTreeItem* findDeepestExistingItemBySignature(QStringList signature, const QHash<QStringList, DbTreeItem*>& allItemsWithSignatures) const;

        static bool confirmReferencedTables(const QStringList& tables);
        static bool resolveNameConflict(QString& nameInConflict);
        static bool confirmConversion(const QList<QPair<QString, QString>>& diffs);
        static bool confirmConversionErrors(const QHash<QString, QSet<QString>>& errors);

        static const QString toolTipTableTmp;
        static const QString toolTipHdrRowTmp;
        static const QString toolTipRowTmp;
        static const QString toolTipIconRowTmp;

        DbTreeView* treeView = nullptr;
        bool requireSchemaReloading = false;
        DbObjectOrganizer* dbOrganizer = nullptr;
        QList<Interruptable*> interruptables;
        bool ignoreDbLoadedSignal = false;
        QString currentFilter;

    private slots:
        void expanded(const QModelIndex &index);
        void collapsed(const QModelIndex &index);
        void dbAdded(Db* db);
        void dbUpdated(const QString &oldName, Db* db);
        void dbRemoved(Db* db);
        void dbConnected(Db* db, bool expandItem = true);
        void dbDisconnected(Db* db);
        void dbUnloaded(Db* db);
        void dbLoaded(Db* db);
        void massSaveBegins();
        void massSaveCommitted();
        void markSchemaReloadingRequired();
        void dbObjectsMoveFinished(bool success, Db* srcDb, Db* dstDb);
        void dbObjectsCopyFinished(bool success, Db* srcDb, Db* dstDb);

    public slots:
        void loadDbList();
        void itemChangedVisibility(DbTreeItem* item);
        void applyFilter(const QString& filter);
        void dbRemoved(const QString& name);
        void dbRemoved(QStandardItem* item);
        void interrupt();

    signals:
        void updateItemHidden(DbTreeItem* item);
};

#endif // DBTREEMODEL_H
