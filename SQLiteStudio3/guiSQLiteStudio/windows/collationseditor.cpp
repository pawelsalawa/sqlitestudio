#include "collationseditor.h"
#include "ui_collationseditor.h"
#include "common/unused.h"
#include "common/compatibility.h"
#include "selectabledbmodel.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include "collationseditormodel.h"
#include "common/utils.h"
#include "uiutils.h"
#include "services/pluginmanager.h"
#include "syntaxhighlighterplugin.h"
#include "plugins/scriptingplugin.h"
#include "uiconfig.h"
#include <QDesktopServices>
#include <QSyntaxHighlighter>

CFG_KEYS_DEFINE(CollationsEditor)

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
    createAction(COMMIT, ICONS.COMMIT, tr("Commit all collation changes"), this, SLOT(commit()), ui->toolbar, this);
    createAction(ROLLBACK, ICONS.ROLLBACK, tr("Rollback all collation changes"), this, SLOT(rollback()), ui->toolbar, this);
    ui->toolbar->addSeparator();
    createAction(ADD, ICONS.NEW_COLLATION, tr("Create new collation"), this, SLOT(newCollation()), ui->toolbar, this);
    createAction(DELETE, ICONS.DELETE_COLLATION, tr("Delete selected collation"), this, SLOT(deleteCollation()), ui->toolbar, this);
    ui->toolbar->addSeparator();
    createAction(HELP, ICONS.HELP, tr("Editing collations manual"), this, SLOT(help()), ui->toolbar, this);
}

void CollationsEditor::setupDefShortcuts()
{
    // Widget context
    setShortcutContext({COMMIT, ROLLBACK}, Qt::WidgetWithChildrenShortcut);
    BIND_SHORTCUTS(CollationsEditor, Action);
}

QToolBar* CollationsEditor::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return ui->toolbar;
}

void CollationsEditor::init()
{
    ui->setupUi(this);
    initActions();

    setFont(CFG_UI.Fonts.SqlEditor.get());

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
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(updateModified()));
    connect(ui->allDatabasesRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->selectedDatabasesRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->functionBasedRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->extensionBasedRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->langCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateModified()));

    connect(dbListModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateModified()));
    connect(CFG_UI.Fonts.SqlEditor, SIGNAL(changed(QVariant)), this, SLOT(changeFont(QVariant)));

    updateLangCombo();

    // Syntax highlighting plugins
    for (SyntaxHighlighterPlugin* plugin : PLUGINS->getLoadedPlugins<SyntaxHighlighterPlugin>())
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

CollationManager::CollationType CollationsEditor::getCurrentType() const
{
    return ui->extensionBasedRadio->isChecked() ? CollationManager::CollationType::EXTENSION_BASED
                                                : CollationManager::CollationType::FUNCTION_BASED;
}

void CollationsEditor::collationDeselected(int row)
{
    model->setName(row, ui->nameEdit->text());
    model->setType(row, getCurrentType());
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
    updatesForSelection = true;
    ui->nameEdit->setText(model->getName(row));
    ui->codeEdit->setPlainText(model->getCode(row));
    ui->functionBasedRadio->setChecked(model->getType(row) == CollationManager::CollationType::FUNCTION_BASED);
    ui->extensionBasedRadio->setChecked(model->getType(row) == CollationManager::CollationType::EXTENSION_BASED);
    updateLangCombo();
    ui->langCombo->setCurrentText(model->getLang(row));

    // Databases
    dbListModel->setDatabases(model->getDatabases(row));
    ui->databaseList->expandAll();

    if (model->getAllDatabases(row))
        ui->allDatabasesRadio->setChecked(true);
    else
        ui->selectedDatabasesRadio->setChecked(true);

    updatesForSelection = false;
    currentModified = model->isModified(row);

    updateCurrentCollationState();
}

void CollationsEditor::clearEdits()
{
    ui->nameEdit->setText(QString());
    ui->codeEdit->setPlainText(QString());
    ui->langCombo->setCurrentText(QString());
    ui->allDatabasesRadio->setChecked(true);
    ui->langCombo->setCurrentIndex(-1);
}

void CollationsEditor::selectCollation(int row)
{
    if (!model->isValidRowIndex(row))
        return;

    ui->collationList->selectionModel()->setCurrentIndex(model->index(row), QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
}

QStringList CollationsEditor::getCurrentDatabases() const
{
    return dbListModel->getDatabases();
}

void CollationsEditor::setFont(const QFont& font)
{
    ui->codeEdit->setFont(font);
}

void CollationsEditor::help()
{
    static const QString url = QStringLiteral("https://github.com/pawelsalawa/sqlitestudio/wiki/User_Manual#custom-collations");
    QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode));
}

void CollationsEditor::commit()
{
    int row = getCurrentCollationRow();
    if (model->isValidRowIndex(row))
        collationDeselected(row);

    QList<CollationManager::CollationPtr> collations = model->getCollations();

    COLLATIONS->setCollations(collations);
    model->clearModified();
    currentModified = false;

    if (model->isValidRowIndex(row))
        selectCollation(row);

    updateState();
}

