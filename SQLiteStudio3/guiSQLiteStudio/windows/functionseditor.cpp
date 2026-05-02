#include "functionseditor.h"
#include "ui_functionseditor.h"
#include "common/utils.h"
#include "uiutils.h"
#include "functionseditormodel.h"
#include "services/pluginmanager.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include "iconmanager.h"
#include "syntaxhighlighterplugin.h"
#include "plugins/scriptingplugin.h"
#include "common/userinputfilter.h"
#include "selectabledbmodel.h"
#include "uiconfig.h"
#include "dialogs/settingsexportdialog.h"
#include "dialogs/settingsimportdialog.h"
#include <QDebug>
#include <QDesktopServices>
#include <QStyleFactory>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtSystemDetection>
#else
#include <qsystemdetection.h>
#endif
#include <QSyntaxHighlighter>

// TODO handle plugin loading/unloading to update editor state

CFG_KEYS_DEFINE(FunctionsEditor)

FunctionsEditor::FunctionsEditor(QWidget *parent) :
    MdiChild(parent),
    ui(new Ui::FunctionsEditor)
{
    init();
}

FunctionsEditor::~FunctionsEditor()
{
    delete ui;
}

bool FunctionsEditor::restoreSessionNextTime()
{
    return false;
}

bool FunctionsEditor::restoreSession(const QVariant &sessionValue)
{
    Q_UNUSED(sessionValue);
    return true;
}

Icon* FunctionsEditor::getIconNameForMdiWindow()
{
    return ICONS.FUNCTIONS_EDITOR;
}

QString FunctionsEditor::getTitleForMdiWindow()
{
    return tr("SQL functions editor");
}

void FunctionsEditor::createActions()
{
    createAction(COMMIT, ICONS.COMMIT, tr("Commit all function changes"), this, SLOT(commit()), ui->toolBar, this);
    createAction(ROLLBACK, ICONS.ROLLBACK, tr("Rollback all function changes"), this, SLOT(rollback()), ui->toolBar, this);
    ui->toolBar->addSeparator();
    createAction(ADD, ICONS.NEW_FUNCTION, tr("Create new function"), this, SLOT(newFunction()), ui->toolBar, this);
    createAction(DELETE, ICONS.DELETE_FUNCTION, tr("Delete selected function"), this, SLOT(deleteFunction()), ui->toolBar, this);
    ui->toolBar->addSeparator();
    createAction(IMPORT, ICONS.FUNCTIONS_IMPORT, tr("Import functions from file"), this, SLOT(importFunctions()), ui->toolBar, this);
    createAction(EXPORT, ICONS.FUNCTIONS_EXPORT, tr("Export functions to file"), this, SLOT(exportFunctions()), ui->toolBar, this);
    createAction(HELP, ICONS.HELP, tr("Custom SQL functions manual"), this, SLOT(help()), ui->toolBar, this);

    // Args toolbar
    createAction(ARG_ADD, ICONS.INSERT_FN_ARG, tr("Add function argument"), this, SLOT(addFunctionArg()), ui->argsToolBar, this);
    createAction(ARG_EDIT, ICONS.RENAME_FN_ARG, tr("Rename function argument"), this, SLOT(editFunctionArg()), ui->argsToolBar, this);
    createAction(ARG_DEL, ICONS.DELETE_FN_ARG, tr("Delete function argument"), this, SLOT(delFunctionArg()), ui->argsToolBar, this);
    ui->argsToolBar->addSeparator();
    createAction(ARG_MOVE_UP, ICONS.MOVE_UP, tr("Move function argument up"), this, SLOT(moveFunctionArgUp()), ui->argsToolBar, this);
    createAction(ARG_MOVE_DOWN, ICONS.MOVE_DOWN, tr("Move function argument down"), this, SLOT(moveFunctionArgDown()), ui->argsToolBar, this);

#ifdef Q_OS_MACX
    QStyle *fusion = QStyleFactory::create("Fusion");
    ui->toolBar->setStyle(fusion);
    ui->argsToolBar->setStyle(fusion);
#endif
}

