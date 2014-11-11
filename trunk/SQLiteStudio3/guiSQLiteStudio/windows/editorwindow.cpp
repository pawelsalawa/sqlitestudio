#include "editorwindow.h"
#include "ui_editorwindow.h"
#include "uiutils.h"
#include "datagrid/sqlquerymodel.h"
#include "iconmanager.h"
#include "dblistmodel.h"
#include "services/notifymanager.h"
#include "dbtree/dbtree.h"
#include "datagrid/sqlqueryitem.h"
#include "datagrid/sqlqueryview.h"
#include "mainwindow.h"
#include "mdiarea.h"
#include "common/unused.h"
#include "common/extaction.h"
#include "uiconfig.h"
#include "services/config.h"
#include "parser/lexer.h"
#include "common/utils_sql.h"
#include "parser/parser.h"
#include <QComboBox>
#include <QDebug>
#include <QStringListModel>
#include <QActionGroup>
#include <QMessageBox>
#include <dbobjectdialogs.h>
#include <dialogs/exportdialog.h>

CFG_KEYS_DEFINE(EditorWindow)
EditorWindow::ResultsDisplayMode EditorWindow::resultsDisplayMode;
QHash<EditorWindow::Action,QAction*> EditorWindow::staticActions;
QHash<EditorWindow::ActionGroup,QActionGroup*> EditorWindow::staticActionGroups;

EditorWindow::EditorWindow(QWidget *parent) :
    MdiChild(parent),
    ui(new Ui::EditorWindow)
{
    ui->setupUi(this);
    init();
}

EditorWindow::EditorWindow(const EditorWindow& editor) :
    MdiChild(editor.parentWidget()),
    ui(new Ui::EditorWindow)
{
    ui->setupUi(this);
    init();
    ui->sqlEdit->setAutoCompletion(false);
    ui->sqlEdit->setPlainText(editor.ui->sqlEdit->toPlainText());
    ui->sqlEdit->setAutoCompletion(true);
}

EditorWindow::~EditorWindow()
{
    delete ui;
}

void EditorWindow::staticInit()
{
    qRegisterMetaType<EditorWindow>("EditorWindow");
    resultsDisplayMode = ResultsDisplayMode::BELOW_QUERY;
    loadTabsMode();
    createStaticActions();
}

void EditorWindow::insertAction(ExtActionPrototype* action, EditorWindow::ToolBar toolbar)
{
    return ExtActionContainer::insertAction<EditorWindow>(action, toolbar);
}

void EditorWindow::insertActionBefore(ExtActionPrototype* action, EditorWindow::Action beforeAction, EditorWindow::ToolBar toolbar)
{
    return ExtActionContainer::insertActionBefore<EditorWindow>(action, beforeAction, toolbar);
}

void EditorWindow::insertActionAfter(ExtActionPrototype* action, EditorWindow::Action afterAction, EditorWindow::ToolBar toolbar)
{
    return ExtActionContainer::insertActionAfter<EditorWindow>(action, afterAction, toolbar);
}

void EditorWindow::removeAction(ExtActionPrototype* action, EditorWindow::ToolBar toolbar)
{
    ExtActionContainer::removeAction<EditorWindow>(action, toolbar);
}

void EditorWindow::init()
{
    setFocusProxy(ui->sqlEdit);
    updateResultsDisplayMode();

    resultsModel = new SqlQueryModel(this);
    ui->dataView->init(resultsModel);

    createDbCombo();
    initActions();
    updateShortcutTips();

    Db* currentDb = getCurrentDb();
    resultsModel->setDb(currentDb);
    ui->sqlEdit->setDb(currentDb);

    connect(resultsModel, SIGNAL(executionSuccessful()), this, SLOT(executionSuccessful()));
    connect(resultsModel, SIGNAL(executionFailed(QString)), this, SLOT(executionFailed(QString)));
    connect(resultsModel, SIGNAL(totalRowsAndPagesAvailable()), this, SLOT(totalRowsAndPagesAvailable()));

    // SQL history list
    ui->historyList->setModel(CFG->getSqlHistoryModel());
    ui->historyList->resizeColumnToContents(1);
    connect(ui->historyList->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(historyEntrySelected(QModelIndex,QModelIndex)));
    connect(ui->historyList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(historyEntryActivated(QModelIndex)));

    updateState();
}

void EditorWindow::loadTabsMode()
{
    QString tabsString = CFG_UI.General.SqlEditorTabs.get();
    if (tabsString == "SEPARATE_TAB")
        resultsDisplayMode = ResultsDisplayMode::SEPARATE_TAB;
    else if (tabsString == "BELOW_QUERY")
        resultsDisplayMode = ResultsDisplayMode::BELOW_QUERY;
}

