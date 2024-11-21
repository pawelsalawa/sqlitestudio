#include "columngeneratedpanel.h"
#include "ui_columngeneratedpanel.h"
#include "parser/parser.h"
#include "parser/keywords.h"
#include "parser/lexer.h"
#include "uiutils.h"
#include "schemaresolver.h"
#include <QDebug>

ColumnGeneratedPanel::ColumnGeneratedPanel(QWidget *parent) :
    ConstraintPanel(parent),
    ui(new Ui::ColumnGeneratedPanel)
{
    ui->setupUi(this);
    init();
}

ColumnGeneratedPanel::~ColumnGeneratedPanel()
{
    delete ui;
}

void ColumnGeneratedPanel::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}


bool ColumnGeneratedPanel::validate()
{
    if (!ui->exprEdit->isSyntaxChecked())
    {
        setValidState(ui->exprEdit, false, tr("Enter the column value generating expression."));
        return false;
    }

    // First check if we already validated this text.
    // This method is called twice, by both errors checking and syntax highlighting,
    // because signal for textChange() is connected with call to updateValidation().
    QString text = ui->exprEdit->toPlainText();
    if (!lastValidatedText.isNull() && lastValidatedText == text)
        return lastValidationResult;

    lastValidatedText = text;

    bool nameOk = true;
    if (ui->namedCheck->isChecked() && ui->namedEdit->text().isEmpty())
        nameOk = false;

    bool exprOk = !ui->exprEdit->toPlainText().trimmed().isEmpty() &&
            !ui->exprEdit->haveErrors();

    QString exprError;
    if (exprOk)
    {
        // Everything looks fine, so lets do the final check - if the value is correct for use as generated column expression in SQLite.
        static QString tempDdlExprTpl = QStringLiteral("CREATE TEMP TABLE %1 (aaa, %2 GENERATED ALWAYS AS (%3));");
        static QString dropTempDdl = QStringLiteral("DROP TABLE %1;");

        SqliteCreateTable::Column* columnStmt = dynamic_cast<SqliteCreateTable::Column*>(constraint->parentStatement());
        SqliteCreateTable* createTableStmt = dynamic_cast<SqliteCreateTable*>(columnStmt->parentStatement());

        QStringList preparedColNames;
        QString thisColumnName = columnStmt->name;
        for (SqliteCreateTable::Column* column : createTableStmt->columns)
        {
            if (column->name == thisColumnName)
                continue;

            preparedColNames << wrapObjIfNeeded(column->name);
        }
        preparedColNames << wrapObjIfNeeded(thisColumnName);

        QString tableName = getTempTable();
        QString tempDdl = tempDdlExprTpl.arg(tableName, preparedColNames.join(", "), ui->exprEdit->toPlainText());
        SqlQueryPtr res = db->exec(tempDdl);
        if (res->isError())
        {
            exprOk = false;
            exprError = tr("Invalid value generating expression: %1.")
                    .arg(res->getErrorText());
        }

        db->exec(dropTempDdl.arg(tableName));
    } else
        exprError = tr("Invalid value generating expression.");

    setValidState(ui->exprEdit, exprOk, exprError);
    setValidState(ui->namedEdit, nameOk, tr("Enter a name of the constraint."));

    lastValidationResult = (exprOk && nameOk);
    return lastValidationResult;
}

bool ColumnGeneratedPanel::validateOnly()
{
    ui->exprEdit->checkSyntaxNow();
    return validate();
}

void ColumnGeneratedPanel::constraintAvailable()
{
    if (constraint.isNull())
        return;

    readConstraint();
    updateVirtualSql();
    validateOnly();
}

void ColumnGeneratedPanel::storeConfiguration()
{
    if (constraint.isNull())
        return;

    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Column::Constraint::GENERATED;
    storeExpr(constr);

    if (ui->typeCheck->isChecked())
        constr->generatedType = SqliteCreateTable::Column::Constraint::generatedTypeFrom(ui->typeCombo->currentText());
    else
        constr->generatedType = SqliteCreateTable::Column::Constraint::GeneratedType::null;

    constr->generatedKw = ui->generatedKwCheck->isChecked();

    if (ui->namedCheck->isChecked())
        constr->name = ui->namedEdit->text();
    else
        constr->name.clear();
}

void ColumnGeneratedPanel::storeExpr(SqliteCreateTable::Column::Constraint* constr)
{
    QString text = ui->exprEdit->toPlainText();
    clear(constr);

    Parser parser;
    SqliteExpr* newExpr = parser.parseExpr(text);
    newExpr->setParent(constraint.data());
    constr->expr = newExpr;
}

void ColumnGeneratedPanel::clear(SqliteCreateTable::Column::Constraint* constr)
{
    if (constr->expr)
    {
        delete constr->expr;
        constr->expr = nullptr;
    }
    constr->generatedKw = false;
    constr->generatedType = SqliteCreateTable::Column::Constraint::GeneratedType::null;
}

void ColumnGeneratedPanel::init()
{
    setFocusProxy(ui->exprEdit);
    ui->exprEdit->setShowLineNumbers(false);

    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SIGNAL(updateValidation()));
    connect(ui->namedEdit, SIGNAL(textChanged(QString)), this, SIGNAL(updateValidation()));
    connect(ui->exprEdit, SIGNAL(textChanged()), this, SIGNAL(updateValidation()));
    connect(ui->exprEdit, SIGNAL(errorsChecked(bool)), this, SIGNAL(updateValidation()));

    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(ui->typeCheck, SIGNAL(toggled(bool)), this, SLOT(updateState()));

    ui->typeCombo->addItems(getGeneratedColumnTypes());

    updateState();
}

void ColumnGeneratedPanel::readConstraint()
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());

    constr->rebuildTokens();
    if (constr->expr)
        ui->exprEdit->setPlainText(constr->expr->detokenize());

    QString typeValue = SqliteCreateTable::Column::Constraint::toString(constr->generatedType);
    switch (constr->generatedType) {
        case SqliteCreateTable::Column::Constraint::GeneratedType::STORED:
            ui->typeCheck->setChecked(true);
            break;
        case SqliteCreateTable::Column::Constraint::GeneratedType::VIRTUAL:
            ui->typeCheck->setChecked(true);
            break;
        case SqliteCreateTable::Column::Constraint::GeneratedType::null:
            ui->typeCheck->setChecked(false);
            typeValue = SqliteCreateTable::Column::Constraint::toString(SqliteCreateTable::Column::Constraint::GeneratedType::VIRTUAL);
            break;
    }
    ui->typeCombo->setCurrentText(typeValue);

    ui->generatedKwCheck->setChecked(constr->generatedKw);

    if (!constr->name.isNull())
    {
        ui->namedCheck->setChecked(true);
        ui->namedEdit->setText(constr->name);
    }
}

void ColumnGeneratedPanel::updateVirtualSql()
{
    static QString sql = QStringLiteral("CREATE TABLE tab (col GENERATED ALWAYS AS (%1))");
    ui->exprEdit->setDb(db);
    ui->exprEdit->setVirtualSqlExpression(sql);
}

QString ColumnGeneratedPanel::getTempTable()
{
    SchemaResolver resolver(db);
    return resolver.getUniqueName("sqlitestudio_temp_table");
}

void ColumnGeneratedPanel::updateState()
{
    ui->namedEdit->setEnabled(ui->namedCheck->isChecked());
    ui->typeCombo->setEnabled(ui->typeCheck->isChecked());
}
