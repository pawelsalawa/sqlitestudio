#include "codesnippeteditor.h"
#include "common/unused.h"
#include "ui_codesnippeteditor.h"
#include "uiconfig.h"
#include "windows/codesnippeteditormodel.h"
#include "sqlitestudio.h"
#include "iconmanager.h"
#include "uiutils.h"
#include "codesnippeteditormodel.h"
#include <QSortFilterProxyModel>
#include <QItemSelection>
#include <QDesktopServices>
#include <QStyleFactory>

CFG_KEYS_DEFINE(CodeSnippetEditor)

CodeSnippetEditor::CodeSnippetEditor(QWidget *parent) :
    MdiChild(parent),
    ui(new Ui::CodeSnippetEditor)
{
    init();
}

CodeSnippetEditor::~CodeSnippetEditor()
{
    delete ui;
}

bool CodeSnippetEditor::restoreSessionNextTime()
{
    return false;
}

bool CodeSnippetEditor::isUncommitted() const
{
    return model->isModified() || currentModified;
}

QString CodeSnippetEditor::getQuitUncommittedConfirmMessage() const
{
    return tr("Code Snippets editor window has uncommitted modifications.");
}

QVariant CodeSnippetEditor::saveSession()
{
    return QVariant();
}

bool CodeSnippetEditor::restoreSession(const QVariant& sessionValue)
{
    UNUSED(sessionValue);
    return true;
}

Icon* CodeSnippetEditor::getIconNameForMdiWindow()
{
    return ICONS.CODE_SNIPPETS;
}

QString CodeSnippetEditor::getTitleForMdiWindow()
{
    return tr("Code Snippets editor");
}

void CodeSnippetEditor::createActions()
{
    createAction(COMMIT, ICONS.COMMIT, tr("Commit all snippet changes"), this, SLOT(commit()), ui->toolBar, this);
    createAction(ROLLBACK, ICONS.ROLLBACK, tr("Rollback all snippet changes"), this, SLOT(rollback()), ui->toolBar, this);
    ui->toolBar->addSeparator();
    createAction(ADD, ICONS.NEW_FUNCTION, tr("Create new snippet"), this, SLOT(newSnippet()), ui->toolBar, this);
    createAction(DELETE, ICONS.DELETE_FUNCTION, tr("Delete selected snippet"), this, SLOT(deleteSnippet()), ui->toolBar, this);
    ui->toolBar->addSeparator();
    createAction(MOVE_UP, ICONS.MOVE_UP, tr("Move the snippet up"), this, SLOT(moveSnippetUp()), ui->toolBar, this);
    createAction(MOVE_DOWN, ICONS.MOVE_DOWN, tr("Move the snippet down"), this, SLOT(moveSnippetDown()), ui->toolBar, this);
    ui->toolBar->addSeparator();
    createAction(HELP, ICONS.HELP, tr("Code snippets manual"), this, SLOT(help()), ui->toolBar, this);

#ifdef Q_OS_MACX
    QStyle *fusion = QStyleFactory::create("Fusion");
    ui->toolBar->setStyle(fusion);
#endif
}

void CodeSnippetEditor::setupDefShortcuts()
{
    // Widget context
    setShortcutContext({COMMIT, ROLLBACK}, Qt::WidgetWithChildrenShortcut);
    BIND_SHORTCUTS(CodeSnippetEditor, Action);
}

QToolBar* CodeSnippetEditor::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return ui->toolBar;
}

void CodeSnippetEditor::init()
{
    ui->setupUi(this);
    clearEdits();

    ui->mainCodeEdit->setFont(CFG_UI.Fonts.SqlEditor.get());

    model = new CodeSnippetEditorModel(this);
    snippetFilterModel = new QSortFilterProxyModel(this);
    snippetFilterModel->setSourceModel(model);
    ui->list->setModel(snippetFilterModel);

    initActions();

    connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(snippetSelected(QItemSelection,QItemSelection)));
    connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateState()));
    connect(ui->assistantShortcutEdit, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(updateModified()));
    connect(ui->mainCodeEdit, SIGNAL(textChanged()), this, SLOT(updateModified()));
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(updateModified()));
    connect(CFG_UI.Fonts.SqlEditor, SIGNAL(changed(QVariant)), this, SLOT(changeFont(QVariant)));
    connect(ui->clearAssistantShortcutButton, SIGNAL(clicked(bool)), this, SLOT(clearAssistantShortcutPressed()));

    model->setData(CODESNIPPETS->getSnippets());

    updateCurrentSnippetState();
}

int CodeSnippetEditor::getCurrentSnippetRow() const
{
    QModelIndexList idxList = ui->list->selectionModel()->selectedIndexes();
    if (idxList.size() == 0)
        return -1;

    return idxList.first().row();
}

void CodeSnippetEditor::snippetDeselected(int row)
{
    model->setName(row, ui->nameEdit->text());
    model->setCode(row, ui->mainCodeEdit->toPlainText());
    model->setHotkey(row, ui->assistantShortcutEdit->keySequence());
    model->setModified(row, currentModified);
    model->validateNames();
}

void CodeSnippetEditor::snippetSelected(int row)
{
    updatesForSelection = true;
    ui->nameEdit->setText(model->getName(row));
    ui->mainCodeEdit->setPlainText(model->getCode(row));
    ui->assistantShortcutEdit->setKeySequence(model->getHotkey(row));

    updatesForSelection = false;
    currentModified = model->isModified(row);

    updateCurrentSnippetState();
}

void CodeSnippetEditor::selectSnippet(int row, bool skipModelUpdates)
{
    if (!model->isValidRowIndex(row))
        return;

    if (skipModelUpdates)
        skipModelUpdatesDuringSelection = true;

    ui->list->selectionModel()->setCurrentIndex(model->index(row), QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);

    if (skipModelUpdates)
        skipModelUpdatesDuringSelection = false;
}

