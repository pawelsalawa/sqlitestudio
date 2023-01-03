#include "dbobjectdialogs.h"
#include "mainwindow.h"
#include "dialogs/indexdialog.h"
#include "dialogs/triggerdialog.h"
#include "common/utils_sql.h"
#include "dbtree/dbtree.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"
#include "mdiarea.h"
#include "mdiwindow.h"
#include "windows/tablewindow.h"
#include "windows/viewwindow.h"
#include "db/sqlquery.h"
#include "services/config.h"
#include <QMessageBox>
#include <QDebug>

DbObjectDialogs::DbObjectDialogs(Db* db) :
    db(db)
{
    mainWindow = MainWindow::getInstance();
    mdiArea = mainWindow->getMdiArea();
    parentWidget = mainWindow;
}

DbObjectDialogs::DbObjectDialogs(Db* db, QWidget* parentWidget) :
    db(db), parentWidget(parentWidget)
{
    mainWindow = MainWindow::getInstance();
    mdiArea = mainWindow->getMdiArea();
}

void DbObjectDialogs::addIndex(const QString& table)
{
    IndexDialog dialog(db, parentWidget);
    if (!table.isNull())
        dialog.setTable(table);

    dialog.exec();
}

void DbObjectDialogs::editIndex(const QString& index)
{
    if (index.isNull())
    {
        qWarning() << "Tried to edit null index.";
        return;
    }

    IndexDialog dialog(db, index, parentWidget);
    dialog.exec();
}

void DbObjectDialogs::addTriggerOnTable(const QString& table)
{
    addTrigger(table, QString());
}

void DbObjectDialogs::addTriggerOnView(const QString& view)
{
    addTrigger(QString(), view);
}

void DbObjectDialogs::addTrigger(const QString& table, const QString& view)
{
    TriggerDialog dialog(db, parentWidget);
    if (!table.isNull())
        dialog.setParentTable(table);
    else if (!view.isNull())
        dialog.setParentView(view);
    else
        return;

    dialog.exec();
}

void DbObjectDialogs::editTrigger(const QString& trigger)
{
    if (trigger.isNull())
    {
        qWarning() << "Tried to edit null trigger.";
        return;
    }

    TriggerDialog dialog(db, parentWidget);
    dialog.setTrigger(trigger);
    dialog.exec();
}

ViewWindow* DbObjectDialogs::addView(const QString &initialSelect)
{
    ViewWindow* win = new ViewWindow(db, mdiArea);
    win->setSelect(initialSelect);
    mdiArea->addSubWindow(win);
    return win;
}

ViewWindow* DbObjectDialogs::editView(const QString& database, const QString& view)
{
    ViewWindow* win = nullptr;
    for (MdiWindow* mdiWin : mdiArea->getWindows())
    {
        win = dynamic_cast<ViewWindow*>(mdiWin->getMdiChild());
        if (!win)
            continue;

        if (win->getDb() == db && win->getView() == view)
        {
            mdiArea->setActiveSubWindow(mdiWin);
            return win;
        }
    }

    win = new ViewWindow(mdiArea, db, database, view);
    if (win->isInvalid())
    {
        delete win;
        return nullptr;
    }

    mdiArea->addSubWindow(win);
    return win;
}

void DbObjectDialogs::editObject(Type type, const QString& name)
{
    editObject(type, "main", name);
}

void DbObjectDialogs::editObject(Type type, const QString& database, const QString& name)
{
    SchemaResolver schemaResolver(db);
    QString normalizedName = schemaResolver.normalizeCaseObjectName(database, name);
    if (type == Type::UNKNOWN)
        type = getObjectType(database, normalizedName);

    switch (type)
    {
        case Type::TABLE:
            editTable(database, normalizedName);
            break;
        case Type::INDEX:
            editIndex(normalizedName);
            break;
        case Type::TRIGGER:
            editTrigger(normalizedName);
            break;
        case Type::VIEW:
            editView(database, normalizedName);
            break;
        default:
        {
            qCritical() << "Unknown object type while trying to edit object. Object name:" << database << "." << name;
            return;
        }
    }
}

