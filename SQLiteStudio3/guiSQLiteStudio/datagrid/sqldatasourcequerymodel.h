#ifndef SQLDATASOURCEQUERYMODEL_H
#define SQLDATASOURCEQUERYMODEL_H

#include "sqlquerymodel.h"

class SqlDataSourceQueryModel : public SqlQueryModel
{
    public:
        explicit SqlDataSourceQueryModel(QObject *parent = 0);

        QString getDatabase() const;
        Features features() const;
        void updateTablesInUse(const QString& inUse);

        void applySqlFilter(const QString& value);
        void applyStringFilter(const QString& value);
        void applyStringFilter(const QStringList& values);
        void applyRegExpFilter(const QString& value);
        void applyRegExpFilter(const QStringList& values);
        void applyStrictFilter(const QString& value);
        void applyStrictFilter(const QStringList& values);
        void resetFilter();

    protected:
        typedef std::function<QString(const QString&)> FilterValueProcessor;

        static QString stringFilterValueProcessor(const QString& value);
        static QString strictFilterValueProcessor(const QString& value);
        static QString regExpFilterValueProcessor(const QString& value);

        void applyFilter(const QString& value, FilterValueProcessor valueProc);
        void applyFilter(const QStringList& values, FilterValueProcessor valueProc);

        QString getDatabasePrefix();

        /**
         * @brief Get the data source for this object.
         * Default implementation returns an empty string. Working implementation
         * (i.e. for a table) should return the data source string.
         */
        virtual QString getDataSource();

        QString database;
};

#endif // SQLDATASOURCEQUERYMODEL_H
