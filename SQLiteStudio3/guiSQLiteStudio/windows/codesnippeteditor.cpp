#include "codesnippeteditor.h"
#include "common/utils.h"
#include "dialogs/settingsexportdialog.h"
#include "dialogs/settingsimportdialog.h"
#include "ui_codesnippeteditor.h"
#include "uiconfig.h"
#include "windows/codesnippeteditormodel.h"
#include "sqlitestudio.h"
#include "iconmanager.h"
#include "uiutils.h"
#include "codesnippeteditormodel.h"
#include "common/userinputfilter.h"
#include "mainwindow.h"
#include <QSortFilterProxyModel>
#include <QItemSelection>
#include <QDesktopServices>
#include <QStyleFactory>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtSystemDetection>
#else
#include <qsystemdetection.h>
#endif

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
    return dataModel->isModified() || currentModified;
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
    Q_UNUSED(sessionValue);
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
    createAction(IMPORT, ICONS.CODE_SNIPPETS_IMPORT, tr("Import snippets from file"), this, SLOT(importSnippets()), ui->toolBar, this);
    createAction(EXPORT, ICONS.CODE_SNIPPETS_EXPORT, tr("Export snippets to file"), this, SLOT(exportSnippets()), ui->toolBar, this);
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
    Q_UNUSED(toolbar);
    return ui->toolBar;
}

void CodeSnippetEditor::init()
{
    ui->setupUi(this);
    ui->mainCodeEdit->setAutoCompletion(false);
    ui->mainCodeEdit->setErrorsCheckingEnabled(false);
    clearEdits();

    ui->mainCodeEdit->setFont(CFG_UI.Fonts.SqlEditor.get());

    dataModel = new CodeSnippetEditorModel(this);
    viewModel = new QSortFilterProxyModel(this);
    viewModel->setSourceModel(dataModel);
    ui->list->setModel(viewModel);
    ui->list->horizontalHeader()->setMinimumSectionSize(20);
    ui->list->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->splitter->setSizes({1, 1});
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);
    Cfg::handleSplitterState(ui->splitter);

    new UserInputFilter(ui->snippetFilterEdit, this, SLOT(applyFilter(QString)));
    viewModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    MAINWINDOW->installToolbarSizeWheelHandler(ui->toolBar);

    initActions();
    setupContextMenu();

    connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(snippetSelected(QItemSelection,QItemSelection)));
    connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateState()));
    connect(ui->assistantShortcutEdit, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(updateModified()));
    connect(ui->mainCodeEdit, SIGNAL(textChanged()), this, SLOT(updateModified()));
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(updateModified()));
    connect(CFG_UI.Fonts.SqlEditor, SIGNAL(changed(QVariant)), this, SLOT(changeFont(QVariant)));
    connect(ui->clearAssistantShortcutButton, SIGNAL(clicked(bool)), this, SLOT(clearAssistantShortcutPressed()));

    dataModel->setSnippets(CODESNIPPETS->getSnippets());
    connect(CODESNIPPETS, SIGNAL(codeSnippetListChanged()), this, SLOT(cfgCodeSnippetListChanged()));
    ui->list->resizeColumnsToContents();

    updateCurrentSnippetState();
}

void CodeSnippetEditor::setupContextMenu()
{
    addFormatSqlToContextMenu(ui->mainCodeEdit);
}

QModelIndex CodeSnippetEditor::getCurrentSnippetIndex() const
{
    QModelIndexList idxList = ui->list->selectionModel()->selectedIndexes();
    if (idxList.size() == 0)
        return QModelIndex();

    return idxList.first();
}

void CodeSnippetEditor::snippetDeselected(const QModelIndex& idx)
{
    viewModel->setData(idx, ui->nameEdit->text(), Qt::DisplayRole);
    viewModel->setData(idx, ui->mainCodeEdit->toPlainText(), CodeSnippetEditorModel::CODE);
    viewModel->setData(idx, ui->assistantShortcutEdit->keySequence().toString(QKeySequence::NativeText), CodeSnippetEditorModel::HOTKEY);
    viewModel->setData(idx, currentModified, CodeSnippetEditorModel::MODIFIED);
    dataModel->validateNames();
}

void CodeSnippetEditor::snippetSelected(const QModelIndex& idx)
{
    updatesForSelection = true;
    ui->nameEdit->setText(viewModel->data(idx, Qt::DisplayRole).toString());
    ui->mainCodeEdit->setPlainText(viewModel->data(idx, CodeSnippetEditorModel::CODE).toString());
    ui->assistantShortcutEdit->setKeySequence(viewModel->data(idx, CodeSnippetEditorModel::HOTKEY).toString());

    updatesForSelection = false;
    currentModified = idx.data(CodeSnippetEditorModel::MODIFIED).toBool();

    updateCurrentSnippetState();

    ui->mainCodeEdit->setFocus(Qt::OtherFocusReason);
}

void CodeSnippetEditor::selectSnippet(const QModelIndex& idx, bool skipModelUpdates)
{
    if (!idx.isValid())
        return;

    if (skipModelUpdates)
        skipModelUpdatesDuringSelection = true;

    ui->list->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);

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
    QModelIndex idx = getCurrentSnippetIndex();
    if (idx.isValid())
        snippetDeselected(idx);

    QList<CodeSnippetManager::CodeSnippet*> snippets = dataModel->generateSnippets();

    CODESNIPPETS->setSnippets(snippets);
    dataModel->clearModified();
    currentModified = false;

    idx = viewModel->index(idx.row(), idx.column());
    if (idx.isValid())
        selectSnippet(idx);

    updateState();
    ui->list->resizeColumnsToContents();
}

