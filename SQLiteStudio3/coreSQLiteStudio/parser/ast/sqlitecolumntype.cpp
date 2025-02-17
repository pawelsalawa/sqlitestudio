#include "sqlitecolumntype.h"
#include "parser/statementtokenbuilder.h"
#include "common/utils_sql.h"
#include "parser/lexer.h"

SqliteColumnType::SqliteColumnType()
{
}

SqliteColumnType::SqliteColumnType(const SqliteColumnType& other) :
    SqliteStatement(other), name(other.name), scale(other.scale), precision(other.precision)
{
}

SqliteColumnType::SqliteColumnType(const QString &name) :
    SqliteColumnType()
{
    this->name = name;
}

SqliteColumnType::SqliteColumnType(const QString &name, const QVariant& scale) :
    SqliteColumnType(name)
{
    this->scale = scale;
}

SqliteColumnType::SqliteColumnType(const QString &name, const QVariant& scale, const QVariant& precision) :
    SqliteColumnType(name, scale)
{
    this->precision = precision;
}

SqliteStatement* SqliteColumnType::clone()
{
    return new SqliteColumnType(*this);
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

    TokenList resultTokens = Lexer::tokenize(name);

    if (!scale.isNull())
    {
        builder.withSpace().withParLeft();
        if (scale.userType() == QMetaType::Int)
            builder.withInteger(scale.toInt());
        else if (scale.userType() == QMetaType::LongLong)
            builder.withInteger(scale.toLongLong());
        else if (scale.userType() == QMetaType::Double)
            builder.withFloat(scale);
        else
            builder.withOther(scale.toString());

        if (!precision.isNull())
        {
            builder.withOperator(",").withSpace();
            if (precision.userType() == QMetaType::Int)
                builder.withInteger(precision.toInt());
            else if (precision.userType() == QMetaType::LongLong)
                builder.withInteger(precision.toLongLong());
            else if (precision.userType() == QMetaType::Double)
                builder.withFloat(precision);
            else
                builder.withOther(precision.toString());
        }
        builder.withParRight();
    }

    return resultTokens + builder.build();
}

DataType SqliteColumnType::toDataType() const
{
    return DataType(name, scale, precision);
}
