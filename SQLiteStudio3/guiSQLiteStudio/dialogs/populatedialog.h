#ifndef POPULATEDIALOG_H
#define POPULATEDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>
#include <QVariant>
#include <QHash>

namespace Ui {
    class PopulateDialog;
}

class PopulatePlugin;
class PopulateEngine;
class QGridLayout;
class DbListModel;
class DbObjListModel;
class Db;
class QComboBox;
class QCheckBox;
class QToolButton;
class QSignalMapper;
class WidgetCover;

class GUI_API_EXPORT PopulateDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit PopulateDialog(QWidget *parent = 0);
        ~PopulateDialog();
        void setDbAndTable(Db* db, const QString& table);

    private:
        struct GUI_API_EXPORT ColumnEntry
        {
            ColumnEntry(const QString& column, QCheckBox* check, QComboBox* combo, QToolButton* button);
            ~ColumnEntry();

            QString column;
            QCheckBox* check = nullptr;
            QComboBox* combo = nullptr;
            QToolButton* button = nullptr;
            PopulateEngine* engine = nullptr;
            PopulatePlugin* plugin = nullptr;
        };

        void init();
        PopulateEngine* getEngine(int selectedPluginIndex);
        void deleteEngines(const QList<PopulateEngine*>& engines);
        void rebuildEngines(const QHash<QString, QPair<QString, QVariant> >& columnConfig);

        Ui::PopulateDialog *ui = nullptr;
        QGridLayout* columnsGrid = nullptr;
        DbListModel* dbListModel = nullptr;
        DbObjListModel* tablesModel = nullptr;
        Db* db = nullptr;
        QStringList pluginTitles;
        QList<PopulatePlugin*> plugins;
        QHash<QString,PopulatePlugin*> pluginByName;
        QList<ColumnEntry> columnEntries;
        QSignalMapper* checkMapper = nullptr;
        QSignalMapper* buttonMapper = nullptr;
        QHash<int,bool> columnsValid;
        WidgetCover* widgetCover = nullptr;
        bool started = false;

    private slots:
        void refreshTables();
        void refreshColumns();
        void pluginSelected(int index);
        void pluginSelected(QComboBox* combo, int index, const QVariant& config);
        void configurePlugin(int index);
        void updateColumnState(int index, bool updateGlobalState = true);
        void updateState();
        void finished();

    public:
        void accept();
        void reject();
};

#endif // POPULATEDIALOG_H
