#include "completerwindow.h"
#include "ui_completerwindow.h"
#include "completermodel.h"
#include "common/unused.h"
#include "sqleditor.h"
#include "common/utils_sql.h"
#include <QKeyEvent>
#include <QListView>
#include <QDebug>

CompleterWindow::CompleterWindow(SqlEditor *parent) :
    QDialog(parent, Qt::FramelessWindowHint),
    ui(new Ui::CompleterWindow),
    sqlEditor(parent)
{
    ui->setupUi(this);
    init();
}

CompleterWindow::~CompleterWindow()
{
    delete ui;
}

void CompleterWindow::init()
{
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
    reset();
}

void CompleterWindow::reset()
{
    model->clear();
    ui->status->showMessage(QString::null);
}

void CompleterWindow::setData(const CompletionHelper::Results& completionResults)
{
    ui->status->showMessage(QString::null);
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
    if (filter.isEmpty() && text.size() == 1 && isWrapperChar(text[0], db->getDialect()))
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

ExpectedTokenPtr CompleterWindow::getSelected()
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
    if (e->key() == Qt::Key_Return)
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
            return tr("String", "completer statusbar");
        case ExpectedToken::NUMBER:
            return tr("Number", "completer statusbar").arg(value);
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
