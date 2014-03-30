#include "collationseditor.h"
#include "ui_collationseditor.h"
#include "common/unused.h"
#include "selectabledbmodel.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include "collationseditormodel.h"
#include "common/utils.h"
#include "uiutils.h"
#include "services/pluginmanager.h"
#include "syntaxhighlighterplugin.h"
#include "plugins/scriptingplugin.h"
#include <QDesktopServices>
#include <QSyntaxHighlighter>

CollationsEditor::CollationsEditor(QWidget *parent) :
    MdiChild(parent),
    ui(new Ui::CollationsEditor)
{
    init();
}

CollationsEditor::~CollationsEditor()
{
    delete ui;
}

bool CollationsEditor::restoreSessionNextTime()
{
    return false;
}

QVariant CollationsEditor::saveSession()
{
    return QVariant();
}

bool CollationsEditor::restoreSession(const QVariant& sessionValue)
{
    UNUSED(sessionValue);
    return true;
}

Icon* CollationsEditor::getIconNameForMdiWindow()
{
    return ICONS.CONSTRAINT_COLLATION;
}

QString CollationsEditor::getTitleForMdiWindow()
{
    return tr("Collations editor");
}

void CollationsEditor::createActions()
{
    createAction(COMMIT, ICONS.COMMIT, tr("Commit all collation changes"), this, SLOT(commit()), ui->toolbar);
    createAction(ROLLBACK, ICONS.ROLLBACK, tr("Rollback all collation changes"), this, SLOT(rollback()), ui->toolbar);
    ui->toolbar->addSeparator();
    createAction(ADD, ICONS.NEW_COLLATION, tr("Create new collation"), this, SLOT(newCollation()), ui->toolbar);
    createAction(DELETE, ICONS.DELETE_COLLATION, tr("Delete selected collation"), this, SLOT(deleteCollation()), ui->toolbar);
    ui->toolbar->addSeparator();
    createAction(HELP, ICONS.HELP, tr("Editing collations manual"), this, SLOT(help()), ui->toolbar);
}

void CollationsEditor::setupDefShortcuts()
{

}

void CollationsEditor::init()
{
    ui->setupUi(this);
    initActions();

    model = new CollationsEditorModel(this);
    collationFilterModel = new QSortFilterProxyModel(this);
    collationFilterModel->setSourceModel(model);
    ui->collationList->setModel(collationFilterModel);

    dbListModel = new SelectableDbModel(this);
    dbListModel->setDisabledVersion(2);
    dbListModel->setSourceModel(DBTREE->getModel());
    ui->databaseList->setModel(dbListModel);
    ui->databaseList->expandAll();

    model->setData(COLLATIONS->getAllCollations());

    connect(ui->collationList->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(collationSelected(QItemSelection,QItemSelection)));
    connect(ui->collationList->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateState()));
    connect(ui->codeEdit, SIGNAL(textChanged()), this, SLOT(updateModified()));
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(validateName()));
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(updateModified()));
    connect(ui->allDatabasesRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->selectedDatabasesRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->langCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateModified()));

    connect(dbListModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateModified()));

    // Language plugins
    foreach (ScriptingPlugin* plugin, PLUGINS->getLoadedPlugins<ScriptingPlugin>())
        ui->langCombo->addItem(plugin->getLanguage());

    // Syntax highlighting plugins
    foreach (SyntaxHighlighterPlugin* plugin, PLUGINS->getLoadedPlugins<SyntaxHighlighterPlugin>())
        highlighterPlugins[plugin->getLanguageName()] = plugin;

    updateState();
}

int CollationsEditor::getCurrentCollationRow() const
{
    QModelIndexList idxList = ui->collationList->selectionModel()->selectedIndexes();
    if (idxList.size() == 0)
        return -1;

    return idxList.first().row();
}

void CollationsEditor::collationDeselected(int row)
{
    model->setName(row, ui->nameEdit->text());
    model->setLang(row, ui->langCombo->currentText());
    model->setAllDatabases(row, ui->allDatabasesRadio->isChecked());
    model->setCode(row, ui->codeEdit->toPlainText());
    model->setModified(row, currentModified);

    if (ui->selectedDatabasesRadio->isChecked())
        model->setDatabases(row, getCurrentDatabases());

    model->validateNames();
}

void CollationsEditor::collationSelected(int row)
{
    ui->nameEdit->setText(model->getName(row));
    ui->codeEdit->setPlainText(model->getCode(row));
    ui->langCombo->setCurrentText(model->getLang(row));

    // Databases
    dbListModel->setDatabases(model->getDatabases(row));
    ui->databaseList->expandAll();

    if (model->getAllDatabases(row))
        ui->allDatabasesRadio->setChecked(true);
    else
        ui->selectedDatabasesRadio->setChecked(true);

    currentModified = false;

    updateCurrentCollationState();
}

void CollationsEditor::clearEdits()
{
    ui->nameEdit->setText(QString::null);
    ui->codeEdit->setPlainText(QString::null);
    ui->langCombo->setCurrentText(QString::null);
    ui->allDatabasesRadio->setChecked(true);
    ui->langCombo->setCurrentIndex(-1);
}

