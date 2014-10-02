#ifndef DBDIALOG_H
#define DBDIALOG_H

#include "db/db.h"
#include "db/dbpluginoption.h"
#include "guiSQLiteStudio_global.h"
#include <QDialog>
#include <QList>
#include <QHash>
#include <QStringList>

class DbPlugin;
class QGridLayout;
struct DbPluginOption;

namespace Ui {
    class DbDialog;
}

class GUI_API_EXPORT DbDialog : public QDialog
{
    Q_OBJECT

    public:
        enum Mode
        {
            ADD,
            EDIT
        };

        DbDialog(Mode mode, QWidget *parent = 0);
        ~DbDialog();

        void setDb(Db* db);
        void setPermanent(bool perm);

        QString getPath();
        void setPath(const QString& path);
        QString getName();
        QHash<QString,QVariant> collectOptions();
        bool isPermanent();

    protected:
        void changeEvent(QEvent *e);
        void showEvent(QShowEvent* e);

    private:
        void init();
        void updateOptions();
        void addOption(const DbPluginOption& option, int row);
        QWidget* getEditor(DbPluginOption::Type type, QWidget *&editorHelper);
        QVariant getValueFrom(DbPluginOption::Type type, QWidget* editor);
        void setValueFor(DbPluginOption::Type type, QWidget* editor, const QVariant& value);
        void updateType();
        Db* getDb();
        bool testDatabase();
        bool validate();
        void updateState();

        Ui::DbDialog *ui;
        Mode mode;
        QStringList existingDatabaseNames;
        Db* db = nullptr;
        QList<DbPlugin*> dbPlugins;
        QList<QWidget*> optionWidgets;
        QHash<QString,QWidget*> optionKeyToWidget;
        QHash<QString,DbPluginOption::Type> optionKeyToType;
        QHash<QWidget*,QString> helperToKey;

    private slots:
        void typeChanged(int index);
        void valueForNameGenerationChanged();
        void browseForFile();
        void generateNameSwitched(bool checked);
        void fileChanged(const QString &arg1);
        void browseClicked();
        void testConnectionClicked();
        void propertyChanged();
        void dbTypeChanged(int index);
        void nameModified(const QString &arg1);

    public slots:
        void accept();
};

#endif // DBDIALOG_H
