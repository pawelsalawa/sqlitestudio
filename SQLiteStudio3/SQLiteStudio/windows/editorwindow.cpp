#include "editorwindow.h"
#include "ui_editorwindow.h"
#include "datagrid/sqlquerymodel.h"
#include "iconmanager.h"
#include "dblistmodel.h"
#include "notifymanager.h"
#include "dbtree/dbtree.h"
#include "datagrid/sqlqueryitem.h"
#include "datagrid/sqlqueryview.h"
#include "mainwindow.h"
#include "mdiarea.h"
#include "unused.h"
#include "common/extaction.h"
#include "uiconfig.h"
#include "config.h"
#include "parser/lexer.h"
#include "utils_sql.h"
#include <QComboBox>
#include <QDebug>
#include <QStringListModel>
#include <QActionGroup>
#include <QMessageBox>

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

    connect(resultsModel, &SqlQueryModel::executionSuccessful, this, &EditorWindow::executionSuccessful);
    connect(resultsModel, &SqlQueryModel::executionFailed, this, &EditorWindow::executionFailed);
    connect(resultsModel, &SqlQueryModel::totalRowsAndPagesAvailable, this, &EditorWindow::totalRowsAndPagesAvailable);

    // SQL history list
    ui->historyList->setModel(CFG->getSqlHistoryModel());
    ui->historyList->resizeColumnToContents(1);
    connect(ui->historyList->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(historyEntrySelected(QModelIndex,QModelIndex)));
    connect(ui->historyList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(historyEntryActivated(QModelIndex)));
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
    staticActions[RESULTS_IN_TAB] = new ExtAction(ICON("results_in_tab"), tr("Results in the separate tab"), MainWindow::getInstance());
    staticActions[RESULTS_BELOW] = new ExtAction(ICON("results_below"), tr("Results below the query"), MainWindow::getInstance());

    staticActionGroups[ActionGroup::RESULTS_POSITIONING] = new QActionGroup(MainWindow::getInstance());
    staticActionGroups[ActionGroup::RESULTS_POSITIONING]->addAction(staticActions[RESULTS_IN_TAB]);
    staticActionGroups[ActionGroup::RESULTS_POSITIONING]->addAction(staticActions[RESULTS_BELOW]);

    connect(staticActions[RESULTS_BELOW], &QAction::triggered, [=]()
    {
        resultsDisplayMode = ResultsDisplayMode::BELOW_QUERY;
    });
    connect(staticActions[RESULTS_IN_TAB], &QAction::triggered, [=]()
    {
        resultsDisplayMode = ResultsDisplayMode::SEPARATE_TAB;
    });

    staticActions[RESULTS_BELOW]->setCheckable(true);
    staticActions[RESULTS_IN_TAB]->setCheckable(true);
    if (resultsDisplayMode == ResultsDisplayMode::BELOW_QUERY)
        staticActions[RESULTS_BELOW]->setChecked(true);
    else
        staticActions[RESULTS_IN_TAB]->setChecked(true);
}

QString EditorWindow::getIconNameForMdiWindow()
{
    return "open_sql_editor";
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
    QString cfgValue;
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

            cfgValue = "SEPARATE_TAB";
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

            cfgValue = "BELOW_QUERY";
            break;
        }
    }
    CFG_UI.General.SqlEditorTabs.set(cfgValue);
}