void CodeSnippetEditor::rollback()
{
    QModelIndex idx = getCurrentSnippetIndex();

    dataModel->setSnippets(CODESNIPPETS->getSnippets());
    currentModified = false;
    clearEdits();

    idx = viewModel->index(idx.row(), idx.column());
    if (idx.isValid())
        selectSnippet(idx);

    updateState();
}

void CodeSnippetEditor::newSnippet()
{
    CodeSnippetManager::CodeSnippet* snip = new CodeSnippetManager::CodeSnippet();
    snip->name = generateUniqueName("snippet", dataModel->getSnippetNames());

    dataModel->addSnippet(snip);

    QModelIndex idx = viewModel->index(viewModel->rowCount() - 1, 0);
    selectSnippet(idx);
}

void CodeSnippetEditor::deleteSnippet()
{
    QModelIndex idx = getCurrentSnippetIndex();
    dataModel->deleteSnippet(viewModel->mapToSource(idx));
    clearEdits();

    idx = getCurrentSnippetIndex();
    if (idx.isValid())
        snippetSelected(idx);

    updateState();
}

void CodeSnippetEditor::moveSnippetUp()
{
    QModelIndex idx = getCurrentSnippetIndex();
    QModelIndex newIdx = viewModel->mapFromSource(dataModel->moveUp(viewModel->mapToSource(idx)));
    if (idx != newIdx)
        selectSnippet(newIdx, true);

    updateState();
}

void CodeSnippetEditor::moveSnippetDown()
{
    QModelIndex idx = getCurrentSnippetIndex();
    QModelIndex newIdx = viewModel->mapFromSource(dataModel->moveDown(viewModel->mapToSource(idx)));
    if (idx != newIdx)
        selectSnippet(newIdx, true);

    updateState();
}

void CodeSnippetEditor::updateModified()
{
    if (updatesForSelection)
        return;

    QModelIndex idx = getCurrentSnippetIndex();
    if (idx.isValid())
    {
        bool nameDiff = idx.data(Qt::DisplayRole) != ui->nameEdit->text();
        bool codeDiff = idx.data(CodeSnippetEditorModel::CODE) != ui->mainCodeEdit->toPlainText();
        bool hotkeyDiff = QKeySequence(idx.data(CodeSnippetEditorModel::HOTKEY).toString()) != ui->assistantShortcutEdit->keySequence();
        currentModified = (nameDiff || codeDiff || hotkeyDiff);
    }

    updateCurrentSnippetState();
}

void CodeSnippetEditor::updateCurrentSnippetState()
{
    QModelIndex idx = getCurrentSnippetIndex();
    bool validRow = idx.isValid();
    ui->rightWidget->setEnabled(validRow);
    if (!validRow)
    {
        setValidState(ui->nameEdit, true);
        setValidState(ui->mainCodeEdit, true);
        setValidState(ui->assistantShortcutEdit, true);
        updateState();
        return;
    }

    QModelIndex srcIdx = viewModel->mapToSource(idx);

    QString name = ui->nameEdit->text();
    bool nameOk = !name.trimmed().isEmpty() && dataModel->isAllowedName(srcIdx, name);
    setValidState(ui->nameEdit, nameOk, tr("Enter a non-empty, unique name of the snippet."));

    bool codeOk = !ui->mainCodeEdit->toPlainText().trimmed().isEmpty();
    setValidState(ui->mainCodeEdit, codeOk, tr("Enter a non-empty snippet content."));

    QKeySequence assistantHotkey = ui->assistantShortcutEdit->keySequence();
    bool hotkeyOk = assistantHotkey.isEmpty() || dataModel->isAllowedHotkey(srcIdx, assistantHotkey);
    setValidState(ui->assistantShortcutWidget, hotkeyOk, tr("This hotkey is not unique in context of a code assistant."));

    viewModel->setData(idx, codeOk && nameOk && hotkeyOk, CodeSnippetEditorModel::VALID);
    updateState();
}

void CodeSnippetEditor::updateState()
{
    bool modified = dataModel->isModified() || currentModified;
    bool valid = dataModel->isValid();

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
        snippetDeselected(deselected.indexes().first());

    if (selCnt > 0)
        snippetSelected(selected.indexes().first());

    if (deselCnt > 0 && selCnt == 0)
    {
        currentModified = false;
        clearEdits();
    }
}

void CodeSnippetEditor::applyFilter(const QString& value)
{
    // The selection hack (clear & set) below is described in more details in FunctionsEditor::applyFilter().
    QModelIndex idx = getCurrentSnippetIndex();
    ui->list->selectionModel()->clearSelection();

    viewModel->setFilterFixedString(value);

    selectSnippet(idx);
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
    static const QString url = QStringLiteral("https://github.com/pawelsalawa/sqlitestudio/wiki/Code-snippets");
    QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode));
}

void CodeSnippetEditor::cfgCodeSnippetListChanged()
{
    if (dataModel->isModified())
        return; // Don't update list if there are uncommitted changes, because it would be disruptive for user. Changes will be visible after commit or rollback.

    dataModel->setSnippets(CODESNIPPETS->getSnippets());
    updateCurrentSnippetState();
}

void CodeSnippetEditor::importSnippets()
{
    SettingsImportDialog::importFromFile(SettingsImportDialog::SNIPPET);
}

void CodeSnippetEditor::exportSnippets()
{
    SettingsExportDialog::exportToFile(SettingsExportDialog::SNIPPET);
}

void CodeSnippetEditor::editSnippet(const QString& name)
{
    QModelIndexList resList = viewModel->match(viewModel->index(0, 0), Qt::DisplayRole, name, 1, Qt::MatchExactly);
    if (resList.isEmpty())
        return;

    selectSnippet(resList.first());
}
