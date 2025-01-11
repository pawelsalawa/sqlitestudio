#include "completerwindow.h"
#include "ui_completerwindow.h"
#include "completermodel.h"
#include "common/unused.h"
#include "sqleditor.h"
#include "common/utils_sql.h"
#include "services/codesnippetmanager.h"
#include "sqlitestudio.h"
#include <QKeyEvent>
#include <QSignalMapper>
#include <QListView>
#include <QShortcut>
#include <QDebug>
#include <QListWidget>

CompleterWindow::CompleterWindow(SqlEditor *parent) :
    QDialog(parent, Qt::FramelessWindowHint),
    ui(new Ui::CompleterWindow),
    sqlEditor(parent)
{
    init();
}

CompleterWindow::~CompleterWindow()
{
    delete ui;
}

void CompleterWindow::init()
{
    ui->setupUi(this);

    modeChangeShortcut = new QShortcut(GET_SHORTCUTS_CATEGORY(SqlEditor).COMPLETE.get(), this);
    snippetSignalMapper = new QSignalMapper(this);

    model = new CompleterModel(this);
    ui->list->setModel(model);
    model->setCompleterView(ui->list);

    setFocusProxy(ui->list);
    connect(ui->list, SIGNAL(focusOut()), this, SLOT(focusOut()));
    connect(ui->list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));
    connect(ui->list->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(currentRowChanged(QModelIndex,QModelIndex)));
    connect(ui->list, SIGNAL(textTyped(QString)), this, SIGNAL(textTyped(QString)));
    connect(ui->list, SIGNAL(backspace()), this, SIGNAL(backspacePressed()));
    connect(ui->list, SIGNAL(left()), this, SIGNAL(leftPressed()));
    connect(ui->list, SIGNAL(right()), this, SIGNAL(rightPressed()));
    connect(modeChangeShortcut, SIGNAL(activated()), this, SLOT(modeChangeRequested()));
    connect(ui->snippets, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(snippetDoubleClicked(QListWidgetItem*)));
    connect(snippetSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(snippetHotkeyPressed(int)));
    reset();
}

void CompleterWindow::refreshSnippets()
{
    ui->snippets->clear();
    for (QShortcut*& sc : snippetShortcuts)
        delete sc;

    snippetShortcuts.clear();

    int i = 0;
    for (CodeSnippetManager::CodeSnippet* snip : CODESNIPPETS->getSnippets())
    {
        ui->snippets->addItem(snip->name);
        if (!snip->hotkey.isEmpty())
        {
            QShortcut* shortcut = new QShortcut(snip->hotkey, ui->snippets);
            snippetShortcuts << shortcut;
            snippetSignalMapper->setMapping(shortcut, i);
            connect(shortcut, SIGNAL(activated()), snippetSignalMapper, SLOT(map()));
        }
        i++;
    }

    if (ui->snippets->count() > 0)
        ui->snippets->setCurrentRow(0);
}

void CompleterWindow::reset()
{
    model->clear();
    ui->status->showMessage(QString());
}

void CompleterWindow::setData(const CompletionHelper::Results& completionResults)
{
    ui->status->showMessage(QString());
    model->setData(completionResults.expectedTokens);
    filter = completionResults.partialToken;
    wrappedFilter = completionResults.wrappedToken;
    updateFilter();
}

void CompleterWindow::setDb(Db* db)
{
    this->db = db;
}

void CompleterWindow::updateFilter()
{
    model->setFilter(filter);
    ui->list->selectFirstVisible();

    if (!ui->list->hasVisibleItem())
        reject();
}

void CompleterWindow::shringFilterBy(int chars)
{
    if (filter.size() < chars)
    {
        if (wrappedFilter && chars == 1)
        {
            wrappedFilter = false;
            updateFilter();
            return;
        }

        reject();
        return;
    }

    filter.truncate(filter.length() - chars);
    updateFilter();
}

void CompleterWindow::extendFilterBy(const QString& text)
{
    if (filter.isEmpty() && text.size() == 1 && isWrapperChar(text[0]))
    {
        wrappedFilter = true;
        updateFilter();
        return;
    }

    filter.append(text);
    updateFilter();
}

bool CompleterWindow::immediateResolution()
{
    if (ui->list->countVisibleItem() == 1)
    {
        accept();
        return true;
    }
    return false;
}

CompleterWindow::Mode CompleterWindow::getMode() const
{
    return static_cast<Mode>(ui->modeStack->currentIndex());
}

