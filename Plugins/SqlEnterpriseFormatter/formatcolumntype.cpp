#include "formatcolumntype.h"
#include "parser/ast/sqlitecolumntype.h"

FormatColumnType::FormatColumnType(SqliteColumnType* colType) :
    colType(colType)
{
}

void FormatColumnType::formatInternal()
{
    if (colType->name.isEmpty())
        return;

    withId(colType->name);

    if (!colType->scale.isNull())
    {
        withParExprLeft();
        if (colType->scale.userType() == QVariant::Int)
            withInteger(colType->scale.toInt());
        else if (colType->scale.userType() == QVariant::LongLong)
            withInteger(colType->scale.toLongLong());
        else if (colType->scale.userType() == QVariant::Double)
            withFloat(colType->scale.toDouble());
        else
            withId(colType->scale.toString());

        if (!colType->precision.isNull())
        {
            withCommaOper();
            if (colType->precision.userType() == QVariant::Int)
                withInteger(colType->precision.toInt());
            else if (colType->precision.userType() == QVariant::LongLong)
                withInteger(colType->precision.toLongLong());
            else if (colType->precision.userType() == QVariant::Double)
                withFloat(colType->precision.toDouble());
            else
                withId(colType->precision.toString());
        }
        withParExprRight();
    }
}