bool DbObjectDialogs::dropObject(Type type, const QString& name)
{
    return dropObject(type, "main", name);
}

bool DbObjectDialogs::dropObject(Type type, const QString& database, const QString& name)
{
    static const QString dropSql3 = "DROP %1 %2.%3;";

    QString dbName = wrapObjIfNeeded(database);

    if (type == Type::UNKNOWN)
        type = getObjectType(database, name);

    QString title;
    QString message;
    QString typeForSql;
    switch (type)
    {
        case Type::TABLE:
            title = tr("Delete table");
            message = tr("Are you sure you want to delete table %1?");
            typeForSql = "TABLE";
            break;
        case Type::INDEX:
            title = tr("Delete index");
            message = tr("Are you sure you want to delete index %1?");
            typeForSql = "INDEX";
            break;
        case Type::TRIGGER:
            title = tr("Delete trigger");
            message = tr("Are you sure you want to delete trigger %1?");
            typeForSql = "TRIGGER";
            break;
        case Type::VIEW:
            title = tr("Delete view");
            message = tr("Are you sure you want to delete view %1?");
            typeForSql = "VIEW";
            break;
        default:
        {
            qCritical() << "Unknown object type while trying to drop object. Object name:" << database << "." << name;
            return false;
        }
    }

    if (!noConfirmation)
    {
        QMessageBox::StandardButton resp = QMessageBox::question(parentWidget, title, message.arg(name));
        if (resp != QMessageBox::Yes)
            return false;
    }

    SqlQueryPtr results;

    QString finalSql = dropSql3.arg(typeForSql, dbName, wrapObjIfNeeded(name));

    results = db->exec(finalSql);
    if (results->isError())
    {
        notifyError(tr("Error while dropping %1: %2").arg(name, results->getErrorText()));
        qCritical() << "Error while dropping object " << database << "." << name << ":" << results->getErrorText();
        return false;
    }

    CFG->addDdlHistory(finalSql, db->getName(), db->getPath());
    if (!noSchemaRefreshing)
        DBTREE->refreshSchema(db);

    return true;
}

bool DbObjectDialogs::dropObjects(const QStringList& names)
{
    QHash<QString, QStringList> objects;
    objects["main"] = names;
    return dropObjects(objects);
}

QHash<QString, QHash<QString, QStringList>> DbObjectDialogs::groupObjects(const QHash<QString, QStringList>& objects)
{
    QHash<QString, QHash<QString, QStringList>> groupedObjects;
    for (QHash<QString, QStringList>::const_iterator it = objects.begin(); it != objects.end(); ++it)
    {
        for (const QString& name : it.value())
        {
            QString typeForSql;
            switch (getObjectType(it.key(), name))
            {
                case Type::TABLE:
                    typeForSql = "TABLE";
                    break;
                case Type::INDEX:
                    typeForSql = "INDEX";
                    break;
                case Type::TRIGGER:
                    typeForSql = "TRIGGER";
                    break;
                case Type::VIEW:
                    typeForSql = "VIEW";
                    break;
                default:
                {
                    qCritical() << "Unknown object type while trying to drop object. Object name:" << it.key() << "." << name;
                    return QHash<QString, QHash<QString, QStringList>>();
                }
            }
            groupedObjects[it.key()][typeForSql] << name;
        }
    }
    return groupedObjects;
}

