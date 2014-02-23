#include "constraintcheckpanel.h"
#include "ui_constraintcheckpanel.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/parser.h"
#include "parser/keywords.h"
#include "uiutils.h"
#include <QDebug>

ConstraintCheckPanel::ConstraintCheckPanel(QWidget *parent) :
    ConstraintPanel(parent),
    ui(new Ui::ConstraintCheckPanel)
{
    ui->setupUi(this);
    init();
}

ConstraintCheckPanel::~ConstraintCheckPanel()
{
    delete ui;
}

void ConstraintCheckPanel::changeEvent(QEvent *e)
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


bool ConstraintCheckPanel::validate()
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

void ConstraintCheckPanel::constraintAvailable()
{
    if (constraint.isNull())
        return;

    if (constraint->dialect == Dialect::Sqlite3)
    {
        ui->onConflictCheck->setVisible(false);
        ui->onConflictCombo->setVisible(false);
    }

    readConstraint();
    updateVirtualSql();
}

void ConstraintCheckPanel::storeConfiguration()
{
    if (constraint.isNull())
        return;

    storeType();

    SqliteExprPtr expr = parseExpression(ui->exprEdit->toPlainText());
    SqliteExpr* newExpr = new SqliteExpr(*expr.data());
    newExpr->setParent(constraint.data());
    storeExpr(newExpr);

    QString name = QString::null;
    if (ui->namedCheck->isChecked())
        name = ui->namedEdit->text();

    storeName(name);

    if (constraint->dialect == Dialect::Sqlite2 && ui->onConflictCheck->isChecked())
        storeConflictAlgo(sqliteConflictAlgo(ui->onConflictCombo->currentText()));
}

void ConstraintCheckPanel::init()
{
    setFocusProxy(ui->exprEdit);
    ui->exprEdit->setShowLineNumbers(false);

    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SIGNAL(updateValidation()));
    connect(ui->namedEdit, SIGNAL(textChanged(QString)), this, SIGNAL(updateValidation()));
    connect(ui->exprEdit, SIGNAL(textChanged()), this, SIGNAL(updateValidation()));
    connect(ui->exprEdit, SIGNAL(errorsChecked(bool)), this, SIGNAL(updateValidation()));

    connect(ui->namedCheck, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(ui->onConflictCheck, SIGNAL(toggled(bool)), this, SLOT(updateState()));

    ui->onConflictCombo->addItems(getConflictAlgorithms());

    updateState();
}

void ConstraintCheckPanel::readConstraint()
{
    SqliteExpr* expr = readExpr();
    if (expr)
        ui->exprEdit->setPlainText(expr->detokenize());

    QString name = readName();
    if (!name.isNull())
    {
        ui->namedCheck->setChecked(true);
        ui->namedEdit->setText(name);
    }

    SqliteConflictAlgo onConflict = readConflictAlgo();
    if (constraint->dialect == Dialect::Sqlite2 && onConflict != SqliteConflictAlgo::null)
    {
        ui->onConflictCheck->setChecked(true);
        ui->onConflictCombo->setCurrentText(sqliteConflictAlgo(onConflict));
    }
}

void ConstraintCheckPanel::updateVirtualSql()
{
    ui->exprEdit->setDb(db);

    SqliteCreateTable* createTable = getCreateTable();

    createTable->rebuildTokens();
    TokenList tokens = createTable->tokens;
    int idx = tokens.lastIndexOf(Token::PAR_RIGHT);
    if (idx == -1)
    {
        qWarning() << "CREATE TABLE tokens are invalid while call to ConstraintCheckPanel::updateVirtualSql().";
        return;
    }

    TokenList newTokens;
    newTokens << TokenPtr::create(Token::OPERATOR, ",")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::KEYWORD, "CHECK")
              << TokenPtr::create(Token::SPACE, " ")
              << TokenPtr::create(Token::PAR_LEFT, "(")
              << TokenPtr::create(Token::OTHER, "%1")
              << TokenPtr::create(Token::PAR_RIGHT, ")");

    tokens.insert(idx, newTokens);
    QString sql = tokens.detokenize();

    ui->exprEdit->setVirtualSqlExpression(sql);
}

SqliteExprPtr ConstraintCheckPanel::parseExpression(const QString& sql)
{
    // TODO modify to use Parser::parseExpr()
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

void ConstraintCheckPanel::updateState()
{
    ui->namedEdit->setEnabled(ui->namedCheck->isChecked());
    ui->onConflictCombo->setEnabled(ui->onConflictCheck->isChecked());
}

bool ConstraintCheckPanel::validateOnly()
{
    ui->exprEdit->checkSyntaxNow();
    return validate();
}