void EditorWindow::createStaticActions()
{
    staticActions[RESULTS_IN_TAB] = new ExtAction(ICONS.RESULTS_IN_TAB, tr("Results in the separate tab"), MainWindow::getInstance());
    staticActions[RESULTS_BELOW] = new ExtAction(ICONS.RESULTS_BELOW, tr("Results below the query"), MainWindow::getInstance());

    staticActionGroups[ActionGroup::RESULTS_POSITIONING] = new QActionGroup(MainWindow::getInstance());
    staticActionGroups[ActionGroup::RESULTS_POSITIONING]->addAction(staticActions[RESULTS_IN_TAB]);
    staticActionGroups[ActionGroup::RESULTS_POSITIONING]->addAction(staticActions[RESULTS_BELOW]);

    connect(staticActions[RESULTS_BELOW], &QAction::triggered, [=]()
    {
        resultsDisplayMode = ResultsDisplayMode::BELOW_QUERY;
        CFG_UI.General.SqlEditorTabs.set("BELOW_QUERY");
    });
    connect(staticActions[RESULTS_IN_TAB], &QAction::triggered, [=]()
    {
        resultsDisplayMode = ResultsDisplayMode::SEPARATE_TAB;
        CFG_UI.General.SqlEditorTabs.set("SEPARATE_TAB");
    });

    staticActions[RESULTS_BELOW]->setCheckable(true);
    staticActions[RESULTS_IN_TAB]->setCheckable(true);
    if (resultsDisplayMode == ResultsDisplayMode::BELOW_QUERY)
        staticActions[RESULTS_BELOW]->setChecked(true);
    else
        staticActions[RESULTS_IN_TAB]->setChecked(true);
}

Icon* EditorWindow::getIconNameForMdiWindow()
{
    return ICONS.OPEN_SQL_EDITOR;
}

QString EditorWindow::getTitleForMdiWindow()
{
    QStringList existingNames = MainWindow::getInstance()->getMdiArea()->getWindowTitles();
    QString title = tr("SQL editor %1").arg(sqlEditorNum++);
    while (existingNames.contains(title))
        title = tr("SQL editor %1").arg(sqlEditorNum++);

    return title;
}

QSize EditorWindow::sizeHint() const
{
    return QSize(500, 400);
}

QAction* EditorWindow::getAction(EditorWindow::Action action)
{
    switch (action)
    {
        case RESULTS_BELOW:
        case RESULTS_IN_TAB:
        {
            if (!staticActions.contains(action))
                return nullptr;

            return staticActions.value(action);
        }
        default:
            break;
    }

    return ExtActionContainer::getAction(action);
}

QString EditorWindow::getQueryToExecute(bool doSelectCurrentQuery)
{
    QString sql;
    if (ui->sqlEdit->textCursor().hasSelection())
    {
        sql = ui->sqlEdit->textCursor().selectedText();
        fixTextCursorSelectedText(sql);
    }
    else if (CFG_UI.General.ExecuteCurrentQueryOnly.get())
    {
        ui->sqlEdit->saveSelection();
        selectCurrentQuery(true);
        sql = ui->sqlEdit->textCursor().selectedText();
        fixTextCursorSelectedText(sql);
        if (!doSelectCurrentQuery)
            ui->sqlEdit->restoreSelection();
    }
    else
    {
        sql = ui->sqlEdit->toPlainText();
    }
    return sql;
}

bool EditorWindow::setCurrentDb(Db *db)
{
    if (dbCombo->findText(db->getName()) == -1)
        return false;

    dbCombo->setCurrentText(db->getName());
    return true;
}

void EditorWindow::setContents(const QString &sql)
{
    ui->sqlEdit->setPlainText(sql);
}

QString EditorWindow::getContents() const
{
    return ui->sqlEdit->toPlainText();
}

void EditorWindow::execute()
{
    execQuery();
}

QToolBar* EditorWindow::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return ui->toolBar;
}

SqlEditor* EditorWindow::getEditor() const
{
    return ui->sqlEdit;
}

QVariant EditorWindow::saveSession()
{
    QHash<QString,QVariant> sessionValue;
    sessionValue["query"] = ui->sqlEdit->toPlainText();
    sessionValue["curPos"] = ui->sqlEdit->textCursor().position();

    Db* db = getCurrentDb();
    if (db)
        sessionValue["db"] = db->getName();

    return sessionValue;
}