void CodeSnippetEditor::clearEdits()
{
    ui->nameEdit->clear();
    ui->assistantShortcutEdit->clear();
    ui->mainCodeEdit->setPlainText(QString());
}

void CodeSnippetEditor::commit()
{
    int row = getCurrentSnippetRow();
    if (model->isValidRowIndex(row))
        snippetDeselected(row);

    QList<CodeSnippetManager::CodeSnippet*> snippets = model->generateSnippets();

    CODESNIPPETS->setSnippets(snippets);
    model->clearModified();
    currentModified = false;

    if (model->isValidRowIndex(row))
        selectSnippet(row);

    updateState();
}

void CodeSnippetEditor::rollback()
{
    int selectedBefore = getCurrentSnippetRow();

    model->setData(CODESNIPPETS->getSnippets());
    currentModified = false;
    clearEdits();

    if (model->isValidRowIndex(selectedBefore))
        selectSnippet(selectedBefore);

    updateState();
}

void CodeSnippetEditor::newSnippet()
{
    CodeSnippetManager::CodeSnippet* snip = new CodeSnippetManager::CodeSnippet();
    snip->name = generateUniqueName("snippet", model->getSnippetNames());

    model->addSnippet(snip);

    selectSnippet(model->rowCount() - 1);
}

void CodeSnippetEditor::deleteSnippet()
{
    int row = getCurrentSnippetRow();
    model->deleteSnippet(row);
    clearEdits();

    row = getCurrentSnippetRow();
    if (model->isValidRowIndex(row))
        snippetSelected(row);

    updateState();
}

void CodeSnippetEditor::moveSnippetUp()
{
    int row = getCurrentSnippetRow();
    int newRow = model->moveUp(row);
    if (row != newRow)
        selectSnippet(newRow, true);
}

void CodeSnippetEditor::moveSnippetDown()
{
    int row = getCurrentSnippetRow();
    int newRow = model->moveDown(row);
    if (row != newRow)
        selectSnippet(newRow, true);
}

void CodeSnippetEditor::updateModified()
{
    if (updatesForSelection)
        return;

    int row = getCurrentSnippetRow();
    if (model->isValidRowIndex(row))
    {
        bool nameDiff = model->getName(row) != ui->nameEdit->text();
        bool codeDiff = model->getCode(row) != ui->mainCodeEdit->toPlainText();
        bool hotkeyDiff = model->getHotkey(row) != ui->assistantShortcutEdit->keySequence();
        currentModified = (nameDiff || codeDiff || hotkeyDiff);
    }

    updateCurrentSnippetState();
}

void CodeSnippetEditor::updateCurrentSnippetState()
{
    int row = getCurrentSnippetRow();
    bool validRow = model->isValidRowIndex(row);
    ui->rightWidget->setEnabled(validRow);
    if (!validRow)
    {
        setValidState(ui->nameEdit, true);
        setValidState(ui->mainCodeEdit, true);
        setValidState(ui->assistantShortcutEdit, true);
        updateState();
        return;
    }

    QString name = ui->nameEdit->text();
    bool nameOk = !name.trimmed().isEmpty() && model->isAllowedName(row, name);
    setValidState(ui->nameEdit, nameOk, tr("Enter a non-empty, unique name of the snippet."));

    bool codeOk = !ui->mainCodeEdit->toPlainText().trimmed().isEmpty();
    setValidState(ui->mainCodeEdit, codeOk, tr("Enter a non-empty snippet content."));

    QKeySequence assistantHotkey = ui->assistantShortcutEdit->keySequence();
    bool hotkeyOk = assistantHotkey.isEmpty() || model->isAllowedHotkey(row, assistantHotkey);
    setValidState(ui->assistantShortcutWidget, hotkeyOk, tr("This hotkey is not unique in context of a code assistant."));

    model->setValid(row, codeOk && nameOk && hotkeyOk);
    updateState();
}

void CodeSnippetEditor::updateState()
{
    bool modified = model->isModified() || currentModified;
    bool valid = model->isValid();

    actionMap[COMMIT]->setEnabled(modified && valid);
    actionMap[ROLLBACK]->setEnabled(modified);
    actionMap[DELETE]->setEnabled(ui->list->selectionModel()->selectedIndexes().size() > 0);
}

void CodeSnippetEditor::snippetSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (skipModelUpdatesDuringSelection)
        return;

    int deselCnt = deselected.indexes().size();
    int selCnt = selected.indexes().size();

    if (deselCnt > 0)
        snippetDeselected(deselected.indexes().first().row());

    if (selCnt > 0)
        snippetSelected(selected.indexes().first().row());

    if (deselCnt > 0 && selCnt == 0)
    {
        currentModified = false;
        clearEdits();
    }
}

void CodeSnippetEditor::applyFilter(const QString& value)
{
    // The selection hack (clear & set) below is described in more details in FunctionsEditor::applyFilter().
    int row = getCurrentSnippetRow();
    ui->list->selectionModel()->clearSelection();

    snippetFilterModel->setFilterFixedString(value);

    selectSnippet(row);
}

void CodeSnippetEditor::changeFont(const QVariant& font)
{
    ui->mainCodeEdit->setFont(font.value<QFont>());
}

void CodeSnippetEditor::clearAssistantShortcutPressed()
{
    ui->assistantShortcutEdit->clear();
    ui->assistantShortcutEdit->setFocus();
}

void CodeSnippetEditor::help()
{
    static const QString url = QStringLiteral("https://github.com/pawelsalawa/sqlitestudio/wiki/User_Manual#code-snippets");
    QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode));
}
