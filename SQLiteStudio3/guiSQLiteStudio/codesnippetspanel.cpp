#include "codesnippetspanel.h"
#include "ui_codesnippetspanel.h"
#include "windows/codesnippeteditormodel.h"
#include "sqlitestudio.h"
#include "sqlitesyntaxhighlighter.h"
#include "services/codesnippetmanager.h"
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

void CodeSnippetsPanel::init()
{
    ui->setupUi(this);

    model = new CodeSnippetEditorModel(this);
    model->setDragEnabled(true);
    snippetFilterModel = new QSortFilterProxyModel(this);
    snippetFilterModel->setSourceModel(model);
    ui->snippetsListView->setModel(snippetFilterModel);

    model->setSnippets(CODESNIPPETS->getSnippets());
    connect(CODESNIPPETS, SIGNAL(codeSnippetListChanged()), this, SLOT(cfgCodeSnippetListChanged()));

    connect(ui->snippetsListView, SIGNAL(activated(QModelIndex)), this, SLOT(editSnippet(QModelIndex)));
    connect(ui->snippetsListView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(snippetSelected(QModelIndex,QModelIndex)));
}

void CodeSnippetsPanel::cfgCodeSnippetListChanged()
{
    model->setSnippets(CODESNIPPETS->getSnippets());
}

void CodeSnippetsPanel::snippetSelected(const QModelIndex& current, const QModelIndex& previous)
{
    ui->previewEdit->setContents(current.data(CodeSnippetEditorModel::CODE).toString());
}
