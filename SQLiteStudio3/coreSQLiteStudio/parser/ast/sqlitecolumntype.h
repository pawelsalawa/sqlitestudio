#ifndef SQLITECOLUMNTYPE_H
#define SQLITECOLUMNTYPE_H

#include "sqlitestatement.h"
#include <QVariant>

class API_EXPORT SqliteColumnType : public SqliteStatement
{
    public:
        SqliteColumnType();
        SqliteColumnType(const SqliteColumnType& other);
        explicit SqliteColumnType(const QString& name);
        SqliteColumnType(const QString& name, const QVariant &scale);
        SqliteColumnType(const QString& name, const QVariant &scale, const QVariant &precision);

        bool isPrecisionDouble();
        bool isScaleDouble();
        TokenList rebuildTokensFromContents();

        QString name = QString::null;
        QVariant scale = QVariant(); // first size number
        QVariant precision = QVariant(); // second size number
};

typedef QSharedPointer<SqliteColumnType> SqliteColumnTypePtr;

#endif // SQLITECOLUMNTYPE_H
