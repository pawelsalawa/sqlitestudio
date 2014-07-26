#ifndef CODEFORMATTER_H
#define CODEFORMATTER_H

#include "coreSQLiteStudio_global.h"
#include "sqlitestudio.h"

class CodeFormatterPlugin;
class Db;

class API_EXPORT CodeFormatter
{
    public:
        QString format(const QString& lang, const QString& code, Db* contextDb);

        void setFormatter(const QString& lang, CodeFormatterPlugin* formatterPlugin);
        CodeFormatterPlugin* getFormatter(const QString& lang);
        bool hasFormatter(const QString& lang);
        void fullUpdate();
        void updateCurrent();

    private:
        QHash<QString,QHash<QString,CodeFormatterPlugin*>> availableFormatters;
        QHash<QString,CodeFormatterPlugin*> currentFormatter;
        bool modifyingConfig = false;
};

#define FORMATTER SQLITESTUDIO->getCodeFormatter()

#endif // CODEFORMATTER_H
