#ifndef CONSTRAINTPANEL_H
#define CONSTRAINTPANEL_H

#include "db/db.h"
#include "parser/ast/sqlitecreatetable.h"
#include "guiSQLiteStudio_global.h"
#include <QWidget>
#include <QPointer>

class GUI_API_EXPORT ConstraintPanel : public QWidget
{
        Q_OBJECT

    public:
        explicit ConstraintPanel(QWidget *parent = 0);
        virtual ~ConstraintPanel();

        void setConstraint(SqliteStatement* stmt);
        void setCreateTableStmt(SqliteCreateTable* stmt);
        void setColumnStmt(SqliteCreateTable::Column* stmt);
        void storeDefinition();
        virtual void setDb(Db* value);

        /**
         * @brief validate Validates panel for correct data filled in.
         * @return true if the data is valid, or false otherwise.
         * Apart from returning boolean result it also marks
         * invalid fields with red color. See validateOnly() description
         * for details on differences between those two methods.
         */
        virtual bool validate() = 0;

        /**
         * @brief validateOnly Validates panel for correct data filled in.
         * @return true if the data is valid, or false otherwise.
         * The difference between validateOnly() and validate() is that validateOnly()
         * will run all validations immediately (ie. SQL syntax checking
         * in DEFAULT constraint, etc), while the validate() will wait for
         * SqlEditor to do the validation in its scheduled time and return
         * false until the validation isn't done yet.
         * The validate() should be used when user actually edits the panel,
         * while validateOnly() is to be used when using ConstraintPanel for validation
         * of a Constraint object, but not displayed - in that case the validation
         * result is needed immediately and that's where validateOnly() does its job.
         * Not every constraint panel has to reimplement this. Most of the constraints
         * don't work asynchronously and return proper result just from a validate() call.
         * In that case the default implementation of validateOnly() will do the job.
         */
        virtual bool validateOnly();

        static ConstraintPanel* produce(SqliteCreateTable::Constraint* constr);
        static ConstraintPanel* produce(SqliteCreateTable::Column::Constraint* constr);

    protected:
        /**
         * @brief constraintAvailable
         * This method is called once the constraint object (the member variable)
         * is available to the panel as well, as the database (the db member).
         * The implementation should read values from constraint object and put them
         * to panel's fields, but also initialise any database related data,
         * like existing collations, etc.
         */
        virtual void constraintAvailable() = 0;

        /**
         * @brief storeConfiguration
         * The implementation should store all field valies into the constraint object.
         */
        virtual void storeConfiguration() = 0;

        Db* db = nullptr;
        QPointer<SqliteStatement> constraint;
        QPointer<SqliteCreateTable> createTableStmt;
        QPointer<SqliteCreateTable::Column> columnStmt;

    public slots:

    signals:
        void updateValidation();

};

#endif // CONSTRAINTPANEL_H
