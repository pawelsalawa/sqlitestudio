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
    public:
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

        void editObject(const QString& name);
        void editObject(const QString& database, const QString& name);
        bool dropObject(const QString& name);
        bool dropObject(const QString& database, const QString& name);

        bool getNoConfirmation() const;
        void setNoConfirmation(bool value);

    private:
        enum class Type
        {
            TABLE = 0,
            INDEX = 1,
            TRIGGER = 2,
            VIEW = 3,
            UNKNOWN = -1
        };

        Type getObjectType(const QString& database, const QString& name);

        Db* db = nullptr;
        QWidget* parentWidget = nullptr;
        MainWindow* mainWindow = nullptr;
        MdiArea* mdiArea = nullptr;
        bool noConfirmation = false;
};

#endif // DBOBJECTDIALOGS_H
