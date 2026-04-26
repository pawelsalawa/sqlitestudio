#include "completerwindow.h"
#include "completer/completersnippetdelegate.h"
#include "ui_completerwindow.h"
#include "completermodel.h"
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QLineEdit>
#include <QtSystemDetection>
#else
#include <qsystemdetection.h>
#endif

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

    snippetDelegate = new CompleterSnippetDelegate(ui->snippets);
    ui->snippets->setItemDelegate(snippetDelegate);

    setFocusProxy(ui->list);
    connect(ui->list, SIGNAL(focusOut()), this, SLOT(focusOut()));
    connect(ui->list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));
    connect(ui->list->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(currentRowChanged(QModelIndex,QModelIndex)));
    connect(modeChangeShortcut, SIGNAL(activated()), this, SLOT(modeChangeRequested()));
    connect(ui->snippets, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(snippetDoubleClicked(QListWidgetItem*)));
    connect(snippetSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(snippetHotkeyPressed(int)));

    ui->list->installEventFilter(this);
    ui->snippets->installEventFilter(this);

    QShortcut* snippetToggleShortcut = new QShortcut(Qt::Key_Slash, ui->snippets);
    connect(snippetToggleShortcut, &QShortcut::activated, this, &CompleterWindow::toggleSnippetsKeyMode);

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
            ui->snippets->item(i)->setData(QListWidgetItem::UserType, snip->hotkey);
        }
        i++;
    }

    if (ui->snippets->count() > 0)
        ui->snippets->setCurrentRow(0);
}

QString CompleterWindow::getSnippetsStatusMsg() const
{
    return snippetKeyMode == HOTKEY ? tr("Press / to filter snippets") : tr("Press / to use hotkeys");
}

void CompleterWindow::setSnippetsKeyMode(SnippetKeyMode mode)
{
    snippetKeyMode = mode;
    ui->status->showMessage(getSnippetsStatusMsg());
    applyFilterToSnippets();

    switch (mode)
    {
        case HOTKEY:
        {
            for (QShortcut*& sc : snippetShortcuts)
                sc->setEnabled(true);

            snippetDelegate->setShowHotkeys(true);
            break;
        }
        case FILTER:
        {
            for (QShortcut*& sc : snippetShortcuts)
                sc->setEnabled(false);

            snippetDelegate->setShowHotkeys(false);
            break;
        }
    }

    auto model = ui->snippets->model();
    QModelIndex topLeft = model->index(0, 0);
    QModelIndex bottomRight = model->index(model->rowCount() - 1, 0);
    emit model->dataChanged(topLeft, bottomRight);
}

CompleterWindow::SnippetKeyMode CompleterWindow::getSnippetKeyMode() const
{
    return snippetKeyMode;
}

void CompleterWindow::setInitialMode(Mode newInitialMode)
{
    initialMode = newInitialMode;
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
    applyFilterToSnippets();

    switch (getMode())
    {
        case CODE:
        {
            if (!ui->list->hasVisibleItem())
                reject();

            break;
        }
        case SNIPPETS:
        {
            if (!hasVisibleSnippets())
                reject();

            break;
        }
    }
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
    setMode(initialMode);
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

void CompleterWindow::setMode(Mode mode)
{
    if (getMode() == mode)
        return;

    ui->modeStack->setCurrentIndex(mode);
    switch (mode)
    {
        case SNIPPETS:
            setSnippetsKeyMode(HOTKEY);
            refreshSnippets();
            break;
        case CODE:
            ui->status->showMessage(getStatusMsg(ui->list->currentIndex()));
            break;
    }
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
#ifdef Q_OS_MAC
        // It appears, that under macOS, the Qt 6.5-6.7 (not sure about further versions of Qt)
        // does not emit the accepted() signal on the accept() method called.
        emit accepted();
#endif
        return;
    }

    QDialog::keyPressEvent(e);
}

QString CompleterWindow::getStatusMsg(const QModelIndex& index)
{
    if (getMode() == CompleterWindow::SNIPPETS)
        return getSnippetsStatusMsg();

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
            return tr("String", "completer statusbar");
        case ExpectedToken::NUMBER:
            return tr("Number", "completer statusbar");
        case ExpectedToken::BLOB:
            return tr("Binary data", "completer statusbar").arg(value);
        case ExpectedToken::COLLATION:
            return tr("Collation: %1", "completer statusbar").arg(value);
        case ExpectedToken::PRAGMA:
            return tr("Pragma function: %1", "completer statusbar").arg(value);
        case ExpectedToken::JOIN_EXPR:
            return tr("Join condition: %1", "completer statusbar").arg(value);
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

bool CompleterWindow::hasVisibleSnippets() const
{
    int visibleCount = 0;
    for (int i = 0; i < ui->snippets->count(); ++i)
    {
        if (!ui->snippets->item(i)->isHidden())
            visibleCount++;
    }
    return visibleCount > 0;
}

void CompleterWindow::applyFilterToSnippets()
{
    bool filterEnabled = !filter.isEmpty() && snippetKeyMode == FILTER;
    QString lowFilter = filter.toLower();
    for (int i = 0; i < ui->snippets->count(); ++i)
    {
        auto item = ui->snippets->item(i);
        item->setHidden(filterEnabled && !item->text().toLower().contains(lowFilter));
    }
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
    Q_UNUSED(index);
    accept();
}

void CompleterWindow::currentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);
    ui->status->showMessage(getStatusMsg(current));
}

void CompleterWindow::modeChangeRequested()
{
    Mode newMode = static_cast<Mode>((ui->modeStack->currentIndex() + 1) % ui->modeStack->count());
    setMode(newMode);
}

void CompleterWindow::snippetHotkeyPressed(int index)
{
    ui->snippets->setCurrentRow(index);
    accept();
}

void CompleterWindow::snippetDoubleClicked(QListWidgetItem* item)
{
    Q_UNUSED(item);
    accept();
}

void CompleterWindow::toggleSnippetsKeyMode()
{
    setSnippetsKeyMode(snippetKeyMode == HOTKEY ? FILTER : HOTKEY);
}

void CompleterWindow::showEvent(QShowEvent*e)
{
    setSnippetsKeyMode(HOTKEY);
    setMode(initialMode);
    QDialog::showEvent(e);

    // A hack for Gnome3 to give this widget a focus. Harmless for others.
    ui->list->activateWindow();

    // Refresh hotkey if changed
    modeChangeShortcut->setKey(GET_SHORTCUTS_CATEGORY(SqlEditor).COMPLETE.get());
}

bool CompleterWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj != ui->snippets && obj != ui->list)
        return false;

    if (event->type() != QEvent::KeyPress)
        return false;

    if (obj == ui->snippets && snippetKeyMode == HOTKEY)
        return false;

    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

    QKeySequence hotkey = GET_SHORTCUTS_CATEGORY(SqlEditor).COMPLETE.get();
    QKeySequence theKey = QKeySequence(keyEvent->key() | keyEvent->modifiers());
    if (hotkey == theKey)
        return true;

    QString txt = keyEvent->text();
    if (!txt.isEmpty() && txt[0].isPrint())
    {
        emit textTyped(txt);
        return true;
    }

    switch (keyEvent->key())
    {
        case Qt::Key_Backspace:
            emit backspacePressed();
            return true;
        case Qt::Key_Left:
            emit leftPressed();
            return true;
        case Qt::Key_Right:
            emit rightPressed();
            return true;
    }

    return false;
}
