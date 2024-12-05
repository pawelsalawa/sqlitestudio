#include "columndefaultpanel.h"
#include "ui_columndefaultpanel.h"
#include "parser/parser.h"
#include "parser/keywords.h"
#include "uiutils.h"
#include "schemaresolver.h"
#include <QDebug>

ColumnDefaultPanel::ColumnDefaultPanel(QWidget *parent) :
    ConstraintPanel(parent),
    ui(new Ui::ColumnDefaultPanel)
{
    ui->setupUi(this);
    init();
}

ColumnDefaultPanel::~ColumnDefaultPanel()
{
    delete ui;
}

void ColumnDefaultPanel::changeEvent(QEvent *e)
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


bool ColumnDefaultPanel::validate()
{
    if (!ui->exprEdit->isSyntaxChecked())
    {
        setValidState(ui->exprEdit, false, tr("Enter a default value expression."));
        currentMode = Mode::ERROR;
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
        // Everything looks fine, so lets do the final check - if the value is considered constant by SQLite.
        static QString tempDdlLiteralTpl = QStringLiteral("CREATE TEMP TABLE %1 (col DEFAULT %2);");
        static QString tempDdlExprTpl = QStringLiteral("CREATE TEMP TABLE %1 (col DEFAULT (%2));");
        static QString dropTempDdl = QStringLiteral("DROP TABLE %1;");

        QString tableName = getTempTable();
        QString tempDdl = tempDdlExprTpl.arg(tableName, ui->exprEdit->toPlainText());
        SqlQueryPtr res = db->exec(tempDdl);
        if (res->isError())
        {
            tempDdl = tempDdlLiteralTpl.arg(tableName, ui->exprEdit->toPlainText());
            res = db->exec(tempDdl);
            if (res->isError())
            {
                exprOk = false;
                exprError = tr("Invalid default value expression: %1. If you want to use simple string as value, remember to surround it with quote characters.")
                        .arg(res->getErrorText());
            }
            else
                currentMode = Mode::LITERAL;
        }
        else
                currentMode = Mode::EXPR;

        db->exec(dropTempDdl.arg(tableName));
    } else
        exprError = tr("Invalid default value expression. If you want to use simple string as value, remember to surround it with quote characters.");

    setValidState(ui->exprEdit, exprOk, exprError);
    setValidState(ui->namedEdit, nameOk, tr("Enter a name of the constraint."));

    lastValidationResult = (exprOk && nameOk);
    return lastValidationResult;
}

bool ColumnDefaultPanel::validateOnly()
{
    ui->exprEdit->checkSyntaxNow();
    return validate();
}

void ColumnDefaultPanel::constraintAvailable()
{
    if (constraint.isNull())
        return;

    readConstraint();
    updateVirtualSql();
    validateOnly();
}

void ColumnDefaultPanel::storeConfiguration()
{
    if (constraint.isNull())
        return;

    if (currentMode == Mode::ERROR)
    {
        qCritical() << "Call to ColumnDefaultPanel::storeConfiguration() while its mode is in ERROR state.";
        return;
    }

    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Column::Constraint::DEFAULT;

    switch (currentMode)
    {
        case Mode::EXPR:
            storeExpr(constr);
            break;
        case Mode::LITERAL:
            storeLiteral(constr);
            break;
        case Mode::ERROR:
            return;
    }

    if (ui->namedCheck->isChecked())
        constr->name = ui->namedEdit->text();
    else
        constr->name.clear();
}

void ColumnDefaultPanel::storeExpr(SqliteCreateTable::Column::Constraint* constr)
{
    QString text = ui->exprEdit->toPlainText();
    clearDefault(constr);
    if (text.toUpper() == "NULL")
    {
        // We will just use literal null, no need to create expression with null.
        constr->literalNull = true;
        return;
    }

    Parser parser;
    SqliteExpr* newExpr = parser.parseExpr(text);
    newExpr->setParent(constraint.data());
    constr->expr = newExpr;
}

