#ifndef SQLQUERYMODELCOLUMN_H
#define SQLQUERYMODELCOLUMN_H

#include "db/queryexecutor.h"
#include "parser/ast/sqlitecreatetable.h"
#include "datatype.h"
#include "global.h"

class SqlQueryModelColumn
{
    public:
        /**
         * @brief The EditionForbiddenReason enum
         * Order of this enum is important, because when user requests item edition,
         * then reason for edition forbidden (if any) is taken as specified order.
         * The earlier item is in the enum, the more significant it is and user
         * will be notified with the more significant reason before any other.
         */
        enum class EditionForbiddenReason
        {
            SYSTEM_TABLE,
            NOT_A_SELECT,
            COMPOUND_SELECT,
            GROUPED_RESULTS,
            EXPRESSION,
            SMART_EXECUTION_FAILED,
            DISTINCT_RESULTS
        };

        class DataType
        {
            public:
                DataType();
                DataType(const QString& type, const QVariant& precision, const QVariant& scale);
                DataType(const DataType& other);
                void setEmpty();
                QString toString();

                ::DataType::Enum type = ::DataType::_NULL;
                QString typeStr = "";
                QVariant precision;
                QVariant scale;
        };

        struct Constraint
        {
            enum class Type
            {
                PRIMARY_KEY,
                NOT_NULL,
                UNIQUE,
                CHECK,
                DEFAULT,
                COLLATE,
                FOREIGN_KEY,
                null
            };

            enum class Scope
            {
                TABLE,
                COLUMN
            };

            virtual ~Constraint() {}

            static Constraint* create(const QString& column, SqliteCreateTable::ConstraintPtr tableConstraint);
            static Constraint* create(SqliteCreateTable::Column::ConstraintPtr columnConstraint);

            virtual QString getTypeString() const = 0;
            virtual QString getDetails() const = 0;
            virtual QString getIconName() const = 0;

            Type type;
            Scope scope;
            QString definition;
        };

        struct ConstraintPk : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            QString getIconName() const;

            bool autoIncrement;
            SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        };

        struct ConstraintFk : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            QString getIconName() const;

            QString foreignTable;
            QString foreignColumn;
        };

        struct ConstraintUnique : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            QString getIconName() const;

            SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        };

        struct ConstraintNotNull : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            QString getIconName() const;

            SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        };

        struct ConstraintDefault : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            QString getIconName() const;

            QString defaultValue;
        };

        struct ConstraintCheck : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            QString getIconName() const;

            QString condition;
            SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        };

        struct ConstraintCollate : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            QString getIconName() const;

            QString collationName;
        };

        SqlQueryModelColumn(const QueryExecutor::ResultColumnPtr& resultColumn);
        virtual ~SqlQueryModelColumn();

        static void initMeta();
        static EditionForbiddenReason convert(QueryExecutor::EditionForbiddenReason reason);
        static EditionForbiddenReason convert(QueryExecutor::ColumnEditionForbiddenReason reason);
        static QString resolveMessage(EditionForbiddenReason reason);
        bool isNumeric();
        bool canEdit();
        QString getEditionForbiddenReason();
        bool isPk() const;
        bool isAutoIncr() const;
        bool isNotNull() const;
        bool isUnique() const;
        bool isFk() const;
        bool isDefault() const;
        bool isCollate() const;
        QList<ConstraintFk*> getFkConstraints() const;
        ConstraintDefault* getDefaultConstraint() const;

        QString displayName;
        QString column;
        QString table;
        QString database;
        DataType dataType;
        QSet<EditionForbiddenReason> editionForbiddenReason;
        QList<Constraint*> constraints;

    private:
        template <class T>
        QList<T> getConstraints() const;
};

typedef QSharedPointer<SqlQueryModelColumn> SqlQueryModelColumnPtr;

int qHash(SqlQueryModelColumn::EditionForbiddenReason reason);

QDataStream &operator<<(QDataStream &out, const SqlQueryModelColumn* col);
QDataStream &operator>>(QDataStream &in, SqlQueryModelColumn*& col);

Q_DECLARE_METATYPE(SqlQueryModelColumn*)

#endif // SQLQUERYMODELCOLUMN_H