bool DbObjectDialogs::dropObjects(const QHash<QString, QStringList>& objects)
{
    static const QString dropSql2 = "DROP %1 IF EXISTS %2;";
    static const QString dropSql3 = "DROP %1 IF EXISTS %2.%3;";

    QStringList names = concat(objects.values());
    QHash<QString, QHash<QString, QStringList>> groupedObjects = groupObjects(objects);

    if (!noConfirmation)
    {
        QMessageBox::StandardButton resp = QMessageBox::question(parentWidget, tr("Delete objects"),
                                                                 tr("Are you sure you want to delete following objects:\n%1").arg(names.join(", ")));
        if (resp != QMessageBox::Yes)
            return false;
    }

    if (!db->begin())
    {
        notifyError(tr("Cannot start transaction. Details: %1").arg(db->getErrorText()));
        return false;
    }

    // Iterate through dbNames, then through db object types and finally through object names. Drop them.
    SqlQueryPtr results;
    QString finalSql;
    QString dbName;
    QHash<QString, QStringList> typeToNames;
    for (QHash<QString, QHash<QString, QStringList>>::const_iterator dbIt = groupedObjects.begin(); dbIt != groupedObjects.end(); ++dbIt)
    {
        dbName = wrapObjIfNeeded(dbIt.key());
        typeToNames = dbIt.value();
        for (QHash<QString, QStringList>::const_iterator typeIt = typeToNames.begin(); typeIt != typeToNames.end(); ++typeIt)
        {
            for (const QString& name : typeIt.value())
            {
                finalSql = dropSql3.arg(typeIt.key(), dbName, wrapObjIfNeeded(name));

                results = db->exec(finalSql);
                if (results->isError())
                {
                    notifyError(tr("Error while dropping %1: %2").arg(name).arg(results->getErrorText()));
                    qCritical() << "Error while dropping object " << dbIt.key() << "." << name << ":" << results->getErrorText();
                    return false;
                }

                CFG->addDdlHistory(finalSql, db->getName(), db->getPath());
            }
        }
    }

    if (!db->commit())
    {
        notifyError(tr("Cannot commit transaction. Details: %1").arg(db->getErrorText()));
        return false;
    }

    if (!noSchemaRefreshing)
        DBTREE->refreshSchema(db);

    return true;
}

DbObjectDialogs::Type DbObjectDialogs::getObjectType(const QString& database, const QString& name)
{
    static const QString typeSql = "SELECT type FROM %1.sqlite_master WHERE name = ?;";
    static const QStringList types = {"table", "index", "trigger", "view"};

    QString dbName = wrapObjIfNeeded(database);
    SqlQueryPtr results = db->exec(typeSql.arg(dbName), {name});
    if (results->isError())
    {
        qCritical() << "Could not get object type. Object name:" << database << "." << name << ", error:"
                    << results->getErrorText();
        return Type::UNKNOWN;
    }

    QString typeStr = results->getSingleCell().toString();
    return static_cast<Type>(types.indexOf(typeStr));
}
bool DbObjectDialogs::getNoSchemaRefreshing() const
{
    return noSchemaRefreshing;
}

void DbObjectDialogs::setNoSchemaRefreshing(bool value)
{
    noSchemaRefreshing = value;
}

bool DbObjectDialogs::getNoConfirmation() const
{
    return noConfirmation;
}

void DbObjectDialogs::setNoConfirmation(bool value)
{
    noConfirmation = value;
}


TableWindow* DbObjectDialogs::editTable(const QString& database, const QString& table)
{
    TableWindow* win = nullptr;
    for (MdiWindow*& mdiWin : mdiArea->getWindows())
    {
        win = dynamic_cast<TableWindow*>(mdiWin->getMdiChild());
        if (!win)
            continue;

        if (win->getDb() == db && win->getTable() == table)
        {
            mdiArea->setActiveSubWindow(mdiWin);
            return win;
        }
    }

    win = new TableWindow(mdiArea, db, database, table);
    if (win->isInvalid())
    {
        delete win;
        return nullptr;
    }

    mdiArea->addSubWindow(win);
    return win;
}

TableWindow *DbObjectDialogs::addTableSimilarTo(const QString &database, const QString &table)
{
    TableWindow* win = new TableWindow(mdiArea, db, database, table);
    mdiArea->addSubWindow(win);
    win->useCurrentTableAsBaseForNew();
    return win;
}

TableWindow* DbObjectDialogs::addTable()
{
    TableWindow* win = new TableWindow(db, mdiArea);
    mdiArea->addSubWindow(win);
    return win;
}