void FunctionsEditor::setupDefShortcuts()
{
    // Widget context
    setShortcutContext({COMMIT, ROLLBACK}, Qt::WidgetWithChildrenShortcut);
    BIND_SHORTCUTS(FunctionsEditor, Action);
}

QToolBar* FunctionsEditor::getToolBar(int toolbar) const
{
    Q_UNUSED(toolbar);
    return ui->toolBar;
}

void FunctionsEditor::init()
{
    ui->setupUi(this);
    clearEdits();
    initCodeTabs();

    setFont(CFG_UI.Fonts.SqlEditor.get());

    model = new FunctionsEditorModel(this);
    functionFilterModel = new QSortFilterProxyModel(this);
    functionFilterModel->setSourceModel(model);
    ui->list->setModel(functionFilterModel);
    ui->list->horizontalHeader()->setMinimumSectionSize(20);
    ui->list->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->splitter->setSizes({1, 1});
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);
    Cfg::handleSplitterState(ui->splitter);
    Cfg::handleSplitterState(ui->splitter_2);

    dbListModel = new SelectableDbModel(this);
    dbListModel->setSourceModel(DBTREE->getModel());
    ui->databasesList->setModel(dbListModel);
    ui->databasesList->expandAll();

    for (auto t : {
         FunctionManager::ScriptFunction::SCALAR,
         FunctionManager::ScriptFunction::AGGREGATE,
         FunctionManager::ScriptFunction::AGG_WINDOW})
    {
        ui->typeCombo->addItem(FunctionManager::FunctionBase::displayString(t), t);
    }

    new UserInputFilter(ui->functionFilterEdit, this, SLOT(applyFilter(QString)));
    functionFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    MAINWINDOW->installToolbarSizeWheelHandler(ui->toolBar);

    initActions();
    setupContextMenu();

    connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(functionSelected(QItemSelection,QItemSelection)));
    connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateState()));
    connect(ui->initCodeEdit, SIGNAL(textChanged()), this, SLOT(updateModified()));
    connect(ui->inverseCodeEdit, SIGNAL(textChanged()), this, SLOT(updateModified()));
    connect(ui->stepCodeEdit, SIGNAL(textChanged()), this, SLOT(updateModified()));
    connect(ui->scalarCodeEdit, SIGNAL(textChanged()), this, SLOT(updateModified()));
    connect(ui->finalCodeEdit, SIGNAL(textChanged()), this, SLOT(updateModified()));
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(updateModified()));
    connect(ui->undefArgsCheck, SIGNAL(toggled(bool)), this, SLOT(updateModified()));
    connect(ui->allDatabasesRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->selDatabasesRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->langCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateModified()));
    connect(ui->typeCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateModified()));
    connect(ui->deterministicCheck, SIGNAL(toggled(bool)), this, SLOT(updateModified()));

    connect(ui->argsList->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateArgsState()));
    connect(ui->argsList->model(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(updateModified()));
    connect(ui->argsList->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateModified()));
    connect(ui->argsList->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(updateModified()));
    connect(ui->argsList->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(updateModified()));

    connect(dbListModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateModified()));
    connect(CFG_UI.Fonts.SqlEditor, SIGNAL(changed(QVariant)), this, SLOT(changeFont(QVariant)));

    model->setData(FUNCTIONS->getAllScriptFunctions());
    connect(FUNCTIONS, SIGNAL(functionListChanged()), this, SLOT(cfgFunctionListChanged()));
    ui->list->resizeColumnsToContents();

    // Language plugins
    for (ScriptingPlugin*& plugin : PLUGINS->getLoadedPlugins<ScriptingPlugin>())
        scriptingPlugins[plugin->getLanguage()] = plugin;

    ui->langCombo->addItems(scriptingPlugins.keys());

    // Syntax highlighting plugins
    for (SyntaxHighlighterPlugin*& plugin : PLUGINS->getLoadedPlugins<SyntaxHighlighterPlugin>())
        highlighterPlugins[plugin->getLanguageName()] = plugin;

    updateCurrentFunctionState();
    updateState();
}