bool EditorWindow::restoreSession(const QVariant& sessionValue)
{
    QHash<QString, QVariant> value = sessionValue.toHash();
    if (value.size() == 0)
        return true;

    if (value.contains("query"))
    {
        ui->sqlEdit->setAutoCompletion(false);
        ui->sqlEdit->setPlainText(value["query"].toString());
        ui->sqlEdit->setAutoCompletion(true);
    }

    if (value.contains("curPos"))
    {
        QTextCursor cursor = ui->sqlEdit->textCursor();
        cursor.setPosition(value["curPos"].toInt());
        ui->sqlEdit->setTextCursor(cursor);
    }

    if (value.contains("db"))
    {
        dbCombo->setCurrentText(value["db"].toString());
        if (dbCombo->currentText().isEmpty() && dbCombo->count() > 0)
            dbCombo->setCurrentIndex(0);
    }
    return true;
}

void EditorWindow::changeEvent(QEvent *e)
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

Db* EditorWindow::getCurrentDb()
{
    return dbComboModel->getDb(dbCombo->currentIndex());
}

void EditorWindow::updateResultsDisplayMode()
{
    switch (resultsDisplayMode)
    {
        case EditorWindow::ResultsDisplayMode::SEPARATE_TAB:
        {
            // Remove old view
            ui->resultsContainer->hide();
            ui->resultsContainer->layout()->removeWidget(ui->resultsFrame);

            // Add new view
            ui->tabWidget->insertTab(1, ui->results, tr("Results"));
            ui->resultsFrame->setParent(ui->results);
            ui->results->layout()->addWidget(ui->resultsFrame);
            break;
        }
        case EditorWindow::ResultsDisplayMode::BELOW_QUERY:
        {
            int currIdx = ui->tabWidget->currentIndex();

            // Remove old view
            ui->tabWidget->removeTab(1);
            ui->results->layout()->removeWidget(ui->resultsFrame);

            // Add new view
            ui->resultsContainer->show();
            ui->resultsFrame->setParent(ui->resultsContainer);
            ui->resultsContainer->layout()->addWidget(ui->resultsFrame);

            // If results tab was selected before, switch to first tab
            if (currIdx == 1)
            {
                ui->tabWidget->setCurrentIndex(0);
                ui->dataView->setCurrentIndex(0);
                ui->dataView->getGridView()->setFocus();
            }
            break;
        }
    }
}

void EditorWindow::createActions()
{
    // SQL editor toolbar
    createAction(EXEC_QUERY, ICONS.EXEC_QUERY, tr("Execute query"), this, SLOT(execQuery()), ui->toolBar, ui->sqlEdit);
    createAction(EXPLAIN_QUERY, ICONS.EXPLAIN_QUERY, tr("Explain query"), this, SLOT(explainQuery()), ui->toolBar, ui->sqlEdit);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->sqlEdit->getAction(SqlEditor::FORMAT_SQL));
    createAction(CLEAR_HISTORY, ICONS.CLEAR_HISTORY, tr("Clear execution history", "sql editor"), this, SLOT(clearHistory()), ui->toolBar);
    ui->toolBar->addSeparator();
    createAction(EXPORT_RESULTS, ICONS.TABLE_EXPORT, tr("Export results", "sql editor"), this, SLOT(exportResults()), ui->toolBar);
    ui->toolBar->addSeparator();
    createAction(CREATE_VIEW_FROM_QUERY, ICONS.VIEW_ADD, tr("Create view from query", "sql editor"), this, SLOT(createViewFromQuery()), ui->toolBar);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->sqlEdit->getAction(SqlEditor::SAVE_SQL_FILE));
    ui->toolBar->addAction(ui->sqlEdit->getAction(SqlEditor::OPEN_SQL_FILE));
    ui->toolBar->addSeparator();
    actionMap[CURRENT_DB] = ui->toolBar->addWidget(dbCombo);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(staticActions[RESULTS_IN_TAB]);
    ui->toolBar->addAction(staticActions[RESULTS_BELOW]);
    createAction(PREV_DB, tr("Previous database"), this, SLOT(prevDb()), this);
    createAction(NEXT_DB, tr("Next database"), this, SLOT(nextDb()), this);

    // Other actions
    createAction(SHOW_NEXT_TAB, tr("Show next tab", "sql editor"), this, SLOT(showNextTab()), this);
    createAction(SHOW_PREV_TAB, tr("Show previous tab", "sql editor"), this, SLOT(showPrevTab()), this);
    createAction(FOCUS_RESULTS_BELOW, tr("Focus results below", "sql editor"), this, SLOT(focusResultsBelow()), this);
    createAction(FOCUS_EDITOR_ABOVE, tr("Focus SQL editor above", "sql editor"), this, SLOT(focusEditorAbove()), this);

    // Static action triggers
    connect(staticActions[RESULTS_IN_TAB], SIGNAL(triggered()), this, SLOT(updateResultsDisplayMode()));
    connect(staticActions[RESULTS_BELOW], SIGNAL(triggered()), this, SLOT(updateResultsDisplayMode()));
}

