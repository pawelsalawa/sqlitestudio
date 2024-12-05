#include "triggerdialog.h"
#include "ui_triggerdialog.h"
#include "services/notifymanager.h"
#include "parser/ast/sqliteexpr.h"
#include "triggercolumnsdialog.h"
#include "common/utils_sql.h"
#include "schemaresolver.h"
#include "iconmanager.h"
#include "db/chainexecutor.h"
#include "dbtree/dbtree.h"
#include "ddlpreviewdialog.h"
#include "uiconfig.h"
#include "services/config.h"
#include "uiutils.h"
#include "services/codeformatter.h"
#include "common/dialogsizehandler.h"
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>

QStringList TriggerDialog::tableEventNames;
QStringList TriggerDialog::viewEventNames;

TriggerDialog::TriggerDialog(Db* db, QWidget *parent) :
    QDialog(parent),
    db(db),
    ui(new Ui::TriggerDialog)
{
    init();
}

TriggerDialog::~TriggerDialog()
{
    delete ui;
}

void TriggerDialog::setParentTable(const QString& name)
{
    forTable = true;
    table = name;
    initTrigger();
}

void TriggerDialog::setParentView(const QString& name)
{
    forTable = false;
    view = name;
    initTrigger();
}

void TriggerDialog::setTrigger(const QString& name)
{
    trigger = name;
    originalTriggerName = name;
    existingTrigger = true;
    initTrigger();
}

void TriggerDialog::staticInit()
{
    tableEventNames = QStringList({
                                      SqliteCreateTrigger::time(SqliteCreateTrigger::Time::null),
                                      SqliteCreateTrigger::time(SqliteCreateTrigger::Time::AFTER),
                                      SqliteCreateTrigger::time(SqliteCreateTrigger::Time::BEFORE)
                                  });
    viewEventNames = QStringList({
                                     SqliteCreateTrigger::time(SqliteCreateTrigger::Time::INSTEAD_OF)
                                  });
}

void TriggerDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void TriggerDialog::init()
{
    ui->setupUi(this);
    limitDialogWidth(this);
    DialogSizeHandler::applyFor(this);

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateDdlTab(int)));
    connect(ui->actionColumns, SIGNAL(clicked()), this, SLOT(showColumnsDialog()));

    // On object combo
    ui->onCombo->setEnabled(false);
    connect(ui->onCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(tableChanged(QString)));

    // Action combo
    ui->actionCombo->addItems({
                                  SqliteCreateTrigger::Event::typeToString(SqliteCreateTrigger::Event::DELETE),
                                  SqliteCreateTrigger::Event::typeToString(SqliteCreateTrigger::Event::INSERT),
                                  SqliteCreateTrigger::Event::typeToString(SqliteCreateTrigger::Event::UPDATE),
                                  SqliteCreateTrigger::Event::typeToString(SqliteCreateTrigger::Event::UPDATE_OF)
                              });
    connect(ui->actionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateState()));

    // Scope combo
    ui->scopeCombo->addItems({
                                 SqliteCreateTrigger::scopeToString(SqliteCreateTrigger::Scope::null),
                                 SqliteCreateTrigger::scopeToString(SqliteCreateTrigger::Scope::FOR_EACH_ROW)
                             });

    // Event combo - default values
    ui->whenCombo->addItems(tableEventNames + viewEventNames);

    // Precondition
    connect(ui->preconditionCheck, SIGNAL(clicked()), this, SLOT(updateState()));
    connect(ui->preconditionEdit, SIGNAL(errorsChecked(bool)), this, SLOT(updateValidation()));
    connect(ui->preconditionEdit, SIGNAL(textChanged()), this, SLOT(updateValidation()));
    ui->preconditionEdit->setDb(db);

    // Code
    connect(ui->codeEdit, SIGNAL(errorsChecked(bool)), this, SLOT(updateValidation()));
    connect(ui->codeEdit, SIGNAL(textChanged()), this, SLOT(updateValidation()));
    ui->codeEdit->setDb(db);
}

void TriggerDialog::initTrigger()
{
    // Name edit
    ui->nameEdit->setText(trigger);

    if (trigger.isNull())
    {
        createTrigger = SqliteCreateTriggerPtr::create();
        createTrigger->event = new SqliteCreateTrigger::Event();
    }
    else
    {
        parseDdl();
        readTrigger();
    }

    // Event combo
    QString eventValue = ui->whenCombo->currentText();
    ui->whenCombo->clear();
    if (forTable)
    {
        ui->whenCombo->addItems(tableEventNames);
    }
    else
    {
        ui->whenCombo->addItems(viewEventNames);
        ui->whenCombo->setEnabled(false);
        ui->onLabel->setText(tr("On view:"));
    }
    ui->whenCombo->setCurrentText(eventValue);

    if (!view.isNull() || !table.isNull())
    {
        readColumns();
        QString target = getTargetObjectName();
        ui->onCombo->addItem(target);
        ui->onCombo->setCurrentText(target);
    }

    // Precondition and code edits
    setupVirtualSqls();

    updateState();
}