void FunctionsEditor::initCodeTabs()
{
    tabIdx[SCALAR] = ui->codeTabs->indexOf(ui->scalarCodeTab);
    tabIdx[INIT] = ui->codeTabs->indexOf(ui->initCodeTab);
    tabIdx[STEP] = ui->codeTabs->indexOf(ui->stepCodeTab);
    tabIdx[INVERSE] = ui->codeTabs->indexOf(ui->inverseCodeTab);
    tabIdx[FINAL] = ui->codeTabs->indexOf(ui->finalCodeTab);

    ui->codeTabs->setTabVisible(tabIdx[INIT], false);
    ui->codeTabs->setTabVisible(tabIdx[STEP], false);
    ui->codeTabs->setTabVisible(tabIdx[INVERSE], false);
    ui->codeTabs->setTabVisible(tabIdx[FINAL], false);
}

void FunctionsEditor::setupContextMenu()
{
    auto formatPredicateFn = [this](QPlainTextEdit* editor)
    {
        return currentHighlighterLang == "SQL";
    };

    addFormatSqlToContextMenu(ui->scalarCodeEdit, formatPredicateFn);
    addFormatSqlToContextMenu(ui->initCodeEdit, formatPredicateFn);
    addFormatSqlToContextMenu(ui->inverseCodeEdit, formatPredicateFn);
    addFormatSqlToContextMenu(ui->stepCodeEdit, formatPredicateFn);
    addFormatSqlToContextMenu(ui->finalCodeEdit, formatPredicateFn);
}

int FunctionsEditor::getCurrentFunctionRow() const
{
    QModelIndexList idxList = ui->list->selectionModel()->selectedIndexes();
    if (idxList.size() == 0)
        return -1;

    return fnRowToSrc(idxList.first()).row();
}

void FunctionsEditor::functionDeselected(int srcRow)
{
    model->setName(srcRow, ui->nameEdit->text());
    model->setUndefinedArgs(srcRow, ui->undefArgsCheck->isChecked());
    if (!ui->undefArgsCheck->isChecked())
        model->setArguments(srcRow, getCurrentArgList());

    model->setLang(srcRow, ui->langCombo->currentText());
    model->setType(srcRow, getCurrentFunctionType());
    model->setAllDatabases(srcRow, ui->allDatabasesRadio->isChecked());
    model->setDeterministic(srcRow, ui->deterministicCheck->isChecked());
    model->setModified(srcRow, currentModified);

    if (model->isAggregateWindow(srcRow))
    {
        model->setInitCode(srcRow, ui->initCodeEdit->toPlainText());
        model->setStepCode(srcRow, ui->stepCodeEdit->toPlainText());
        model->setInverseCode(srcRow, ui->inverseCodeEdit->toPlainText());
        model->setFinalCode(srcRow, ui->finalCodeEdit->toPlainText());
        // Do not clear "code" field in model, as it is shared for step code
    }
    else if (model->isAggregate(srcRow))
    {
        model->setInitCode(srcRow, ui->initCodeEdit->toPlainText());
        model->setStepCode(srcRow, ui->stepCodeEdit->toPlainText());
        model->setFinalCode(srcRow, ui->finalCodeEdit->toPlainText());
        model->setInverseCode(srcRow, QString());
        // Do not clear "code" field in model, as it is shared for step code
    }
    else
    {
        model->setCode(srcRow, ui->scalarCodeEdit->toPlainText());
        model->setInitCode(srcRow, QString());
        model->setInverseCode(srcRow, QString());
        model->setFinalCode(srcRow, QString());
    }

    if (ui->selDatabasesRadio->isChecked())
        model->setDatabases(srcRow, getCurrentDatabases());

    model->validateNames();
}

