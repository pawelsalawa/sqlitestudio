#ifndef DBOBJECTDIALOGS_H
#define DBOBJECTDIALOGS_H

#include "db/db.h"
#include "guiSQLiteStudio_global.h"
#include <QString>
#include <QStringList>

class QWidget;
class MainWindow;
class MdiArea;
class TableWindow;
class ViewWindow;

class GUI_API_EXPORT DbObjectDialogs : public QObject
{
        Q_OBJECT

    public:
        enum class Type
        {
            TABLE = 0,
            INDEX = 1,
            TRIGGER = 2,
            VIEW = 3,
            UNKNOWN = -1
        };

        explicit DbObjectDialogs(Db* db);
        DbObjectDialogs(Db* db, QWidget* parentWidget);

        TableWindow* addTable();
        TableWindow* editTable(const QString& database, const QString& table);
        TableWindow* addTableSimilarTo(const QString& database, const QString& table);

        void addIndex(const QString& table);
        void editIndex(const QString& index);

        void addTriggerOnTable(const QString& table);
        void addTriggerOnView(const QString& view);
        void addTrigger(const QString& table, const QString& view);
        void editTrigger(const QString& trigger);

        ViewWindow* addView(const QString& initialSelect = QString());
        ViewWindow* editView(const QString& database, const QString& view);

        void editObject(Type type, const QString& name);
        void editObject(Type type, const QString& database, const QString& name);
        bool dropObject(Type type, const QString& name);
        bool dropObject(Type type, const QString& database, const QString& name);
        bool dropObjects(const QStringList& names);
        bool dropObjects(const QHash<QString, QStringList>& objects);

        bool getNoConfirmation() const;
        void setNoConfirmation(bool value);

        bool getNoSchemaRefreshing() const;
        void setNoSchemaRefreshing(bool value);

    private:
        Type getObjectType(const QString& database, const QString& name);
        QHash<QString, QHash<QString, QStringList> > groupObjects(const QHash<QString, QStringList>& objects);

        Db* db = nullptr;
        QWidget* parentWidget = nullptr;
        MainWindow* mainWindow = nullptr;
        MdiArea* mdiArea = nullptr;
        bool noConfirmation = false;
        bool noSchemaRefreshing = false;
};

#endif // DBOBJECTDIALOGS_H
