#include "indexexprcolumndialog.h"
#include "ui_indexexprcolumndialog.h"
#include "parser/ast/sqliteexpr.h"
#include "db/db.h"
#include "uiutils.h"
#include "parser/parser.h"
#include "parser/ast/sqliteselect.h"
#include <QPushButton>

IndexExprColumnDialog::IndexExprColumnDialog(Db* db, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::IndexExprColumnDialog)
{
    ui->setupUi(this);

    this->db = db;
    ui->sqlEditor->setDb(db);
    ui->sqlEditor->setVirtualSqlExpression("CREATE INDEX idx ON tab (%1 COLLATE NOCASE ASC)");

    connect(ui->sqlEditor, SIGNAL(textChanged()), this, SLOT(validate()));
    connect(ui->sqlEditor, SIGNAL(errorsChecked(bool)), this, SLOT(validate()));
}

IndexExprColumnDialog::IndexExprColumnDialog(Db* db, SqliteExpr* col, QWidget *parent) :
    IndexExprColumnDialog(db, parent)
{
    readColumn(col);
}

IndexExprColumnDialog::~IndexExprColumnDialog()
{
    delete ui;
}

void IndexExprColumnDialog::readColumn(SqliteExpr* col)
{
    ui->sqlEditor->setPlainText(col->tokens.detokenize());
}

void IndexExprColumnDialog::setOkEnabled(bool enabled)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

SqliteExpr* IndexExprColumnDialog::parseExpr()
{
    Parser parser(db->getDialect());
    return parser.parseExpr(ui->sqlEditor->toPlainText());
}

bool IndexExprColumnDialog::checkRestrictions(QString& errorMsg)
{
    SqliteExprPtr expr = SqliteExprPtr(parseExpr());
    if (!expr)
        return false;

    QString key = expr->tokens.filterWhiteSpaces(false).detokenize();
    if (existingExprColumnKeys.contains(key))
    {
        errorMsg = tr("This expression is already indexed by the index.");
        return false;
    }

    if (tableColumns.contains(key))
    {
        errorMsg = tr("Column should be indexed directly, not by expression. Either extend this expression to contain something more "
                      "than just column name, or abort and select this column in index dialog directly.");
        return false;
    }

    QStringList usedColumns = expr->getContextColumns(false, true);
    for (const QString& col : usedColumns)
    {
        if (!tableColumns.contains(col))
        {
            errorMsg = tr("Column '%1' does not belong to the table covered by this index. Indexed expressions can refer only to columns from the indexed table.").arg(col);
            return false;
        }
    }

    QList<SqliteSelect*> selects = expr->getAllTypedStatements<SqliteSelect>();
    if (!selects.isEmpty())
    {
        errorMsg = tr("It's forbidden to use 'SELECT' statements in indexed expressions.");
        return false;
    }

    return true;
}

void IndexExprColumnDialog::setExistingExprColumnKeys(const QStringList& value)
{
    existingExprColumnKeys = value;
}

void IndexExprColumnDialog::setTableColumns(const QStringList& value)
{
    tableColumns = value;
}

void IndexExprColumnDialog::validate()
{
    if (!ui->sqlEditor->isSyntaxChecked())
    {
        setValidState(ui->sqlEditor, false, tr("Enter an indexed expression."));
        setOkEnabled(false);
        return;
    }

    // First check if we already validated this text.
    // This method is called twice, by both errorsChecked() and textChanged().
    QString text = ui->sqlEditor->toPlainText();
    if (!lastValidatedText.isNull() && lastValidatedText == text)
        return;

    lastValidatedText = text;

    bool exprOk = !ui->sqlEditor->toPlainText().trimmed().isEmpty() && !ui->sqlEditor->haveErrors();
    QString errorMsg = tr("Invalid expression.");
    if (exprOk)
        exprOk = checkRestrictions(errorMsg);

    setValidState(ui->sqlEditor, exprOk, errorMsg);
    setOkEnabled(exprOk);
}

SqliteExpr* IndexExprColumnDialog::getColumn() const
{
    return theColumn;
}

void IndexExprColumnDialog::accept()
{
    SqliteExpr* expr = parseExpr();
    if (expr)
    {
        expr->rebuildTokens();
        theColumn = expr;
    }
    else
        qCritical() << "Accepted IndexExprColumnDialog with unparsable expr! This should not happen. IndexDialog will get null expr.";

    QDialog::accept();
}


int IndexExprColumnDialog::exec()
{
    ui->sqlEditor->checkSyntaxNow();
    return QDialog::exec();
}
