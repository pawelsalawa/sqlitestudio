#include "functionseditor.h"
#include "ui_functionseditor.h"
#include "common/unused.h"
#include "common/utils.h"
#include "common/compatibility.h"
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
#include <QDebug>
#include <QDesktopServices>
#include <QStyleFactory>
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
    UNUSED(sessionValue);
    return true;
}

Icon* FunctionsEditor::getIconNameForMdiWindow()
{
    return ICONS.FUNCTION;
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
    UNUSED(toolbar);
    return ui->toolBar;
}

void FunctionsEditor::init()
{
    ui->setupUi(this);
    clearEdits();
    ui->initCodeGroup->setVisible(false);
    ui->finalCodeGroup->setVisible(false);

    setFont(CFG_UI.Fonts.SqlEditor.get());

    model = new FunctionsEditorModel(this);
    functionFilterModel = new QSortFilterProxyModel(this);
    functionFilterModel->setSourceModel(model);
    ui->list->setModel(functionFilterModel);

    dbListModel = new SelectableDbModel(this);
    dbListModel->setSourceModel(DBTREE->getModel());
    ui->databasesList->setModel(dbListModel);
    ui->databasesList->expandAll();

    ui->typeCombo->addItem(tr("Scalar"), FunctionManager::ScriptFunction::SCALAR);
    ui->typeCombo->addItem(tr("Aggregate"), FunctionManager::ScriptFunction::AGGREGATE);

    new UserInputFilter(ui->functionFilterEdit, this, SLOT(applyFilter(QString)));
    functionFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    initActions();

    connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(functionSelected(QItemSelection,QItemSelection)));
    connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateState()));
    connect(ui->initCodeEdit, SIGNAL(textChanged()), this, SLOT(updateModified()));
    connect(ui->mainCodeEdit, SIGNAL(textChanged()), this, SLOT(updateModified()));
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

    // Language plugins
    for (ScriptingPlugin*& plugin : PLUGINS->getLoadedPlugins<ScriptingPlugin>())
        scriptingPlugins[plugin->getLanguage()] = plugin;

    ui->langCombo->addItems(scriptingPlugins.keys());

    // Syntax highlighting plugins
    for (SyntaxHighlighterPlugin*& plugin : PLUGINS->getLoadedPlugins<SyntaxHighlighterPlugin>())
        highlighterPlugins[plugin->getLanguageName()] = plugin;

    updateState();
}

int FunctionsEditor::getCurrentFunctionRow() const
{
    QModelIndexList idxList = ui->list->selectionModel()->selectedIndexes();
    if (idxList.size() == 0)
        return -1;

    return idxList.first().row();
}

void FunctionsEditor::functionDeselected(int row)
{
    model->setName(row, ui->nameEdit->text());
    model->setLang(row, ui->langCombo->currentText());
    model->setType(row, getCurrentFunctionType());
    model->setUndefinedArgs(row, ui->undefArgsCheck->isChecked());
    model->setAllDatabases(row, ui->allDatabasesRadio->isChecked());
    model->setCode(row, ui->mainCodeEdit->toPlainText());
    model->setDeterministic(row, ui->deterministicCheck->isChecked());
    model->setModified(row, currentModified);

    if (model->isAggregate(row))
    {
        model->setInitCode(row, ui->initCodeEdit->toPlainText());
        model->setFinalCode(row, ui->finalCodeEdit->toPlainText());
    }
    else
    {
        model->setInitCode(row, QString());
        model->setFinalCode(row, QString());
    }

    if (!ui->undefArgsCheck->isChecked())
        model->setArguments(row, getCurrentArgList());

    if (ui->selDatabasesRadio->isChecked())
        model->setDatabases(row, getCurrentDatabases());

    model->validateNames();
}

