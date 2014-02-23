#include "functionseditor.h"
#include "ui_functionseditor.h"
#include "unused.h"
#include "utils.h"
#include "uiutils.h"
#include "functionseditormodel.h"
#include "pluginmanager.h"
#include "sqlfunctionplugin.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include "dbtree/dbtreeitem.h"
#include "iconmanager.h"
#include "syntaxhighlighterplugin.h"
#include "sqlitesyntaxhighlighter.h"
#include <QDebug>

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

QString FunctionsEditor::getIconNameForMdiWindow()
{
    return "function";
}

QString FunctionsEditor::getTitleForMdiWindow()
{
    return tr("SQL function editor");
}

void FunctionsEditor::createActions()
{
    createAction(COMMIT, "commit", tr("Commit all function changes"), this, SLOT(commit()), ui->toolBar);
    createAction(ROLLBACK, "rollback", tr("Rollback all function changes"), this, SLOT(rollback()), ui->toolBar);
    ui->toolBar->addSeparator();
    createAction(ADD, "new_function", tr("Create new function"), this, SLOT(newFunction()), ui->toolBar);
    createAction(DELETE, "delete_function", tr("Delete selected function"), this, SLOT(deleteFunction()), ui->toolBar);

    // Args toolbar
    createAction(ARG_ADD, "insert_fn_arg", tr("Add function argument"), this, SLOT(addFunctionArg()), ui->argsToolBar);
    createAction(ARG_EDIT, "rename_fn_arg", tr("Rename function argument"), this, SLOT(editFunctionArg()), ui->argsToolBar);
    createAction(ARG_DEL, "delete_fn_arg", tr("Delete function argument"), this, SLOT(delFunctionArg()), ui->argsToolBar);
    ui->toolBar->addSeparator();
    createAction(ARG_MOVE_UP, "move_up", tr("Move function argument up"), this, SLOT(moveFunctionArgUp()), ui->argsToolBar);
    createAction(ARG_MOVE_DOWN, "move_down", tr("Move function argument down"), this, SLOT(moveFunctionArgDown()), ui->argsToolBar);
}

void FunctionsEditor::setupDefShortcuts()
{
}

void FunctionsEditor::init()
{
    ui->setupUi(this);
    clearEdits();

    model = new FunctionsEditorModel(this);
    ui->list->setModel(model);

    dbListModel = new DbModel(this);
    dbListModel->setSourceModel(DBTREE->getModel());
    ui->databasesList->setModel(dbListModel);
    ui->databasesList->expandAll();

    ui->typeCombo->addItem(tr("Scalar"));
    ui->typeCombo->addItem(tr("Aggregate"));

    initActions();

    connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(functionSelected(QItemSelection,QItemSelection)));
    connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateState()));
    connect(ui->edit, SIGNAL(textChanged()), this, SLOT(updateModified()));
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(validateName()));
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(updateModified()));
    connect(ui->undefArgsCheck, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->allDatabasesRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->selDatabasesRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->langCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateModified()));
    connect(ui->typeCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateModified()));

    connect(ui->argsList->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateArgsState()));
    connect(ui->argsList->model(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(updateModified()));
    connect(ui->argsList->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateModified()));
    connect(ui->argsList->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(updateModified()));
    connect(ui->argsList->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(updateModified()));

    connect(dbListModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateModified()));

    model->setData(CFG->getFunctions());

    // Language plugins
    foreach (SqlFunctionPlugin* plugin, PLUGINS->getLoadedPlugins<SqlFunctionPlugin>())
        functionPlugins[plugin->getLanguageName()] = plugin;

    ui->langCombo->addItems(functionPlugins.keys());

    // Syntax highlighting plugins
    foreach (SyntaxHighlighterPlugin* plugin, PLUGINS->getLoadedPlugins<SyntaxHighlighterPlugin>())
        highlighterPlugins[plugin->getLanguageName()] = plugin;

    updateState();
}

QString FunctionsEditor::getCurrentFunctionName() const
{
    QModelIndexList idxList = ui->list->selectionModel()->selectedIndexes();
    if (idxList.size() == 0)
        return QString::null;

    return idxList.first().data().toString();
}

void FunctionsEditor::functionDeselected(const QString& name)
{
    UNUSED(name);
    model->setModified(name, currentModified);
    model->validateNames();
}