void FunctionsEditor::functionSelected(int srcRow)
{
    updatesForSelection = true;
    ui->nameEdit->setText(model->getName(srcRow));
    if (model->isAnyAggregate(srcRow))
    {
        ui->stepCodeEdit->setPlainText(model->getStepCode(srcRow));
        ui->scalarCodeEdit->clear();
    }
    else
    {
        ui->scalarCodeEdit->setPlainText(model->getCode(srcRow));
        ui->stepCodeEdit->clear();
    }
    ui->initCodeEdit->setPlainText(model->getInitCode(srcRow));
    ui->inverseCodeEdit->setPlainText(model->getInverseCode(srcRow));
    ui->finalCodeEdit->setPlainText(model->getFinalCode(srcRow));
    ui->undefArgsCheck->setChecked(model->getUndefinedArgs(srcRow));
    ui->langCombo->setCurrentText(model->getLang(srcRow));
    ui->deterministicCheck->setChecked(model->isDeterministic(srcRow));

    // Arguments
    ui->argsList->clear();
    QListWidgetItem* item = nullptr;
    for (const QString& arg : model->getArguments(srcRow))
    {
        item = new QListWidgetItem(arg);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->argsList->addItem(item);
    }

    // Databases
    dbListModel->setDatabases(model->getDatabases(srcRow));
    ui->databasesList->expandAll();

    if (model->getAllDatabases(srcRow))
        ui->allDatabasesRadio->setChecked(true);
    else
        ui->selDatabasesRadio->setChecked(true);

    // Type
    FunctionManager::ScriptFunction::Type type = model->getType(srcRow);
    for (int i = 0; i < ui->typeCombo->count(); i++)
    {
        if (ui->typeCombo->itemData(i).toInt() == type)
        {
            ui->typeCombo->setCurrentIndex(i);
            break;
        }
    }

    updatesForSelection = false;
    currentModified = model->isModified(srcRow);

    updateCurrentFunctionState();
}

void FunctionsEditor::clearEdits()
{
    ui->nameEdit->setText(QString());
    ui->scalarCodeEdit->setPlainText(QString());
    ui->initCodeEdit->setPlainText(QString());
    ui->stepCodeEdit->setPlainText(QString());
    ui->inverseCodeEdit->setPlainText(QString());
    ui->finalCodeEdit->setPlainText(QString());
    ui->langCombo->setCurrentText(QString());
    ui->undefArgsCheck->setChecked(true);
    ui->argsList->clear();
    ui->allDatabasesRadio->setChecked(true);
    ui->typeCombo->setCurrentIndex(0);
    ui->langCombo->setCurrentIndex(-1);
    ui->deterministicCheck->setChecked(false);
}