void FunctionsEditor::functionSelected(int row)
{
    updatesForSelection = true;
    ui->nameEdit->setText(model->getName(row));
    ui->initCodeEdit->setPlainText(model->getInitCode(row));
    ui->mainCodeEdit->setPlainText(model->getCode(row));
    ui->finalCodeEdit->setPlainText(model->getFinalCode(row));
    ui->undefArgsCheck->setChecked(model->getUndefinedArgs(row));
    ui->langCombo->setCurrentText(model->getLang(row));
    ui->deterministicCheck->setChecked(model->isDeterministic(row));

    // Arguments
    ui->argsList->clear();
    QListWidgetItem* item = nullptr;
    for (const QString& arg : model->getArguments(row))
    {
        item = new QListWidgetItem(arg);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->argsList->addItem(item);
    }

    // Databases
    dbListModel->setDatabases(model->getDatabases(row));
    ui->databasesList->expandAll();

    if (model->getAllDatabases(row))
        ui->allDatabasesRadio->setChecked(true);
    else
        ui->selDatabasesRadio->setChecked(true);

    // Type
    FunctionManager::ScriptFunction::Type type = model->getType(row);
    for (int i = 0; i < ui->typeCombo->count(); i++)
    {
        if (ui->typeCombo->itemData(i).toInt() == type)
        {
            ui->typeCombo->setCurrentIndex(i);
            break;
        }
    }

    updatesForSelection = false;
    currentModified = model->isModified(row);

    updateCurrentFunctionState();
}

void FunctionsEditor::clearEdits()
{
    ui->nameEdit->setText(QString());
    ui->mainCodeEdit->setPlainText(QString());
    ui->langCombo->setCurrentText(QString());
    ui->undefArgsCheck->setChecked(true);
    ui->argsList->clear();
    ui->allDatabasesRadio->setChecked(true);
    ui->typeCombo->setCurrentIndex(0);
    ui->langCombo->setCurrentIndex(-1);
    ui->deterministicCheck->setChecked(false);
}