void TriggerDialog::parseDdl()
{
    SchemaResolver resolver(db);
    SqliteQueryPtr parsedObject = resolver.getParsedObject(trigger, SchemaResolver::TRIGGER);
    if (!parsedObject.dynamicCast<SqliteCreateTrigger>())
    {
        notifyError(tr("Could not process trigger %1 correctly. Unable to open a trigger dialog.").arg(trigger));
        reject();
        return;
    }

    createTrigger = parsedObject.dynamicCast<SqliteCreateTrigger>();
    ddl = createTrigger->detokenize();
}

void TriggerDialog::readTrigger()
{
    if (!createTrigger)
        return;

    forTable = createTrigger->eventTime != SqliteCreateTrigger::Time::INSTEAD_OF;
    if (forTable)
        table = createTrigger->table;
    else
        view = createTrigger->table;

    ui->onCombo->addItem(createTrigger->table);
    ui->onCombo->setCurrentText(createTrigger->table);
    ui->whenCombo->setCurrentText(SqliteCreateTrigger::time(createTrigger->eventTime));
    ui->actionCombo->setCurrentText(SqliteCreateTrigger::Event::typeToString(createTrigger->event->type));
    ui->scopeCombo->setCurrentText(SqliteCreateTrigger::scopeToString(createTrigger->scope));
    if (createTrigger->precondition)
    {
        ui->preconditionCheck->setChecked(true);
        ui->preconditionEdit->setPlainText(createTrigger->precondition->detokenize());
    }

    if (createTrigger->queries.size() > 0)
    {
        QStringList sqls;
        for (SqliteQuery*& query : createTrigger->queries)
            sqls << query->detokenize();

        ui->codeEdit->setPlainText(sqls.join(";\n")+";");
    }

    rebuildTrigger();
    originalDdl = ddl;
}

void TriggerDialog::setupVirtualSqls()
{
    static QString preconditionVirtSql = QStringLiteral("CREATE TRIGGER %1 BEFORE INSERT ON %2 WHEN %3 BEGIN SELECT 1; END;");
    static QString codeVirtSql = QStringLiteral("CREATE TRIGGER %1 BEFORE INSERT ON %2 BEGIN %3 END;");
    ui->codeEdit->setVirtualSqlCompleteSemicolon(true);
    if (!trigger.isNull())
    {
        if (createTrigger) // if false, then there was a parsing error in parseDdl().
        {
            ui->preconditionEdit->setVirtualSqlExpression(
                        preconditionVirtSql.arg(wrapObjIfNeeded(trigger),
                                                wrapObjIfNeeded(createTrigger->table),
                                                "%1"));

            ui->codeEdit->setVirtualSqlExpression(
                        codeVirtSql.arg(
                            wrapObjIfNeeded(trigger),
                            wrapObjIfNeeded(createTrigger->table),
                            "%1"));
        }
    }
    else if (!table.isNull() || !view.isNull())
    {
        ui->preconditionEdit->setVirtualSqlExpression(
                    preconditionVirtSql.arg("trig",
                                            wrapObjIfNeeded(getTargetObjectName()),
                                            "%1"));

        ui->codeEdit->setVirtualSqlExpression(
                    codeVirtSql.arg("trig",
                                    wrapObjIfNeeded(getTargetObjectName()),
                                    "%1"));
    }
    else
    {
        qCritical() << "TriggerDialog is in invalid state. Called initTrigger() but none of trigger/table/view values are set.";
    }
}

void TriggerDialog::readColumns()
{
    SchemaResolver resolver(db);
    if (!table.isNull())
        targetColumns = resolver.getTableColumns(table);
    else if (!view.isNull())
        targetColumns = resolver.getViewColumns(view);
    else
        targetColumns.clear();

    if (createTrigger)
        selectedColumns = createTrigger->event->columnNames;
}

QString TriggerDialog::getTargetObjectName() const
{
    if (!table.isNull())
        return table;

    return view;
}

