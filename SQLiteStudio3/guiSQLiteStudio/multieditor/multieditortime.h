#ifndef MULTIEDITORTIME_H
#define MULTIEDITORTIME_H

#include "multieditordatetime.h"
#include "guiSQLiteStudio_global.h"

class GUI_API_EXPORT MultiEditorTime : public MultiEditorDateTime
{
    public:
        explicit MultiEditorTime(QWidget *parent = 0);

        QString getTabLabel();

        static void staticInit();

    protected:
        QStringList getParsingFormats();

    private:
        static QStringList formats;
};

class GUI_API_EXPORT MultiEditorTimePlugin : public BuiltInPlugin, public MultiEditorWidgetPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")
    SQLITESTUDIO_PLUGIN_DESC("Time data editor.")
    SQLITESTUDIO_PLUGIN_TITLE("Time")
    SQLITESTUDIO_PLUGIN_VERSION(10000)

    public:
        MultiEditorWidget* getInstance();
        bool validFor(const DataType& dataType);
        int getPriority(const DataType& dataType);
};

#endif // MULTIEDITORTIME_H