void FunctionsEditor::selectFunction(int srcRow)
{
    if (!model->isValidRowIndex(srcRow))
        return;

    ui->list->selectionModel()->setCurrentIndex(functionFilterModel->mapFromSource(model->index(srcRow)), QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
}

void FunctionsEditor::setFont(const QFont& font)
{
    ui->scalarCodeEdit->setFont(font);
    ui->initCodeEdit->setFont(font);
    ui->stepCodeEdit->setFont(font);
    ui->inverseCodeEdit->setFont(font);
    ui->finalCodeEdit->setFont(font);
}

QModelIndex FunctionsEditor::fnRowToSrc(const QModelIndex &idx) const
{
    return functionFilterModel->mapToSource(idx);
}

QModelIndex FunctionsEditor::getSelectedArg() const
{
    QModelIndexList indexes = ui->argsList->selectionModel()->selectedIndexes();
    if (indexes.size() == 0 || !indexes.first().isValid())
        return QModelIndex();

    return indexes.first();

}

QStringList FunctionsEditor::getCurrentArgList() const
{
    QStringList currArgList;
    for (int row = 0; row < ui->argsList->model()->rowCount(); row++)
        currArgList << ui->argsList->item(row)->text();

    return currArgList;
}

QStringList FunctionsEditor::getCurrentDatabases() const
{
    return dbListModel->getDatabases();
}

FunctionManager::ScriptFunction::Type FunctionsEditor::getCurrentFunctionType() const
{
    int intValue = ui->typeCombo->itemData(ui->typeCombo->currentIndex()).toInt();
    return static_cast<FunctionManager::ScriptFunction::Type>(intValue);
}

void FunctionsEditor::safeClearHighlighter(QSyntaxHighlighter*& highlighterPtr)
{
    // A pointers swap with local var - this is necessary, cause deleting highlighter
    // triggers textChanged on QPlainTextEdit, which then calls this method,
    // so it becomes an infinite recursion with deleting the same pointer.
    // We set the pointer to null first, then delete it. That way it's safe.
    QSyntaxHighlighter* highlighter = highlighterPtr;
    highlighterPtr = nullptr;
    delete highlighter;
}

void FunctionsEditor::commit()
{
    int srcRow = getCurrentFunctionRow();
    if (model->isValidRowIndex(srcRow))
        functionDeselected(srcRow);

    QList<FunctionManager::ScriptFunction*> functions = model->generateFunctions();

    FUNCTIONS->setScriptFunctions(functions);
    model->clearModified();
    currentModified = false;

    if (model->isValidRowIndex(srcRow))
        selectFunction(srcRow);

    updateState();
    ui->list->resizeColumnsToContents();
}

void FunctionsEditor::rollback()
{
    int selectedBefore = getCurrentFunctionRow();

    model->setData(FUNCTIONS->getAllScriptFunctions());
    currentModified = false;
    clearEdits();

    if (model->isValidRowIndex(selectedBefore))
        selectFunction(selectedBefore);

    updateState();
}

void FunctionsEditor::newFunction()
{
    if (ui->langCombo->currentIndex() == -1 && ui->langCombo->count() > 0)
        ui->langCombo->setCurrentIndex(0);

    FunctionManager::ScriptFunction* func = new FunctionManager::ScriptFunction();
    func->name = generateUniqueName("function", model->getFunctionNames());

    if (ui->langCombo->currentIndex() > -1)
        func->lang = ui->langCombo->currentText();

    model->addFunction(func);

    selectFunction(model->rowCount() - 1);
}

void FunctionsEditor::deleteFunction()
{
    int srcRow = getCurrentFunctionRow();
    model->deleteFunction(srcRow);

    srcRow = getCurrentFunctionRow();
    if (model->isValidRowIndex(srcRow))
        functionSelected(srcRow);
    else
        clearEdits();

    updateState();
}

void FunctionsEditor::updateModified()
{
    if (updatesForSelection)
        return;

    int row = getCurrentFunctionRow();
    if (model->isValidRowIndex(row))
    {
        bool nameDiff = model->getName(row) != ui->nameEdit->text();

        bool codeDiff = false;
        bool stepCodeDiff = false;
        if (model->isAnyAggregate(row))
            stepCodeDiff = model->getStepCode(row) != ui->stepCodeEdit->toPlainText();
        else
            codeDiff = model->getCode(row) != ui->scalarCodeEdit->toPlainText();

        bool initCodeDiff = model->getInitCode(row) != ui->initCodeEdit->toPlainText();
        bool inverseCodeDiff = model->getInverseCode(row) != ui->inverseCodeEdit->toPlainText();
        bool finalCodeDiff = model->getFinalCode(row) != ui->finalCodeEdit->toPlainText();
        bool langDiff = model->getLang(row) != ui->langCombo->currentText();
        bool undefArgsDiff = model->getUndefinedArgs(row) != ui->undefArgsCheck->isChecked();
        bool allDatabasesDiff = model->getAllDatabases(row) != ui->allDatabasesRadio->isChecked();
        bool argDiff = getCurrentArgList() != model->getArguments(row);
        bool dbDiff = toSet(getCurrentDatabases()) != toSet(model->getDatabases(row)); // QSet to ignore order
        bool typeDiff = model->getType(row) != getCurrentFunctionType();
        bool deterministicDiff = model->isDeterministic(row) != ui->deterministicCheck->isChecked();

        currentModified = (nameDiff || codeDiff || typeDiff || langDiff || undefArgsDiff || allDatabasesDiff || argDiff || dbDiff ||
                           initCodeDiff || finalCodeDiff || stepCodeDiff || inverseCodeDiff || deterministicDiff);

        if (langDiff)
            model->setLang(row, ui->langCombo->currentText());
    }

    updateCurrentFunctionState();
}

void FunctionsEditor::updateState()
{
    bool modified = model->isModified() || currentModified;
    bool valid = model->isValid();

    actionMap[COMMIT]->setEnabled(modified && valid);
    actionMap[ROLLBACK]->setEnabled(modified);
    actionMap[DELETE]->setEnabled(ui->list->selectionModel()->selectedIndexes().size() > 0);
}

void FunctionsEditor::updateCurrentFunctionState()
{
    int srcRow = getCurrentFunctionRow();
    bool validRow = model->isValidRowIndex(srcRow);
    ui->rightWidget->setEnabled(validRow);
    if (!validRow)
    {
        setValidState(ui->langCombo, true);
        setValidState(ui->nameEdit, true);
        setValidState(ui->scalarCodeTab, true);
        setValidState(ui->stepCodeTab, true);
        setValidState(ui->finalCodeTab, true);
        return;
    }

    QString name = ui->nameEdit->text();
    QStringList argList = getCurrentArgList();
    bool undefArgs = ui->undefArgsCheck->isChecked();
    bool nameOk = model->isAllowedName(srcRow, name, argList, undefArgs) && !name.trimmed().isEmpty();
    setValidState(ui->nameEdit, nameOk, tr("Enter a unique, non-empty function name. Duplicate names are allowed if the number of input parameters differs."));

    bool langOk = ui->langCombo->currentIndex() >= 0;
    ui->codeTabs->setEnabled(langOk);
    ui->argsGroup->setEnabled(langOk);
    ui->deterministicCheck->setEnabled(langOk);
    ui->databasesGroup->setEnabled(langOk);
    ui->nameEdit->setEnabled(langOk);
    ui->nameLabel->setEnabled(langOk);
    ui->typeCombo->setEnabled(langOk);
    ui->typeLabel->setEnabled(langOk);
    setValidState(ui->langCombo, langOk, tr("Pick the implementation language."));

    FunctionManager::FunctionBase::Type funType = getCurrentFunctionType();
    bool aggWindow = funType == FunctionManager::ScriptFunction::AGG_WINDOW;
    bool aggregate = funType == FunctionManager::ScriptFunction::AGGREGATE || aggWindow;
    ui->codeTabs->setTabVisible(tabIdx[SCALAR], !aggregate);
    ui->codeTabs->setTabVisible(tabIdx[INIT], aggregate);
    ui->codeTabs->setTabVisible(tabIdx[STEP], aggregate);
    ui->codeTabs->setTabVisible(tabIdx[INVERSE], aggWindow);
    ui->codeTabs->setTabVisible(tabIdx[FINAL], aggregate);

    ui->databasesList->setEnabled(ui->selDatabasesRadio->isChecked());

    // Declare mandatory code fields
    bool codeOk = true;
    if (!aggregate)
        codeOk = !ui->scalarCodeEdit->toPlainText().trimmed().isEmpty();

    setValidState(ui->scalarCodeTab, codeOk, tr("Enter a non-empty implementation code."));

    bool stepCodeOk = true;
    bool finalCodeOk = true;
    if (aggregate)
    {
        stepCodeOk = !ui->stepCodeEdit->toPlainText().trimmed().isEmpty();
        finalCodeOk = !ui->finalCodeEdit->toPlainText().trimmed().isEmpty();
    }

    setValidState(ui->stepCodeTab, stepCodeOk, tr("Enter a non-empty implementation code."));
    setValidState(ui->finalCodeTab, finalCodeOk, tr("Enter a non-empty implementation code."));

    // Syntax highlighter
    QString lang = ui->langCombo->currentText();
    if (lang != currentHighlighterLang)
    {
        safeClearHighlighter(currentScalarHighlighter);
        safeClearHighlighter(currentInitHighlighter);
        safeClearHighlighter(currentStepHighlighter);
        safeClearHighlighter(currentInverseHighlighter);
        safeClearHighlighter(currentFinalHighlighter);

        if (langOk && highlighterPlugins.contains(lang))
        {
            currentScalarHighlighter = highlighterPlugins[lang]->createSyntaxHighlighter(ui->scalarCodeEdit);
            currentInitHighlighter = highlighterPlugins[lang]->createSyntaxHighlighter(ui->initCodeEdit);
            currentStepHighlighter = highlighterPlugins[lang]->createSyntaxHighlighter(ui->stepCodeEdit);
            currentInverseHighlighter = highlighterPlugins[lang]->createSyntaxHighlighter(ui->inverseCodeEdit);
            currentFinalHighlighter = highlighterPlugins[lang]->createSyntaxHighlighter(ui->finalCodeEdit);
        }

        currentHighlighterLang = lang;
    }

    bool argsOk = updateArgsState();
    model->setValid(srcRow, langOk && codeOk && stepCodeOk && finalCodeOk && nameOk && argsOk);
    updateState();
}

void FunctionsEditor::functionSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    int deselCnt = deselected.indexes().size();
    int selCnt = selected.indexes().size();

    if (deselCnt > 0)
        functionDeselected(fnRowToSrc(deselected.indexes().first()).row());

    if (selCnt > 0)
        functionSelected(fnRowToSrc(selected.indexes().first()).row());

    if (deselCnt > 0 && selCnt == 0)
    {
        currentModified = false;
        clearEdits();
    }
}

