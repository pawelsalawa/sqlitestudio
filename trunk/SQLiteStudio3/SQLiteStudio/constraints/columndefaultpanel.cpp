#include "columndefaultpanel.h"
#include "ui_columndefaultpanel.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/parser.h"
#include "parser/keywords.h"
#include "uiutils.h"
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
    bool nameOk = true;
    if (ui->namedCheck->isChecked() && ui->namedEdit->text().isEmpty())
        nameOk = false;

    bool exprOk = !ui->exprEdit->toPlainText().trimmed().isEmpty() &&
            !ui->exprEdit->haveErrors();

    bool exprCheckedOk = exprOk && ui->exprEdit->isSyntaxChecked();

    setValidStyle(ui->exprGroup, exprOk);
    setValidStyle(ui->namedCheck, nameOk);

    return exprCheckedOk && nameOk;
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
}

void ColumnDefaultPanel::storeConfiguration()
{
    if (constraint.isNull())
        return;

    SqliteCreateTable::Column::Constraint* constr = dynamic_cast<SqliteCreateTable::Column::Constraint*>(constraint.data());
    constr->type = SqliteCreateTable::Column::Constraint::DEFAULT;

    SqliteExprPtr expr = parseExpression(ui->exprEdit->toPlainText());
    SqliteExpr* newExpr = new SqliteExpr(*expr.data());
    newExpr->setParent(constraint.data());
    constr->expr = newExpr;

    if (ui->namedCheck->isChecked())
        constr->name = ui->namedEdit->text();
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
        ui->exprEdit->setPlainText(constr->expr->detokenize());
    else if (!constr->literalValue.isNull())
        ui->exprEdit->setPlainText(constr->literalValue.toString());

    if (!constr->name.isNull())
    {
        ui->namedCheck->setChecked(true);
        ui->namedEdit->setText(constr->name);
    }
}

void ColumnDefaultPanel::updateVirtualSql()
{
    ui->exprEdit->setDb(db);

    SqliteCreateTable::Column* column = dynamic_cast<SqliteCreateTable::Column*>(constraint->parentStatement());
    SqliteCreateTable* createTable = dynamic_cast<SqliteCreateTable*>(column->parentStatement());

    createTable->rebuildTokens();
    TokenList tokens = createTable->tokens;
    TokenList colTokens = column->tokens;
    if (createTable->columns.indexOf(column) == -1)
    {
        if (createTable->columns.size() == 0)
        {
            // No columns. Cannot get any context info.
            return;
        }

        colTokens = createTable->columns.last()->tokens;
    }

    if (colTokens.size() == 0)
    {
        qWarning() << "CREATE TABLE tokens are invalid (0) while call to ColumnDefaultPanel::updateVirtualSql().";
        return;
    }

    int idx = tokens.lastIndexOf(colTokens.last());
    if (idx == -1)
    {
        qWarning() << "CREATE TABLE tokens are invalid while call to ColumnDefaultPanel::updateVirtualSql().";
        return;
    }
    idx++;

    TokenList newTokens;
    newTokens << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::KEYWORD, "DEFAULT")
              << TokenPtr::create(Token::SPACE, " ");

    if (constraint->dialect == Dialect::Sqlite3)
    {
        newTokens << TokenPtr::create(Token::PAR_LEFT, "(")
                  << TokenPtr::create(Token::OTHER, "%1")
                  << TokenPtr::create(Token::PAR_RIGHT, ")");
    }
    else
    {
        newTokens << TokenPtr::create(Token::OTHER, "%1");
    }

    tokens.insert(idx, newTokens);
    QString sql = tokens.detokenize();

    ui->exprEdit->setVirtualSqlExpression(sql);
}

SqliteExprPtr ColumnDefaultPanel::parseExpression(const QString& sql)
{
    Parser parser(db->getDialect());
    if (!parser.parse("SELECT "+sql))
        return SqliteExprPtr();

    QList<SqliteQueryPtr> queries = parser.getQueries();
    if (queries.size() == 0)
        return SqliteExprPtr();

    SqliteQueryPtr first = queries.first();
    if (first->queryType != SqliteQueryType::Select)
        return SqliteExprPtr();

    SqliteSelectPtr select = first.dynamicCast<SqliteSelect>();
    if (select->coreSelects.size() < 1)
        return SqliteExprPtr();

    SqliteSelect::Core* core = select->coreSelects.first();
    if (core->resultColumns.size() < 1)
        return SqliteExprPtr();

    SqliteSelect::Core::ResultColumn* resCol = core->resultColumns.first();
    if (!resCol->expr)
        return SqliteExprPtr();

    return resCol->expr->detach().dynamicCast<SqliteExpr>();
}

void ColumnDefaultPanel::updateState()
{
    ui->namedEdit->setEnabled(ui->namedCheck->isChecked());
}
