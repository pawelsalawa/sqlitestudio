#ifndef SQLITECOLUMNTYPE_H
#define SQLITECOLUMNTYPE_H

#include "sqlitestatement.h"
#include "datatype.h"
#include <QVariant>

class API_EXPORT SqliteColumnType : public SqliteStatement
{
    Q_OBJECT

    public:
        SqliteColumnType();
        SqliteColumnType(const SqliteColumnType& other);
        explicit SqliteColumnType(const QString& name);
        SqliteColumnType(const QString& name, const QVariant &scale);
        SqliteColumnType(const QString& name, const QVariant &scale, const QVariant &precision);
        SqliteStatement* clone();

        bool isPrecisionDouble();
        bool isScaleDouble();
        TokenList rebuildTokensFromContents();
        DataType toDataType() const;

        QString name = QString();
        QVariant scale = QVariant(); // first size number
        QVariant precision = QVariant(); // second size number
};

typedef QSharedPointer<SqliteColumnType> SqliteColumnTypePtr;

#endif // SQLITECOLUMNTYPE_H