void FunctionsEditor::addFunctionArg()
{
    QListWidgetItem* item = new QListWidgetItem(tr("argument", "new function argument name in function editor window"));
    item->setFlags(item->flags () | Qt::ItemIsEditable);
    ui->argsList->addItem(item);

    QModelIndex idx = ui->argsList->model()->index(ui->argsList->model()->rowCount() - 1, 0);
    ui->argsList->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);

    ui->argsList->editItem(item);
}

void FunctionsEditor::editFunctionArg()
{
    QModelIndex selected = getSelectedArg();
    if (!selected.isValid())
        return;

    int row = selected.row();
    QListWidgetItem* item = ui->argsList->item(row);
    ui->argsList->editItem(item);
}

void FunctionsEditor::delFunctionArg()
{
    QModelIndex selected = getSelectedArg();
    if (!selected.isValid())
        return;

    int row = selected.row();
    delete ui->argsList->takeItem(row);
}

void FunctionsEditor::moveFunctionArgUp()
{
    QModelIndex selected = getSelectedArg();
    if (!selected.isValid())
        return;

    int row = selected.row();
    if (row <= 0)
        return;

    ui->argsList->insertItem(row - 1, ui->argsList->takeItem(row));

    QModelIndex idx = ui->argsList->model()->index(row - 1, 0);
    ui->argsList->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
}

