#ifndef CONFIGMIGRATIONITEM_H
#define CONFIGMIGRATIONITEM_H

struct
{
    enum class Type
    {
        COLORS,
        SQL_HISTORY,
        DATABASES,
        FUNCTIONS
    };

    QString label;
    Type type;
};

#endif // CONFIGMIGRATIONITEM_H