void EditorWindow::createDbCombo()
{
    dbCombo = new QComboBox(this);
    dbComboModel = new DbListModel(this);
    dbComboModel->setCombo(dbCombo);
    dbCombo->setModel(dbComboModel);
    dbCombo->setEditable(false);
    dbCombo->setFixedWidth(100);
    connect(dbCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(dbChanged()));
}

void EditorWindow::setupDefShortcuts()
{
    // Widget context
    setShortcutContext({EXEC_QUERY, EXEC_QUERY, SHOW_NEXT_TAB, SHOW_PREV_TAB, FOCUS_RESULTS_BELOW,
                        FOCUS_EDITOR_ABOVE}, Qt::WidgetWithChildrenShortcut);

    BIND_SHORTCUTS(EditorWindow, Action);
}

void EditorWindow::selectCurrentQuery(bool fallBackToPreviousIfNecessary)
{
    Dialect dialect = Dialect::Sqlite3;
    Db* db = getCurrentDb();
    if (db && db->isValid())
        dialect = db->getDialect();

    QTextCursor cursor = ui->sqlEdit->textCursor();
    int pos = cursor.position();
    int queryStartPos;
    QString contents = ui->sqlEdit->toPlainText();
    QString query = getQueryWithPosition(contents, pos, dialect, &queryStartPos);
    TokenList tokens = Lexer::tokenize(query, dialect);
    tokens.trim();
    tokens.trimRight(Token::OPERATOR, ";");

    if (tokens.size() == 0 && fallBackToPreviousIfNecessary)
    {
        // Fallback
        pos = contents.lastIndexOf(";", pos - 1);
        if (pos > -1)
        {
            query = getQueryWithPosition(contents, pos, dialect, &queryStartPos);
            tokens = Lexer::tokenize(query, dialect);
            tokens.trim();
            tokens.trimRight(Token::OPERATOR, ";");
        }
    }

    if (tokens.size() == 0)
    {
        qWarning() << "No tokens to select in EditorWindow::selectCurrentQuery().";
        return;
    }

    cursor.clearSelection();
    cursor.setPosition(tokens.first()->start + queryStartPos);
    cursor.setPosition(tokens.last()->end + 1 + queryStartPos, QTextCursor::KeepAnchor);
    ui->sqlEdit->setTextCursor(cursor);
}

void EditorWindow::updateShortcutTips()
{
    if (actionMap.contains(PREV_DB) && actionMap.contains(NEXT_DB))
    {
        QString prevDbKey = actionMap[PREV_DB]->shortcut().toString(QKeySequence::NativeText);
        QString nextDbKey = actionMap[NEXT_DB]->shortcut().toString(QKeySequence::NativeText);
        dbCombo->setToolTip(tr("Active database (%1/%2)").arg(prevDbKey).arg(nextDbKey));
    }
}

void EditorWindow::execQuery(bool explain)
{
    QString sql = getQueryToExecute(true);
    resultsModel->setDb(getCurrentDb());
    resultsModel->setExplainMode(explain);
    resultsModel->setQuery(sql);
    ui->dataView->refreshData();
    updateState();

    if (resultsDisplayMode == ResultsDisplayMode::SEPARATE_TAB)
    {
        ui->tabWidget->setCurrentIndex(1);
        ui->dataView->setCurrentIndex(0);
        ui->dataView->getGridView()->setFocus();
    }
}

void EditorWindow::explainQuery()
{
    execQuery(true);
}

void EditorWindow::dbChanged()
{
    Db* currentDb = getCurrentDb();
    ui->sqlEdit->setDb(currentDb);
}

void EditorWindow::executionSuccessful()
{
    double secs = ((double)resultsModel->getExecutionTime()) / 1000;
    QString time = QString::number(secs, 'f', 3);
    notifyInfo(tr("Query finished in %2 second(s).").arg(time));

    lastQueryHistoryId = CFG->addSqlHistory(resultsModel->getQuery(), resultsModel->getDb()->getName(), resultsModel->getExecutionTime(), 0);

    // If we added first history entry - resize dates column.
    if (ui->historyList->model()->rowCount() == 1)
        ui->historyList->resizeColumnToContents(1);

    Db* currentDb = getCurrentDb();
    if (currentDb && resultsModel->wasSchemaModified())
        DBTREE->refreshSchema(currentDb);

    lastSuccessfulQuery = resultsModel->getQuery();

    updateState();
}

