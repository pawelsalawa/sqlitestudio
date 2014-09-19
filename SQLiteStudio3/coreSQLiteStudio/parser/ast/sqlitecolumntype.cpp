#include "sqlitecolumntype.h"
#include "parser/statementtokenbuilder.h"
#include "common/utils_sql.h"

SqliteColumnType::SqliteColumnType()
{
}

SqliteColumnType::SqliteColumnType(const SqliteColumnType& other) :
    SqliteStatement(other), name(other.name), scale(other.scale), precision(other.precision)
{
}

SqliteColumnType::SqliteColumnType(const QString &name)
{
    this->name = name;
}

SqliteColumnType::SqliteColumnType(const QString &name, const QVariant& scale)
{
    this->name = name;
    this->scale = scale;
}

SqliteColumnType::SqliteColumnType(const QString &name, const QVariant& scale, const QVariant& precision)
{
    this->name = name;
    this->precision = precision;
    this->scale = scale;
}

bool SqliteColumnType::isPrecisionDouble()
{
    return !precision.isNull() && precision.toString().indexOf(".") > -1;
}

bool SqliteColumnType::isScaleDouble()
{
    return !scale.isNull() && scale.toString().indexOf(".") > -1;
}

TokenList SqliteColumnType::rebuildTokensFromContents()
{
    StatementTokenBuilder builder;
    if (name.isEmpty())
        return TokenList();

    builder.withOther(name);

    if (!scale.isNull())
    {
        builder.withSpace().withParLeft();
        if (scale.userType() == QVariant::Int)
            builder.withInteger(scale.toInt());
        else if (scale.userType() == QVariant::LongLong)
            builder.withInteger(scale.toLongLong());
        else if (scale.userType() == QVariant::Double)
            builder.withFloat(scale.toDouble());
        else
            builder.withOther(scale.toString());

        if (!precision.isNull())
        {
            builder.withOperator(",").withSpace();
            if (precision.userType() == QVariant::Int)
                builder.withInteger(precision.toInt());
            else if (precision.userType() == QVariant::LongLong)
                builder.withInteger(precision.toLongLong());
            else if (precision.userType() == QVariant::Double)
                builder.withFloat(precision.toDouble());
            else
                builder.withOther(precision.toString());
        }
        builder.withParRight();
    }

    return builder.build();
}

DataType SqliteColumnType::toDataType() const
{
    return DataType(name, scale, precision);
}