void FunctionsEditor::moveFunctionArgDown()
{
    QModelIndex selected = getSelectedArg();
    if (!selected.isValid())
        return;

    int row = selected.row();
    if (row >= ui->argsList->model()->rowCount() - 1)
        return;

    ui->argsList->insertItem(row + 1, ui->argsList->takeItem(row));

    QModelIndex idx = ui->argsList->model()->index(row + 1, 0);
    ui->argsList->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
}

bool FunctionsEditor::updateArgsState()
{
    bool argsEnabled = !ui->undefArgsCheck->isChecked();
    QModelIndexList indexes = ui->argsList->selectionModel()->selectedIndexes();
    bool argSelected = indexes.size() > 0;

    bool canMoveUp = false;
    bool canMoveDown = false;
    if (argSelected)
    {
        canMoveUp = indexes.first().row() > 0;
        canMoveDown = (indexes.first().row() + 1) < ui->argsList->count();
    }

    actionMap[ARG_ADD]->setEnabled(argsEnabled);
    actionMap[ARG_EDIT]->setEnabled(argsEnabled && argSelected);
    actionMap[ARG_DEL]->setEnabled(argsEnabled && argSelected);
    actionMap[ARG_MOVE_UP]->setEnabled(argsEnabled && canMoveUp);
    actionMap[ARG_MOVE_DOWN]->setEnabled(argsEnabled && canMoveDown);
    ui->argsList->setEnabled(argsEnabled);

    if (argsEnabled)
    {
        bool argsOk = true;
        QSet<QString> usedNames;
        for (int rowIdx = 0; rowIdx < ui->argsList->model()->rowCount(); rowIdx++)
        {
            QListWidgetItem* item = ui->argsList->item(rowIdx);
            QString argName = item->text().toLower();
            if (argName.isEmpty())
            {
                argsOk = false;
                break;
            }
            if (usedNames.contains(argName))
            {
                argsOk = false;
                break;
            }
            usedNames << argName;
        }
        setValidState(ui->argsList, argsOk, tr("Function argument cannot be empty and it cannot have duplicated name."));
        return argsOk;
    }
    else
        return true;
}