void EditorWindow::createActions()
{
    // SQL editor toolbar
    createAction(EXEC_QUERY, "exec_query", tr("Execute query"), this, SLOT(execQuery()), ui->toolBar, ui->sqlEdit);
    createAction(EXPLAIN_QUERY, "explain_query", tr("Explain query"), this, SLOT(explainQuery()), ui->toolBar, ui->sqlEdit);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->sqlEdit->getAction(SqlEditor::FORMAT_SQL));
    createAction(CLEAR_HISTORY, "clear_history", tr("Clear execution history", "sql editor"), this, SLOT(clearHistory()), ui->toolBar);
    ui->toolBar->addSeparator();
    createAction(EXPORT_RESULTS, "table_export", tr("Export results", "sql editor"), this, SLOT(exportResults()), ui->toolBar);
    createAction(CREATE_VIEW_FROM_QUERY, "view_add", tr("Create view from query", "sql editor"), this, SLOT(createViewFromQuery()), ui->toolBar);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->sqlEdit->getAction(SqlEditor::SAVE_SQL_FILE));
    ui->toolBar->addAction(ui->sqlEdit->getAction(SqlEditor::OPEN_SQL_FILE));
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(dbCombo);
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

    defShortcut(EXEC_QUERY, Qt::Key_F9);
    defShortcut(EXPLAIN_QUERY, Qt::Key_F8);
    defShortcut(PREV_DB, Qt::CTRL + Qt::Key_Up);
    defShortcut(NEXT_DB, Qt::CTRL + Qt::Key_Down);
    defShortcut(SHOW_NEXT_TAB, Qt::ALT + Qt::Key_Right);
    defShortcut(SHOW_PREV_TAB, Qt::ALT + Qt::Key_Left);
    defShortcut(FOCUS_RESULTS_BELOW, Qt::ALT + Qt::Key_PageDown);
    defShortcut(FOCUS_EDITOR_ABOVE, Qt::ALT + Qt::Key_PageUp);
}

void EditorWindow::selectCurrentQuery()
{
    Dialect dialect = Dialect::Sqlite3;
    Db* db = getCurrentDb();
    if (db)
        dialect = db->getDialect();

    QTextCursor cursor = ui->sqlEdit->textCursor();
    int pos = cursor.position();
    int queryStartPos;
    QString query = getQueryWithPosition(ui->sqlEdit->toPlainText(), pos, dialect, &queryStartPos);
    TokenList tokens = Lexer::tokenize(query, dialect);
    tokens.trim();
    tokens.trimRight(Token::OPERATOR, ";");
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
    QString sql;
    if (ui->sqlEdit->textCursor().hasSelection())
    {
        sql = ui->sqlEdit->textCursor().selectedText();
    }
    else if (CFG_UI.General.ExecuteCurrentQueryOnly.get())
    {
        selectCurrentQuery();
        sql = ui->sqlEdit->textCursor().selectedText();
    }
    else
    {
        sql = ui->sqlEdit->toPlainText();
    }

    // Forbid changing db during execution
    dbCombo->setEnabled(false);

    resultsModel->setDb(getCurrentDb());
    resultsModel->setExplainMode(explain);
    resultsModel->setQuery(sql);
    ui->dataView->refreshData();

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
    dbCombo->setEnabled(true);

    double secs = ((double)resultsModel->getExecutionTime()) / 1000;
    QString time = QString::number(secs, 'f', 3);
    notifyInfo(tr("Query finished in %2 second(s).").arg(time));

    lastQueryHistoryId = CFG->addSqlHistory(resultsModel->getQuery(), resultsModel->getDb()->getName(), resultsModel->getExecutionTime(), 0);

    // If we added first history entry - resize dates column.
    if (ui->historyList->model()->rowCount() == 1)
        ui->historyList->resizeColumnToContents(1);

    Db* currentDb = getCurrentDb();
    if (currentDb)
        DBTREE->refreshSchema(currentDb);
}

void EditorWindow::executionFailed(const QString &errorText)
{
    dbCombo->setEnabled(true);
    notifyError(errorText);
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
    CFG->clearSqlHistory();
}

void EditorWindow::exportResults()
{
    qDebug() << "not implemented"; // TODO implement exportResults
}

void EditorWindow::createViewFromQuery()
{
    qDebug() << "not implemented"; // TODO implement createViewFromQuery
}

int qHash(EditorWindow::ActionGroup actionGroup)
{
    return static_cast<int>(actionGroup);
}