void FunctionsEditor::selectFunction(int row)
{
    if (!model->isValidRowIndex(row))
        return;

    ui->list->selectionModel()->setCurrentIndex(model->index(row), QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
}

void FunctionsEditor::setFont(const QFont& font)
{
    ui->initCodeEdit->setFont(font);
    ui->mainCodeEdit->setFont(font);
    ui->finalCodeEdit->setFont(font);
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

void FunctionsEditor::commit()
{
    int row = getCurrentFunctionRow();
    if (model->isValidRowIndex(row))
        functionDeselected(row);

    QList<FunctionManager::ScriptFunction*> functions = model->generateFunctions();

    FUNCTIONS->setScriptFunctions(functions);
    model->clearModified();
    currentModified = false;

    if (model->isValidRowIndex(row))
        selectFunction(row);

    updateState();
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
    int row = getCurrentFunctionRow();
    model->deleteFunction(row);
    clearEdits();

    row = getCurrentFunctionRow();
    if (model->isValidRowIndex(row))
        functionSelected(row);

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
        bool codeDiff = model->getCode(row) != ui->mainCodeEdit->toPlainText();
        bool initCodeDiff = model->getInitCode(row) != ui->initCodeEdit->toPlainText();
        bool finalCodeDiff = model->getFinalCode(row) != ui->finalCodeEdit->toPlainText();
        bool langDiff = model->getLang(row) != ui->langCombo->currentText();
        bool undefArgsDiff = model->getUndefinedArgs(row) != ui->undefArgsCheck->isChecked();
        bool allDatabasesDiff = model->getAllDatabases(row) != ui->allDatabasesRadio->isChecked();
        bool argDiff = getCurrentArgList() != model->getArguments(row);
        bool dbDiff = toSet(getCurrentDatabases()) != toSet(model->getDatabases(row)); // QSet to ignore order
        bool typeDiff = model->getType(row) != getCurrentFunctionType();
        bool deterministicDiff = model->isDeterministic(row) != ui->deterministicCheck->isChecked();

        currentModified = (nameDiff || codeDiff || typeDiff || langDiff || undefArgsDiff || allDatabasesDiff || argDiff || dbDiff ||
                           initCodeDiff || finalCodeDiff || deterministicDiff);
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
    int row = getCurrentFunctionRow();
    bool validRow = model->isValidRowIndex(row);
    ui->rightWidget->setEnabled(validRow);
    if (!validRow)
    {
        setValidState(ui->langCombo, true);
        setValidState(ui->nameEdit, true);
        setValidState(ui->mainCodeEdit, true);
        setValidState(ui->finalCodeEdit, true);
        return;
    }

    QString name = ui->nameEdit->text();
    bool nameOk = model->isAllowedName(row, name) && !name.trimmed().isEmpty();
    setValidState(ui->nameEdit, nameOk, tr("Enter a non-empty, unique name of the function."));

    bool langOk = ui->langCombo->currentIndex() >= 0;
    ui->initCodeGroup->setEnabled(langOk);
    ui->mainCodeGroup->setEnabled(langOk);
    ui->finalCodeGroup->setEnabled(langOk);
    ui->argsGroup->setEnabled(langOk);
    ui->deterministicCheck->setEnabled(langOk);
    ui->databasesGroup->setEnabled(langOk);
    ui->nameEdit->setEnabled(langOk);
    ui->nameLabel->setEnabled(langOk);
    ui->typeCombo->setEnabled(langOk);
    ui->typeLabel->setEnabled(langOk);
    setValidState(ui->langCombo, langOk, tr("Pick the implementation language."));

    bool aggregate = getCurrentFunctionType() == FunctionManager::ScriptFunction::AGGREGATE;
    ui->initCodeGroup->setVisible(aggregate);
    ui->mainCodeGroup->setTitle(aggregate ? tr("Per step code:") : tr("Function implementation code:"));
    ui->finalCodeGroup->setVisible(aggregate);

    ui->databasesList->setEnabled(ui->selDatabasesRadio->isChecked());

    bool codeOk = !ui->mainCodeEdit->toPlainText().trimmed().isEmpty();
    setValidState(ui->mainCodeEdit, codeOk, tr("Enter a non-empty implementation code."));

    bool finalCodeOk = true;
    if (aggregate)
        finalCodeOk = !ui->finalCodeEdit->toPlainText().trimmed().isEmpty();

    setValidState(ui->finalCodeEdit, finalCodeOk);

    // Syntax highlighter
    QString lang = ui->langCombo->currentText();
    if (lang != currentHighlighterLang)
    {
        QSyntaxHighlighter* highlighter = nullptr;
        if (currentMainHighlighter)
        {
            // A pointers swap with local var - this is necessary, cause deleting highlighter
            // triggers textChanged on QPlainTextEdit, which then calls this method,
            // so it becomes an infinite recursion with deleting the same pointer.
            // We set the pointer to null first, then delete it. That way it's safe.
            highlighter = currentMainHighlighter;
            currentMainHighlighter = nullptr;
            delete highlighter;
        }

        if (currentFinalHighlighter)
        {
            highlighter = currentFinalHighlighter;
            currentFinalHighlighter = nullptr;
            delete highlighter;
        }

        if (currentInitHighlighter)
        {
            highlighter = currentInitHighlighter;
            currentInitHighlighter = nullptr;
            delete highlighter;
        }

        if (langOk && highlighterPlugins.contains(lang))
        {
            currentInitHighlighter = highlighterPlugins[lang]->createSyntaxHighlighter(ui->initCodeEdit);
            currentMainHighlighter = highlighterPlugins[lang]->createSyntaxHighlighter(ui->mainCodeEdit);
            currentFinalHighlighter = highlighterPlugins[lang]->createSyntaxHighlighter(ui->finalCodeEdit);
        }

        currentHighlighterLang = lang;
    }

    bool argsOk = updateArgsState();
    model->setValid(row, langOk && codeOk && finalCodeOk && nameOk && argsOk);
    updateState();
}

void FunctionsEditor::functionSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    int deselCnt = deselected.indexes().size();
    int selCnt = selected.indexes().size();

    if (deselCnt > 0)
        functionDeselected(deselected.indexes().first().row());

    if (selCnt > 0)
        functionSelected(selected.indexes().first().row());

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
    // which causees application to crash, when the item was selected, but after applying filter string,
    // item was about to disappear.
    // This must have something to do with the underlying model (FunctionsEditorModel) implementation,
    // but for now I don't really know what is that.
    // I have tested simple Qt application with the same routine, but the underlying model was QStandardItemModel
    // and everything worked fine.
    int row = getCurrentFunctionRow();
    ui->list->selectionModel()->clearSelection();

    functionFilterModel->setFilterFixedString(value);

    selectFunction(row);
}

void FunctionsEditor::help()
{
    static const QString url = QStringLiteral("https://github.com/pawelsalawa/sqlitestudio/wiki/User_Manual#custom-sql-functions");
    QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode));
}

void FunctionsEditor::changeFont(const QVariant& font)
{
    setFont(font.value<QFont>());
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