void FunctionsEditor::functionSelected(const QString& name)
{
    ui->nameEdit->setText(model->getName(name));
    ui->edit->setPlainText(model->getCode(name));
    ui->undefArgsCheck->setChecked(model->getUndefinedArgs(name));
    ui->langCombo->setCurrentText(model->getLang(name));

    // Arguments
    ui->argsList->clear();
    QListWidgetItem* item;
    foreach (const QString& arg, model->getArguments(name))
    {
        item = new QListWidgetItem(arg);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->argsList->addItem(item);
    }

    // Databases
    dbListModel->setDatabases(model->getDatabases(name));
    ui->databasesList->expandAll();

    if (model->getAllDatabases(name))
        ui->allDatabasesRadio->setChecked(true);
    else
        ui->selDatabasesRadio->setChecked(true);

    // Type
    QString  type = model->getType(name);
    if (type == Config::Function::AGGREGATE_TYPE)
        ui->typeCombo->setCurrentText(tr("Aggregate"));
    else
        ui->typeCombo->setCurrentText(tr("Scalar"));

    currentModified = false;

    updateCurrentFunctionState();
}

void FunctionsEditor::clearEdits()
{
    ui->nameEdit->setText(QString::null);
    ui->edit->setPlainText(QString::null);
    ui->langCombo->setCurrentText(QString::null);
    ui->undefArgsCheck->setChecked(true);
    ui->argsList->clear();
    ui->allDatabasesRadio->setChecked(true);
    ui->typeCombo->setCurrentText(tr("Scalar"));
    ui->langCombo->setCurrentIndex(-1);
}

void FunctionsEditor::selectFunction(const QString& name)
{
    if (!model->getFunctionNames().contains(name))
        return;

    ui->list->selectionModel()->setCurrentIndex(model->indexOf(name), QItemSelectionModel::SelectCurrent);
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

void FunctionsEditor::commit()
{
    QList<Config::Function> configFunctions = model->getConfigFunctions();

    if (CFG->setFunctions(configFunctions))
    {
        model->clearModified();
        currentModified = false;
    }
    updateState();
}

void FunctionsEditor::rollback()
{
    QString selectedBefore = getCurrentFunctionName();

    model->setData(CFG->getFunctions());
    currentModified = false;
    clearEdits();

    if (!selectedBefore.isNull() && model->getFunctionNames().contains(selectedBefore))
        selectFunction(selectedBefore);

    updateState();
}

void FunctionsEditor::newFunction()
{
    if (ui->langCombo->currentIndex() == -1 && ui->langCombo->count() > 0)
        ui->langCombo->setCurrentIndex(0);

    Config::Function func;
    func.name = generateUniqueName("function", model->getFunctionNames());

    if (ui->langCombo->currentIndex() > -1)
        func.lang = ui->langCombo->currentText();

    model->addFunction(func);

    selectFunction(func.name);
}

void FunctionsEditor::deleteFunction()
{
    QString name = getCurrentFunctionName();
    model->deleteFunction(name);
    clearEdits();

    updateState();
}

void FunctionsEditor::updateModified()
{
    QString name = getCurrentFunctionName();
    if (!name.isNull())
    {
        bool nameDiff = name != ui->nameEdit->text();
        bool codeDiff = model->getCode(name) != ui->edit->toPlainText();
        bool langDiff = model->getLang(name) != ui->langCombo->currentText();
        bool undefArgsDiff = model->getUndefinedArgs(name) != ui->undefArgsCheck->isChecked();
        bool allDatabasesDiff = model->getAllDatabases(name) != ui->allDatabasesRadio->isChecked();
        bool argDiff = getCurrentArgList() != model->getArguments(name);
        bool dbDiff = getCurrentDatabases().toSet() != model->getDatabases(name).toSet(); // QSet to ignore order

        QString type = model->getType(name);
        int expectedIndex = (type == Config::Function::AGGREGATE_TYPE) ? 1 : 0;
        bool typeDiff = expectedIndex != ui->typeCombo->currentIndex();

        currentModified = (nameDiff || codeDiff || typeDiff || langDiff || undefArgsDiff || allDatabasesDiff || argDiff || dbDiff);
    }

    updateState();
}

void FunctionsEditor::updateState()
{
    bool modified = model->isModified() || currentModified;

    actionMap[COMMIT]->setEnabled(modified);
    actionMap[ROLLBACK]->setEnabled(modified);
    actionMap[DELETE]->setEnabled(ui->list->selectionModel()->selectedIndexes().size() > 0);

    updateCurrentFunctionState();
}

void FunctionsEditor::updateCurrentFunctionState()
{
    QString name = getCurrentFunctionName();
    bool nameOk = !name.isNull();

    setValidStyle(ui->langLabel, true);

    ui->rightWidget->setEnabled(nameOk);
    if (!nameOk)
        return;

    bool langOk = ui->langCombo->currentIndex() >= 0;
    ui->bottomWidget->setEnabled(langOk);
    ui->argsGroup->setEnabled(langOk);
    ui->databasesGroup->setEnabled(langOk);
    ui->nameEdit->setEnabled(langOk);
    ui->nameLabel->setEnabled(langOk);
    ui->typeCombo->setEnabled(langOk);
    ui->typeLabel->setEnabled(langOk);
    setValidStyle(ui->langLabel, langOk);

    ui->databasesList->setEnabled(ui->selDatabasesRadio->isChecked());

    // Syntax highlighter
    QString lang = ui->langCombo->currentText();
    if (lang != currentHighlighterLang)
    {
        if (currentHighlighter)
        {
            delete currentHighlighter;
            currentHighlighter = nullptr;
        }

        if (langOk && highlighterPlugins.contains(lang))
            currentHighlighter = highlighterPlugins[lang]->createSyntaxHighlighter(ui->edit);

        currentHighlighterLang = lang;
    }

    updateArgsState();
}

void FunctionsEditor::functionSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    int deselCnt = deselected.indexes().size();
    int selCnt = selected.indexes().size();

    if (deselCnt > 0)
        functionDeselected(deselected.indexes().first().data().toString());

    if (selCnt > 0)
        functionSelected(selected.indexes().first().data().toString());

    if (deselCnt > 0 && selCnt == 0)
    {
        currentModified = false;
        clearEdits();
    }
}

