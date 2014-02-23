#ifndef SQLRESULTSROWQT_H
#define SQLRESULTSROWQT_H

#include "sqlresultsrow.h"
#include <QString>
#include <QVariant>

/**
 * @brief The SqlResultsRow implemented with Qt SQL classes
 */
class SqlResultsRowQt : public SqlResultsRow
{
    public:
        /**
         * @brief Defines value for given column.
         * @param column Column to define value for.
         * @param value Value to be defined.
         */
        void setValue(const QString& column, const QVariant& value);
};

#endif // SQLRESULTSROWQT_H