QString CompleterWindow::getSnippetName() const
{
    return ui->snippets->currentItem()->text();
}

ExpectedTokenPtr CompleterWindow::getSelected() const
{
    QModelIndex current = ui->list->currentIndex();
    if (!current.isValid())
        return ExpectedTokenPtr();

    return model->getToken(current.row());
}

int CompleterWindow::getNumberOfCharsToRemove()
{
    return filter.size() + (wrappedFilter ? 1 : 0);
}

void CompleterWindow::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void CompleterWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
    {
        accept();
        return;
    }

    QDialog::keyPressEvent(e);
}

QString CompleterWindow::getStatusMsg(const QModelIndex& index)
{
    ExpectedToken::Type type = (ExpectedToken::Type)index.data(CompleterModel::TYPE).toInt();
    QString value = index.data(CompleterModel::VALUE).toString();
    QString label = index.data(CompleterModel::LABEL).toString();

    switch (type)
    {
        case ExpectedToken::COLUMN:
            return tr("Column: %1", "completer statusbar").arg(value);
        case ExpectedToken::TABLE:
            return tr("Table: %1", "completer statusbar").arg(value);
        case ExpectedToken::INDEX:
            return tr("Index: %1", "completer statusbar").arg(value);
        case ExpectedToken::TRIGGER:
            return tr("Trigger: %1", "completer statusbar").arg(value);
        case ExpectedToken::VIEW:
            return tr("View: %1", "completer statusbar").arg(value);
        case ExpectedToken::DATABASE:
            return tr("Database: %1", "completer statusbar").arg(value);
        case ExpectedToken::OTHER:
        {
            if (!label.isEmpty())
                return label;

            if (!value.isEmpty())
                return value;

            return "";
        }
        case ExpectedToken::KEYWORD:
            return tr("Keyword: %1", "completer statusbar").arg(value);
        case ExpectedToken::FUNCTION:
            return tr("Function: %1", "completer statusbar").arg(value);
        case ExpectedToken::OPERATOR:
            return tr("Operator: %1", "completer statusbar").arg(value);
        case ExpectedToken::STRING:
            return tr("String", "completer statusbar").arg(value);
        case ExpectedToken::NUMBER:
            return tr("Number", "completer statusbar");
        case ExpectedToken::BLOB:
            return tr("Binary data", "completer statusbar").arg(value);
        case ExpectedToken::COLLATION:
            return tr("Collation: %1", "completer statusbar").arg(value);
        case ExpectedToken::PRAGMA:
            return tr("Pragma function: %1", "completer statusbar").arg(value);
        case ExpectedToken::NO_VALUE:
        {
            if (!label.isEmpty())
                return label;

            if (!value.isEmpty())
                return value;

            return "";
        }
    }
    return "";
}

void CompleterWindow::focusOut()
{
    QWidget* focused = QApplication::focusWidget();
    if (!focused || focused == this || isAncestorOf(focused))
        return;

    reject();
}

void CompleterWindow::doubleClicked(const QModelIndex& index)
{
    UNUSED(index);
    accept();
}

void CompleterWindow::currentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    UNUSED(previous);
    ui->status->showMessage(getStatusMsg(current));
}

void CompleterWindow::modeChangeRequested()
{
    ui->modeStack->setCurrentIndex((ui->modeStack->currentIndex() + 1) % ui->modeStack->count());

    switch (ui->modeStack->currentIndex())
    {
        case SNIPPETS:
            ui->status->showMessage(tr("Insert a code snippet"));
            refreshSnippets();
            break;
        case CODE:
            ui->status->showMessage(getStatusMsg(ui->list->currentIndex()));
            break;
        }
}

void CompleterWindow::snippetHotkeyPressed(int index)
{
    ui->snippets->setCurrentRow(index);
    accept();
}

void CompleterWindow::snippetDoubleClicked(QListWidgetItem* item)
{
    UNUSED(item);
    accept();
}

void CompleterWindow::showEvent(QShowEvent*e)
{
    ui->modeStack->setCurrentIndex(0);
    QDialog::showEvent(e);

    // A hack for Gnome3 to give this widget a focus. Harmless for others.
    ui->list->activateWindow();

    // Refresh hotkey if changed
    modeChangeShortcut->setKey(GET_SHORTCUTS_CATEGORY(SqlEditor).COMPLETE.get());
}