void EditorWindow::executionFailed(const QString &errorText)
{
    notifyError(errorText);
    updateState();
}

void EditorWindow::totalRowsAndPagesAvailable()
{
    qint64 rowsReturned = resultsModel->getTotalRowsReturned();
    qint64 rowsAffected = resultsModel->getTotalRowsAffected();
    qint64 rows;
    if (rowsReturned > 0)
        rows = rowsReturned;
    else
        rows = rowsAffected;

    CFG->updateSqlHistory(lastQueryHistoryId, resultsModel->getQuery(), resultsModel->getDb()->getName(), resultsModel->getExecutionTime(), rows);
}

void EditorWindow::prevDb()
{
    int idx = dbCombo->currentIndex() - 1;
    if (idx < 0)
        return;

    dbCombo->setCurrentIndex(idx);
}

void EditorWindow::nextDb()
{
    int idx = dbCombo->currentIndex() + 1;
    if (idx >= dbCombo->count())
        return;

    dbCombo->setCurrentIndex(idx);
}

void EditorWindow::showNextTab()
{
    int tabIdx = ui->tabWidget->currentIndex();
    tabIdx++;
    ui->tabWidget->setCurrentIndex(tabIdx);
}

void EditorWindow::showPrevTab()
{
    int tabIdx = ui->tabWidget->currentIndex();
    tabIdx--;
    ui->tabWidget->setCurrentIndex(tabIdx);
}

void EditorWindow::focusResultsBelow()
{
    if (resultsDisplayMode != ResultsDisplayMode::BELOW_QUERY)
        return;

    ui->dataView->setCurrentIndex(0);
    ui->dataView->getGridView()->setFocus();
}

void EditorWindow::focusEditorAbove()
{
    if (resultsDisplayMode != ResultsDisplayMode::BELOW_QUERY)
        return;

    ui->sqlEdit->setFocus();
}

void EditorWindow::historyEntrySelected(const QModelIndex& current, const QModelIndex& previous)
{
    UNUSED(previous);
    QString sql = ui->historyList->model()->index(current.row(), 4).data().toString();
    ui->historyContents->setPlainText(sql);
}

void EditorWindow::historyEntryActivated(const QModelIndex& current)
{
    QString sql = ui->historyList->model()->index(current.row(), 4).data().toString();
    ui->sqlEdit->setPlainText(sql);
    ui->tabWidget->setCurrentIndex(0);
}

void EditorWindow::clearHistory()
{
    QMessageBox::StandardButton res = QMessageBox::question(this, tr("Clear execution history"), tr("Are you sure you want to erase the entire SQL execution history? "
                                                                                                    "This cannot be undone."));
    if (res != QMessageBox::Yes)
        return;

    CFG->clearSqlHistory();
}

void EditorWindow::exportResults()
{
    if (!ExportManager::isAnyPluginAvailable())
    {
        notifyError(tr("Cannot export, because no export plugin is loaded."));
        return;
    }

    QString query = lastSuccessfulQuery.isEmpty() ?  getQueryToExecute() : lastSuccessfulQuery;
    QStringList queries = splitQueries(query, getCurrentDb()->getDialect(), false);
    if (queries.size() == 0)
    {
        qWarning() << "No queries after split in EditorWindow::exportResults()";
        return;
    }

    ExportDialog dialog(this);
    dialog.setQueryMode(getCurrentDb(), queries.last().trimmed());
    dialog.exec();
}

void EditorWindow::createViewFromQuery()
{
    if (!getCurrentDb())
    {
        notifyError(tr("No database selected in the SQL editor. Cannot create a view for unknown database."));
        return;
    }

    QString sql = getQueryToExecute(true);
    DbObjectDialogs dialogs(getCurrentDb());
    dialogs.addView(sql);
}

void EditorWindow::updateState()
{
    bool executionInProgress = resultsModel->isExecutionInProgress();
    actionMap[CURRENT_DB]->setEnabled(!executionInProgress);
    actionMap[EXEC_QUERY]->setEnabled(!executionInProgress);
    actionMap[EXPLAIN_QUERY]->setEnabled(!executionInProgress);
}

int qHash(EditorWindow::ActionGroup actionGroup)
{
    return static_cast<int>(actionGroup);
}


bool EditorWindow::isUncommited() const
{
    return ui->dataView->isUncommited();
}

QString EditorWindow::getQuitUncommitedConfirmMessage() const
{
    return tr("Editor window \"%1\" has uncommited data.").arg(getMdiWindow()->windowTitle());
}
