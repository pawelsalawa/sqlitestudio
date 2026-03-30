#ifndef SQLQUERYMODELCOLUMN_H
#define SQLQUERYMODELCOLUMN_H

#include "db/queryexecutor.h"
#include "parser/ast/sqlitecreatetable.h"
#include "datatype.h"
#include "common/global.h"
#include "guiSQLiteStudio_global.h"

class CellRendererPlugin;
class Icon;

class GUI_API_EXPORT SqlQueryModelColumn
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
            DISTINCT_RESULTS,
            RESULT_INLINE_SUBSELECT,
            COMMON_TABLE_EXPRESSION,
            GENERATED_COLUMN,
            VIEW_NOT_EXPANDED
        };

        struct Constraint
        {
            enum class Type : unsigned int
            {
                PRIMARY_KEY  = 0x0001,
                NOT_NULL     = 0x0002,
                UNIQUE       = 0x0004,
                CHECK        = 0x0008,
                DEFAULT      = 0x0010,
                COLLATE      = 0x0020,
                GENERATED    = 0x0040,
                FOREIGN_KEY  = 0x0080,
                // Meta types below (not directly corresponding to SQLite constraints, but useful for edition and display purposes)
                _AUTOINCR    = 0x1000,
                _ROWID_PK    = 0x2000,
                null         = 0x0000
            };
            Q_DECLARE_FLAGS(Types, Type)

            enum class Scope
            {
                TABLE,
                COLUMN
            };

            virtual ~Constraint() {}

            static Constraint* create(const QString& column, SqliteCreateTable::Constraint* tableConstraint);
            static Constraint* create(const QString& column, SqliteCreateTable::ConstraintPtr tableConstraint);
            static Constraint* create(SqliteCreateTable::Column::ConstraintPtr columnConstraint);
            static Constraint* create(SqliteCreateTable::Column::Constraint* columnConstraint);

            virtual QString getTypeString() const = 0;
            virtual QString getDetails() const = 0;
            virtual Icon* getIcon() const = 0;


            Type type;
            Scope scope;
            QString definition;
        };

        struct ConstraintPk : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            Icon* getIcon() const;

            bool autoIncrement;
            QStringList multiColumns;
            SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        };

        struct ConstraintFk : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            Icon* getIcon() const;

            QString foreignTable;
            QString foreignColumn;
        };

        struct ConstraintUnique : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            Icon* getIcon() const;

            SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        };

        struct ConstraintNotNull : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            Icon* getIcon() const;

            SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        };

        struct ConstraintDefault : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            Icon* getIcon() const;

            QString defaultValue;
        };

        struct ConstraintCheck : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            Icon* getIcon() const;

            QString condition;
            SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        };

        struct ConstraintCollate : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            Icon* getIcon() const;

            QString collationName;
        };

        struct ConstraintGenerated : public Constraint
        {
            QString getTypeString() const;
            QString getDetails() const;
            Icon* getIcon() const;

            QString expr;
            SqliteCreateTable::Column::Constraint::GeneratedType generatedType = SqliteCreateTable::Column::Constraint::GeneratedType::null;
        };

        SqlQueryModelColumn(const QueryExecutor::ResultColumnPtr& resultColumn);
        virtual ~SqlQueryModelColumn();

        static void initMeta();
        static EditionForbiddenReason convert(QueryExecutor::EditionForbiddenReason reason);
        static EditionForbiddenReason convert(QueryExecutor::ColumnEditionForbiddenReason reason);
        static QString resolveMessage(EditionForbiddenReason reason);
        void postProcessConstraints();
        bool isNumeric();
        bool isNull();
        bool canEdit();
        QString getEditionForbiddenReason();
        bool isPk() const;
        bool isRowIdPk() const;
        bool isAutoIncr() const;
        bool isNotNull() const;
        bool isUnique() const;
        bool isFk() const;
        bool isDefault() const;
        bool isCollate() const;
        bool isGenerated() const;
        Constraint::Types getConstraintsTypes() const;
        QList<Constraint*> getConstraints() const;
        void addConstraint(Constraint* constraint);
        void clearConstraints();
        bool hasDefaultValueForInsert() const;
        QString getAliasedName() const;
        QList<ConstraintFk*> getFkConstraints() const;
        ConstraintDefault* getDefaultConstraint() const;
        AliasedTable getAliasedTable() const;
        QString bestEffortIdentifier() const;

        QString displayName;
        QString column;
        QString alias;
        QString table;
        QString database;
        QString tableAlias;
        DataType dataType;
        QSet<EditionForbiddenReason> editionForbiddenReason;
        QString queryExecutorAlias;

    private:
        template <class T>
        QList<T> getTypedConstraints() const;

        QList<Constraint*> constraints;
        Constraint::Types constraintsTypes = Constraint::Type::null;
};

typedef QSharedPointer<SqlQueryModelColumn> SqlQueryModelColumnPtr;

size_t qHash(SqlQueryModelColumn::EditionForbiddenReason reason);

QDataStream &operator<<(QDataStream &out, const SqlQueryModelColumn* col);
QDataStream &operator>>(QDataStream &in, SqlQueryModelColumn*& col);

Q_DECLARE_METATYPE(SqlQueryModelColumn*)

Q_DECLARE_OPERATORS_FOR_FLAGS(SqlQueryModelColumn::Constraint::Types)

#endif // SQLQUERYMODELCOLUMN_H