void FunctionsEditor::validateName()
{
    bool valid = model->isAllowedName(getCurrentFunctionName(), ui->nameEdit->text());
    setValidStyle(ui->nameEdit, valid);
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

void FunctionsEditor::updateArgsState()
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
}

QVariant FunctionsEditor::saveSession()
{
    return QVariant();
}

FunctionsEditor::DbModel::DbModel(QObject* parent) :
    QSortFilterProxyModel(parent)
{
}

QVariant FunctionsEditor::DbModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::CheckStateRole)
        return QSortFilterProxyModel::data(index, role);

    DbTreeItem* item = getItemForProxyIndex(index);
    if (!item)
        return QSortFilterProxyModel::data(index, role);

    DbTreeItem::Type type = item->getType();
    if (type != DbTreeItem::Type::DB && type != DbTreeItem::Type::INVALID_DB)
        return QSortFilterProxyModel::data(index, role);

    return checkedDatabases.contains(item->text(), Qt::CaseInsensitive) ? Qt::Checked : Qt::Unchecked;
}

bool FunctionsEditor::DbModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::CheckStateRole)
        return QSortFilterProxyModel::setData(index, value, role);

    DbTreeItem* item = getItemForProxyIndex(index);
    if (!item)
        return QSortFilterProxyModel::setData(index, value, role);

    DbTreeItem::Type type = item->getType();
    if (type != DbTreeItem::Type::DB && type != DbTreeItem::Type::INVALID_DB)
        return QSortFilterProxyModel::setData(index, value, role);

    if (value.toBool())
        checkedDatabases << item->text();
    else
        checkedDatabases.removeOne(item->text());

    emit dataChanged(index, index, {Qt::CheckStateRole});

    return true;
}

Qt::ItemFlags FunctionsEditor::DbModel::flags(const QModelIndex& index) const
{
    return QSortFilterProxyModel::flags(index) | Qt::ItemIsUserCheckable;
}

void FunctionsEditor::DbModel::setDatabases(const QStringList& databases)
{
    beginResetModel();
    checkedDatabases = databases;
    endResetModel();
}

QStringList FunctionsEditor::DbModel::getDatabases() const
{
    return checkedDatabases;
}

bool FunctionsEditor::DbModel::filterAcceptsRow(int srcRow, const QModelIndex& srcParent) const
{
    QModelIndex idx = sourceModel()->index(srcRow, 0, srcParent);
    DbTreeItem* item = dynamic_cast<DbTreeItem*>(dynamic_cast<DbTreeModel*>(sourceModel())->itemFromIndex(idx));
    switch (item->getType())
    {
        case DbTreeItem::Type::DIR:
        case DbTreeItem::Type::DB:
        case DbTreeItem::Type::INVALID_DB:
            return true;
        default:
            return false;
    }
    return false;
}

DbTreeItem* FunctionsEditor::DbModel::getItemForProxyIndex(const QModelIndex& index) const
{
    QModelIndex srcIdx = mapToSource(index);
    DbTreeItem* item = dynamic_cast<DbTreeItem*>(dynamic_cast<DbTreeModel*>(sourceModel())->itemFromIndex(srcIdx));
    return item;
}