void CollationsEditor::rollback()
{
    int selectedBefore = getCurrentCollationRow();

    model->setData(COLLATIONS->getAllCollations());
    currentModified = false;
    clearEdits();

    if (model->isValidRowIndex(selectedBefore))
        selectCollation(selectedBefore);

    updateState();
}

void CollationsEditor::newCollation()
{
    if (ui->langCombo->currentIndex() == -1 && ui->langCombo->count() > 0)
        ui->langCombo->setCurrentIndex(0);

    CollationManager::CollationPtr coll = CollationManager::CollationPtr::create();
    coll->name = generateUniqueName("collation", model->getCollationNames());
    coll->type = getCurrentType();
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
    if (model->isValidRowIndex(row))
        collationSelected(row);

    updateState();
}

void CollationsEditor::updateState()
{
    bool modified = model->isModified() || currentModified;
    bool valid = model->isValid();

    actionMap[COMMIT]->setEnabled(modified && valid);
    actionMap[ROLLBACK]->setEnabled(modified);
    actionMap[DELETE]->setEnabled(ui->collationList->selectionModel()->selectedIndexes().size() > 0);
}

void CollationsEditor::updateCurrentCollationState()
{
    int row = getCurrentCollationRow();
    bool validRow = model->isValidRowIndex(row);
    ui->rightWidget->setEnabled(validRow);
    if (!validRow)
    {
        setValidState(ui->langCombo, true);
        setValidState(ui->nameEdit, true);
        setValidState(ui->codeEdit, true);
        return;
    }

    QString name = ui->nameEdit->text();
    bool nameOk = model->isAllowedName(row, name) && !name.trimmed().isEmpty();
    setValidState(ui->nameEdit, nameOk, tr("Enter a non-empty, unique name of the collation."));

    updateLangCombo();

    bool langOk = ui->langCombo->currentIndex() >= 0;
    ui->codeGroup->setEnabled(langOk);
    ui->databasesGroup->setEnabled(langOk);
    ui->nameEdit->setEnabled(langOk);
    ui->nameLabel->setEnabled(langOk);
    ui->databaseList->setEnabled(ui->selectedDatabasesRadio->isChecked());
    setValidState(ui->langCombo, langOk, tr("Pick the implementation language."));

    bool codeOk = !ui->codeEdit->toPlainText().trimmed().isEmpty();
    if (ui->extensionBasedRadio->isChecked())
    {
        ui->codeGroup->setTitle(tr("Registration code"));
        setValidState(ui->codeEdit, codeOk, tr("Enter a non-empty registration code."));
    }
    else
    {
        ui->codeGroup->setTitle(tr("Implementation code"));
        setValidState(ui->codeEdit, codeOk, tr("Enter a non-empty implementation code."));
    }

    // Syntax highlighter
    QString lang = ui->langCombo->currentText();
    if (lang != currentHighlighterLang)
    {
        QSyntaxHighlighter* highlighter = nullptr;
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
    model->setValid(row, langOk && codeOk && nameOk);
    updateState();
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

void CollationsEditor::updateLangCombo()
{
    QComboBox *combo = ui->langCombo;
    bool alreadyInternalUpdate = updatesForSelection;
    updatesForSelection = true;
    if (ui->extensionBasedRadio->isChecked())
    {
        if (combo->isEnabled())
        {
            combo->setEnabled(false);
            combo->clear();
            combo->addItem("SQL");
            combo->setCurrentIndex(0);
        }
    }
    else
    {
        if (!combo->isEnabled())
        {
            combo->clear();
            for (ScriptingPlugin* plugin : PLUGINS->getLoadedPlugins<ScriptingPlugin>())
                combo->addItem(plugin->getLanguage());
            combo->setEnabled(true);
        }
    }
    updatesForSelection = alreadyInternalUpdate;
}

void CollationsEditor::updateModified()
{
    if (updatesForSelection)
        return;

    int row = getCurrentCollationRow();
    if (model->isValidRowIndex(row))
    {
        bool nameDiff = model->getName(row) != ui->nameEdit->text();
        bool codeDiff = model->getCode(row) != ui->codeEdit->toPlainText();
        bool typeDiff = model->getType(row) != getCurrentType();
        bool langDiff = model->getLang(row) != ui->langCombo->currentText();
        bool allDatabasesDiff = model->getAllDatabases(row) != ui->allDatabasesRadio->isChecked();
        bool dbDiff = toSet(getCurrentDatabases()) != toSet(model->getDatabases(row)); // QSet to ignore order

        currentModified = (nameDiff || codeDiff || typeDiff || langDiff || allDatabasesDiff || dbDiff);
    }

    updateCurrentCollationState();
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

void CollationsEditor::changeFont(const QVariant& font)
{
    setFont(font.value<QFont>());
}

bool CollationsEditor::isUncommitted() const
{
    return model->isModified() || currentModified;
}

QString CollationsEditor::getQuitUncommittedConfirmMessage() const
{
    return tr("Collations editor window has uncommitted modifications.");
}
