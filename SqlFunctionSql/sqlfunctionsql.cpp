#include "sqlfunctionsql.h"
#include "db/db.h"
#include "unused.h"
#include "pluginmanager.h"
#include <QDir>

QVariant SqlFunctionSql::evaluate(Db* db, const QString& function, const QString& code, const QList<QVariant>& args, bool& success)
{
    UNUSED(function);

    SqlResultsPtr results = db->exec(code, args, Db::Flag::NO_LOCK);
    if (results->isError())
    {
        success = false;
        return results->getErrorText();
    }

    success = true;
    return results->getSingleCell();
}

QString SqlFunctionSql::getLanguageName() const
{
    return "SQL";
}

QByteArray SqlFunctionSql::getIconData() const
{
    static const char* icon = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJ"
            "bWFnZVJlYWR5ccllPAAAARpJREFUeNrE0z9LQlEYx/Fr/gXdJILegSQJEk13yrUxxDFwUhqagyAE"
            "F8E34BsIFKlozpqao8K9WVrCJSWu30d+F44SOjj4wId77nPOeTjn3HMjQRB4m8SOt2Fsv0CE8Hle"
            "YoRdnGEPF8jhB4+4xw0ONLc7R4E7VJBX23JNtJBFSflD9YXjrT3fQhxpfKqqRQEf+MaLxuTVF3e3"
            "bo02yjjBrTMojD+9R52+sO3F8KxC13YmeELCHbT0vtBnE6ua1EfDGZRQO7qiQNEK1LiNRzynSKpj"
            "CJ+8fY1T5d/VZ+2Y5nQs8YVXPW01+zjGg3JvuFK+q1yoZ3tOaVnJpcOzqOtwzzF28rbaX0zWXbQM"
            "Bvqc/r83cet/40yAAQCHjz1eQkhXqAAAAABJRU5ErkJggg==";

    return QByteArray(icon);
}