void FunctionsEditor::applyFilter(const QString& value)
{
    // Remembering old selection, clearing it and restoring afterwards is a workaround for a problem,
    // which causes application to crash, when the item was selected, but after applying filter string,
    // item was about to disappear.
    // This must have something to do with the underlying model (FunctionsEditorModel) implementation,
    // but for now I don't really know what is that.
    // I have tested simple Qt application with the same routine, but the underlying model was QStandardItemModel
    // and everything worked fine.
    int srcRow = getCurrentFunctionRow();
    ui->list->selectionModel()->clearSelection();

    functionFilterModel->setFilterFixedString(value);

    selectFunction(srcRow);
}

void FunctionsEditor::help()
{
    static const QString url = QStringLiteral("https://github.com/pawelsalawa/sqlitestudio/wiki/Custom-SQL-Functions");
    QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode));
}

void FunctionsEditor::changeFont(const QVariant& font)
{
    setFont(font.value<QFont>());
}

void FunctionsEditor::cfgFunctionListChanged()
{
    if (model->isModified())
        return; // Don't update list if there are uncommitted changes, because it would be disruptive for user. Changes will be visible after commit or rollback.

    model->setData(FUNCTIONS->getAllScriptFunctions());
    updateCurrentFunctionState();
}

void FunctionsEditor::importFunctions()
{
    SettingsImportDialog::importFromFile(SettingsImportDialog::FUNCTION);
}

void FunctionsEditor::exportFunctions()
{
    SettingsExportDialog::exportToFile(SettingsExportDialog::FUNCTION);
}

QVariant FunctionsEditor::saveSession()
{
    return QVariant();
}


bool FunctionsEditor::isUncommitted() const
{
    return model->isModified() || currentModified;
}

QString FunctionsEditor::getQuitUncommittedConfirmMessage() const
{
    return tr("Functions editor window has uncommitted modifications.");
}
