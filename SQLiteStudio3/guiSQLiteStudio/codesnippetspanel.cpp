#include "codesnippetspanel.h"
#include "ui_codesnippetspanel.h"
#include "windows/codesnippeteditor.h"
#include "windows/codesnippeteditormodel.h"
#include "sqlitestudio.h"
#include "sqlitesyntaxhighlighter.h"
#include "services/codesnippetmanager.h"
#include "common/userinputfilter.h"
#include "mainwindow.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QMimeData>
#include <QSortFilterProxyModel>

CodeSnippetsPanel::CodeSnippetsPanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::CodeSnippetsPanel)
{
    init();
}

CodeSnippetsPanel::~CodeSnippetsPanel()
{
    delete ui;
}

QVariant CodeSnippetsPanel::saveSession()
{
    QVariantHash session;
    session["splitter"] = ui->splitter->saveState();
    return session;
}

void CodeSnippetsPanel::restoreSession(const QVariant& value)
{
    QVariantHash session = value.toHash();
    if (session.contains("splitter"))
        ui->splitter->restoreState(session["splitter"].toByteArray());
}

void CodeSnippetsPanel::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasText())
    {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else
        event->ignore();
}

void CodeSnippetsPanel::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasText())
    {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else
        event->ignore();
}

void CodeSnippetsPanel::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasText())
    {
        const QString code = event->mimeData()->text();
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add code snippet"), tr("Enter a name for the new code snippet:"), QLineEdit::Normal, QString(), &ok);
        if (ok)
        {
            CodeSnippetManager::CodeSnippet* snippet = new CodeSnippetManager::CodeSnippet;
            snippet->name = name;
            snippet->code = code;
            CODESNIPPETS->addSnippet(snippet);
        }
        event->setDropAction(Qt::CopyAction);
        event->accept();
        return;
    }

    event->ignore();
}

void CodeSnippetsPanel::init()
{
    ui->setupUi(this);

    dataModel = new CodeSnippetEditorModel(this);
    dataModel->setDragEnabled(true);
    viewModel = new QSortFilterProxyModel(this);
    viewModel->setSourceModel(dataModel);
    ui->snippetsListView->setModel(viewModel);

    nameFilter = new QLineEdit(ui->toolBar);
    nameFilter->setClearButtonEnabled(true);
    nameFilter->setPlaceholderText(tr("Filter by name"));
    nameFilter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->toolBar->addWidget(nameFilter);

    new UserInputFilter(nameFilter, this, SLOT(applyFilter(QString)));
    viewModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    dataModel->setSnippets(CODESNIPPETS->getSnippets());
    connect(CODESNIPPETS, SIGNAL(codeSnippetListChanged()), this, SLOT(cfgCodeSnippetListChanged()));

    connect(ui->snippetsListView, SIGNAL(activated(QModelIndex)), this, SLOT(editSnippet(QModelIndex)));
    connect(ui->snippetsListView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(snippetSelected(QModelIndex,QModelIndex)));
}

void CodeSnippetsPanel::cfgCodeSnippetListChanged()
{
    dataModel->setSnippets(CODESNIPPETS->getSnippets());
}

void CodeSnippetsPanel::snippetSelected(const QModelIndex& current, const QModelIndex& previous)
{
    ui->previewEdit->setContents(current.data(CodeSnippetEditorModel::CODE).toString());
}

void CodeSnippetsPanel::applyFilter(const QString& value)
{
    // The selection hack (clear & set) below is described in more details in FunctionsEditor::applyFilter().
    QModelIndex idx = ui->snippetsListView->selectionModel()->currentIndex();
    ui->snippetsListView->selectionModel()->clearSelection();

    viewModel->setFilterFixedString(value);

    if (idx.isValid())
        ui->snippetsListView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
}

void CodeSnippetsPanel::editSnippet(const QModelIndex& idx)
{
    MAINWINDOW->openCodeSnippetEditor()->editSnippet(idx.data(Qt::DisplayRole).toString());
}
