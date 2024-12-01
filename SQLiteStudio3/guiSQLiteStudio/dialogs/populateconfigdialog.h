#ifndef POPULATECONFIGDIALOG_H
#define POPULATECONFIGDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>
#include <QHash>

class PopulateEngine;
class ConfigMapper;
class CfgEntry;

namespace Ui {
    class PopulateConfigDialog;
}

class GUI_API_EXPORT PopulateConfigDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit PopulateConfigDialog(PopulateEngine* engine, const QString& column, const QString& pluginName, QWidget *parent = 0);
        ~PopulateConfigDialog();

        int exec();

    protected:
        void showEvent(QShowEvent* e);

    private:
        void init();

        Ui::PopulateConfigDialog *ui = nullptr;
        PopulateEngine* engine = nullptr;
        ConfigMapper* configMapper = nullptr;
        QHash<CfgEntry*,bool> pluginConfigOk;
        QString column;
        QString pluginName;
        QWidget* innerWidget = nullptr;

    private slots:
        void validateEngine();
        void validationResultFromPlugin(bool valid, CfgEntry* key, const QString& msg);
        void stateUpdateRequestFromPlugin(CfgEntry* key, bool visible, bool enabled);
        void widgetPropertyFromPlugin(CfgEntry* key, const QString& propName, const QVariant& value);
        void updateState();
};

#endif // POPULATECONFIGDIALOG_H
