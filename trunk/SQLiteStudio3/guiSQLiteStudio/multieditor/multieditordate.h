#ifndef MULTIEDITORDATE_H
#define MULTIEDITORDATE_H

#include "multieditordatetime.h"

class GUI_API_EXPORT MultiEditorDate : public MultiEditorDateTime
{
        Q_OBJECT

    public:
        explicit MultiEditorDate(QWidget *parent = 0);

        QString getTabLabel();

        static void staticInit();

    protected:
        QStringList getParsingFormats();

    private:
        static QStringList formats;
};

class GUI_API_EXPORT MultiEditorDatePlugin : public BuiltInPlugin, public MultiEditorWidgetPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")
    SQLITESTUDIO_PLUGIN_DESC("Date data editor.")
    SQLITESTUDIO_PLUGIN_TITLE("Date")
    SQLITESTUDIO_PLUGIN_VERSION(10000)

    public:
        MultiEditorWidget* getInstance();
        bool validFor(const DataType& dataType);
        int getPriority(const DataType& dataType);
};

#endif // MULTIEDITORDATE_H