void CollationsEditor::selectCollation(int row)
{
    if (!model->isValidRow(row))
        return;

    ui->collationList->selectionModel()->setCurrentIndex(model->index(row), QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
}

QStringList CollationsEditor::getCurrentDatabases() const
{
    return dbListModel->getDatabases();
}

void CollationsEditor::help()
{
    static const QString url = QStringLiteral("http://sqlitestudio.pl/wiki/index.php/User_Manual#Custom_collations");
    QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode));
}

void CollationsEditor::commit()
{
    int row = getCurrentCollationRow();
    if (model->isValidRow(row))
        collationDeselected(row);

    QList<CollationManager::CollationPtr> collations = model->getCollations();

    COLLATIONS->setCollations(collations);
    model->clearModified();
    currentModified = false;

    if (model->isValidRow(row))
        selectCollation(row);

    updateState();
}

void CollationsEditor::rollback()
{
    int selectedBefore = getCurrentCollationRow();

    model->setData(COLLATIONS->getAllCollations());
    currentModified = false;
    clearEdits();

    if (model->isValidRow(selectedBefore))
        selectCollation(selectedBefore);

    updateState();
}

void CollationsEditor::newCollation()
{
    if (ui->langCombo->currentIndex() == -1 && ui->langCombo->count() > 0)
        ui->langCombo->setCurrentIndex(0);

    CollationManager::CollationPtr coll = CollationManager::CollationPtr::create();
    coll->name = generateUniqueName("collation", model->getCollationNames());

    if (ui->langCombo->currentIndex() > -1)
        coll->lang = ui->langCombo->currentText();

    model->addCollation(coll);

    selectCollation(model->rowCount() - 1);
}

void CollationsEditor::deleteCollation()
{
    int row = getCurrentCollationRow();
    model->deleteCollation(row);
    clearEdits();

    row = getCurrentCollationRow();
    if (model->isValidRow(row))
        collationSelected(row);

    updateState();
}

void CollationsEditor::updateState()
{
    bool modified = model->isModified() || currentModified;

    actionMap[COMMIT]->setEnabled(modified);
    actionMap[ROLLBACK]->setEnabled(modified);
    actionMap[DELETE]->setEnabled(ui->collationList->selectionModel()->selectedIndexes().size() > 0);

    updateCurrentCollationState();
}

void CollationsEditor::updateCurrentCollationState()
{
    int row = getCurrentCollationRow();

    QString name = model->getName(row);
    bool nameOk = !name.isNull();

    setValidStyle(ui->langLabel, true);

    ui->rightWidget->setEnabled(nameOk);
    if (!nameOk)
        return;

    bool langOk = ui->langCombo->currentIndex() >= 0;
    ui->codeGroup->setEnabled(langOk);
    ui->databasesGroup->setEnabled(langOk);
    ui->nameEdit->setEnabled(langOk);
    ui->nameLabel->setEnabled(langOk);
    ui->databaseList->setEnabled(ui->selectedDatabasesRadio->isChecked());
    setValidStyle(ui->langLabel, langOk);

    // Syntax highlighter
    QString lang = ui->langCombo->currentText();
    if (lang != currentHighlighterLang)
    {
        QSyntaxHighlighter* highlighter;
        if (currentHighlighter)
        {
            // A pointers swap with local var - this is necessary, cause deleting highlighter
            // triggers textChanged on QPlainTextEdit, which then calls this method,
            // so it becomes an infinite recursion with deleting the same pointer.
            // We set the pointer to null first, then delete it. That way it's safe.
            highlighter = currentHighlighter;
            currentHighlighter = nullptr;
            delete highlighter;
        }

        if (langOk && highlighterPlugins.contains(lang))
        {
            currentHighlighter = highlighterPlugins[lang]->createSyntaxHighlighter(ui->codeEdit);
        }

        currentHighlighterLang = lang;
    }
}

void CollationsEditor::collationSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    int deselCnt = deselected.indexes().size();
    int selCnt = selected.indexes().size();

    if (deselCnt > 0)
        collationDeselected(deselected.indexes().first().row());

    if (selCnt > 0)
        collationSelected(selected.indexes().first().row());

    if (deselCnt > 0 && selCnt == 0)
    {
        currentModified = false;
        clearEdits();
    }
}

void CollationsEditor::validateName()
{
    bool valid = model->isAllowedName(getCurrentCollationRow(), ui->nameEdit->text());
    setValidStyle(ui->nameEdit, valid);
}

void CollationsEditor::updateModified()
{
    int row = getCurrentCollationRow();
    if (model->isValidRow(row))
    {
        bool nameDiff = model->getName(row) != ui->nameEdit->text();
        bool codeDiff = model->getCode(row) != ui->codeEdit->toPlainText();
        bool langDiff = model->getLang(row) != ui->langCombo->currentText();
        bool allDatabasesDiff = model->getAllDatabases(row) != ui->allDatabasesRadio->isChecked();
        bool dbDiff = getCurrentDatabases().toSet() != model->getDatabases(row).toSet(); // QSet to ignore order

        currentModified = (nameDiff || codeDiff || langDiff || allDatabasesDiff || dbDiff);
    }

    updateState();
}

void CollationsEditor::applyFilter(const QString& value)
{
    //
    // See FunctionsEditor::applyFilter() for details why we remember current selection and restore it at the end.
    //

    int row = getCurrentCollationRow();
    ui->collationList->selectionModel()->clearSelection();

    collationFilterModel->setFilterFixedString(value);

    selectCollation(row);
}