void ColumnDefaultPanel::storeLiteral(SqliteCreateTable::Column::Constraint* constr)
{
    QString text = ui->exprEdit->toPlainText();

    Parser parser;
    SqliteCreateTablePtr createTable = parser.parse<SqliteCreateTable>("CREATE TABLE tab (col DEFAULT "+text+");");
    if (!createTable || createTable->columns.size() == 0 || createTable->columns.first()->constraints.size() == 0)
    {
        qCritical() << "ColumnDefaultPanel::storeLiteral(): create table not parsed! Cannot store literal. Expression was:" << text;
        return;
    }

    SqliteCreateTable::Column::Constraint* parsedConstr = createTable->columns.first()->constraints.first();
    if (parsedConstr->type != SqliteCreateTable::Column::Constraint::Type::DEFAULT)
    {
        qCritical() << "ColumnDefaultPanel::storeLiteral(): parsed constraint not a DEFAULT! Cannot store literal. Expression was:" << text;
        return;
    }

    clearDefault(constr);
    if (!parsedConstr->id.isNull())
        constr->id = parsedConstr->id;
    else if (!parsedConstr->ctime.isNull())
        constr->ctime = parsedConstr->ctime.toUpper();
    else if (parsedConstr->expr)
    {
        qWarning() << "ColumnDefaultPanel::storeLiteral(): parsed constraint turned out to be an expression. This should be handled by ColumnDefaultPanel::storeExpr."
                   << "Expression was:" << text;
        constr->expr = parsedConstr->expr;
        parsedConstr->expr = nullptr;
        constr->expr->setParent(constr);
    }
    else if (parsedConstr->literalNull)
        constr->literalNull = true;
    else
        constr->literalValue = parsedConstr->literalValue;
}

void ColumnDefaultPanel::clearDefault(SqliteCreateTable::Column::Constraint* constr)
{
    if (constr->expr)
    {
        delete constr->expr;
        constr->expr = nullptr;
    }
    constr->literalNull = false;
    constr->literalValue = QVariant();
    constr->id = QString();
    constr->ctime = QString();
}

void ColumnDefaultPanel::init()
{
    setFocusProxy(ui->exprEdit);
    ui->exprEdit->setShowLineNumbers(false);

    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SIGNAL(updateValidation()));
    connect(ui->namedEdit, SIGNAL(textChanged(QString)), this, SIGNAL(updateValidation()));
    connect(ui->exprEdit, SIGNAL(textChanged()), this, SIGNAL(updateValidation()));
    connect(ui->exprEdit, SIGNAL(errorsChecked(bool)), this, SIGNAL(updateValidation()));

    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SLOT(updateState()));

    updateState();
}

void ColumnDefaultPanel::readConstraint()
{
    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());

    if (constr->expr)
    {
        ui->exprEdit->setPlainText(constr->expr->detokenize());
        currentMode = Mode::EXPR;
    }
    else if (!constr->literalValue.isNull())
    {
        ui->exprEdit->setPlainText(constr->literalValue.toString());
        currentMode = Mode::LITERAL;
    }
    else if (!constr->id.isNull())
    {
        ui->exprEdit->setPlainText(wrapObjIfNeeded(constr->id, true));
        currentMode = Mode::LITERAL;
    }
    else if (!constr->ctime.isNull())
    {
        ui->exprEdit->setPlainText(constr->ctime);
        currentMode = Mode::LITERAL;
    }
    else if (constr->literalNull)
    {
        ui->exprEdit->setPlainText("NULL");
        currentMode = Mode::LITERAL;
    }

    if (!constr->name.isNull())
    {
        ui->namedCheck->setChecked(true);
        ui->namedEdit->setText(constr->name);
    }
}

void ColumnDefaultPanel::updateVirtualSql()
{
    static QString sql = QStringLiteral("CREATE TABLE tab (col DEFAULT %1)");
    ui->exprEdit->setDb(db);
    ui->exprEdit->setVirtualSqlExpression(sql.arg("(%1)"));
}

QString ColumnDefaultPanel::getTempTable()
{
    SchemaResolver resolver(db);
    return resolver.getUniqueName("sqlitestudio_temp_table");
}

void ColumnDefaultPanel::updateState()
{
    ui->namedEdit->setEnabled(ui->namedCheck->isChecked());
}