void TriggerDialog::rebuildTrigger()
{
    /*
     * Trigger is not rebuilt into SqliteCreateTrigger, because it's impossible to parse
     * precondition or queries if they are invalid and we still need an invalid queries
     * to be presented on the DDL tab.
     */
    static const QString tempDdl = QStringLiteral("CREATE TRIGGER %1%2 %3%4 ON %5%6%7 BEGIN %8 END;");

    QString trigName = wrapObjIfNeeded(ui->nameEdit->text());
    QString when = ui->whenCombo->currentText();
    QString action = ui->actionCombo->currentText();
    QString columns = "";
    QString target = wrapObjIfNeeded(getTargetObjectName());
    QString scope = ui->scopeCombo->currentText();
    QString precondition = "";
    QString queries = ui->codeEdit->toPlainText();

    // Columns
    SqliteCreateTrigger::Event::Type actionType = SqliteCreateTrigger::Event::stringToType(ui->actionCombo->currentText());
    if (actionType == SqliteCreateTrigger::Event::UPDATE_OF)
    {
        QStringList colNames;
        for (QString& colName : selectedColumns)
            colNames << wrapObjIfNeeded(colName);

        columns = " "+colNames.join(", ");
    }

    // Precondition
    if (ui->preconditionCheck->isChecked())
        precondition = " WHEN "+ui->preconditionEdit->toPlainText();

    // Queries
    if (!queries.trimmed().endsWith(";"))
        queries += ";";

    // When
    if (!when.isNull())
        when.prepend(" ");

    // Scope
    if (!scope.isNull())
        scope.prepend(" ");

    ddl = tempDdl.arg(trigName, when, action, columns, target, scope, precondition, queries);
}

void TriggerDialog::updateState()
{
    SqliteCreateTrigger::Event::Type type = SqliteCreateTrigger::Event::stringToType(ui->actionCombo->currentText());
    ui->actionColumns->setEnabled(type == SqliteCreateTrigger::Event::UPDATE_OF);
    ui->preconditionEdit->setEnabled(ui->preconditionCheck->isChecked());
    updateValidation();
}

void TriggerDialog::updateValidation()
{
    SqliteCreateTrigger::Event::Type type = SqliteCreateTrigger::Event::stringToType(ui->actionCombo->currentText());
    bool columnsOk = (type != SqliteCreateTrigger::Event::UPDATE_OF || selectedColumns.size() > 0);

    bool preconditionOk = (!ui->preconditionCheck->isChecked() ||
                           (ui->preconditionEdit->isSyntaxChecked() && !ui->preconditionEdit->haveErrors()));

    bool codeOk = (ui->codeEdit->isSyntaxChecked() && !ui->codeEdit->haveErrors());

    setValidState(ui->preconditionCheck, preconditionOk, tr("Enter a valid condition."));
    setValidState(ui->codeEdit, codeOk, tr("Enter a valid trigger code."));
    ui->actionColumns->setIcon(columnsOk ? ICONS.TRIGGER_COLUMNS : ICONS.TRIGGER_COLUMNS_INVALID);

    QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(columnsOk && preconditionOk && codeOk);
}

void TriggerDialog::showColumnsDialog()
{
    QPoint topRight = ui->actionColumns->mapToGlobal(ui->actionColumns->rect().topRight());

    TriggerColumnsDialog dialog(this, topRight.x(), topRight.y());
    for (const QString& colName : targetColumns)
        dialog.addColumn(colName, selectedColumns.contains(colName, Qt::CaseInsensitive));

    if (dialog.exec() != QDialog::Accepted)
        return;

    QStringList newColumns = dialog.getCheckedColumns();
    selectedColumns = newColumns;
    updateValidation();
}

void TriggerDialog::updateDdlTab(int tabIdx)
{
    if (tabIdx != 1)
        return;

    rebuildTrigger();
    QString formatted = FORMATTER->format("sql", ddl, db);
    ui->ddlEdit->setPlainText(formatted);
}

void TriggerDialog::tableChanged(const QString& newValue)
{
    ui->preconditionEdit->setTriggerContext(newValue);
    ui->codeEdit->setTriggerContext(newValue);
}

void TriggerDialog::accept()
{
    rebuildTrigger();
    if (originalDdl == ddl)
    {
        // Nothing changed. Just close.
        QDialog::accept();
        return;
    }

    QStringList sqls;
    if (existingTrigger)
        sqls << QString("DROP TRIGGER %1").arg(wrapObjIfNeeded(originalTriggerName));

    sqls << ddl;

    if (!CFG_UI.General.DontShowDdlPreview.get())
    {
        DdlPreviewDialog dialog(db, this);
        dialog.setDdl(sqls);
        if (dialog.exec() != QDialog::Accepted)
            return;
    }

    ChainExecutor executor;
    executor.setDb(db);
    executor.setAsync(false);
    executor.setQueries(sqls);
    executor.exec();

    if (executor.getSuccessfulExecution())
    {
        CFG->addDdlHistory(sqls.join("\n"), db->getName(), db->getPath());

        QDialog::accept();
        DBTREE->refreshSchema(db);
        return;
    }

    QMessageBox::critical(this, tr("Error", "trigger dialog"), tr("An error occurred while executing SQL statements:\n%1")
                          .arg(executor.getErrorsMessages().join(",\n")), QMessageBox::Ok);
}
