#include "ddlhistorywindow.h"
#include "ui_ddlhistorywindow.h"
#include "services/config.h"
#include "ddlhistorymodel.h"
#include "common/unused.h"
#include "iconmanager.h"
#include <QDate>
#include <QLineEdit>
#include <QMessageBox>
#include <QStringListModel>

DdlHistoryWindow::DdlHistoryWindow(QWidget *parent) :
    MdiChild(parent),
    ui(new Ui::DdlHistoryWindow)
{
    init();
}

DdlHistoryWindow::~DdlHistoryWindow()
{
    delete ui;
}

void DdlHistoryWindow::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void DdlHistoryWindow::init()
{
    ui->setupUi(this);

    dataModel = CFG->getDdlHistoryModel();

    dbListModel = new QStringListModel(this);
    QStringList dbList = dataModel->getDbNames();
    dbList.prepend("");
    dbListModel->setStringList(dbList);
    ui->comboBox->setModel(dbListModel);
    ui->comboBox->setCurrentIndex(-1);
    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(applyFilter(QString)));
    connect(dataModel, SIGNAL(refreshed()), this, SLOT(refreshDbList()));

    ui->tableView->setModel(dataModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    connect(ui->tableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(activated(QModelIndex,QModelIndex)));
    connect(ui->clearButton, SIGNAL(clicked(bool)), this, SLOT(clearHistory()));
}

void DdlHistoryWindow::activated(const QModelIndex& current, const QModelIndex& previous)
{
    UNUSED(previous);

    int row = current.row();
    QString dbName = dataModel->data(dataModel->index(row, 0)).toString();
    QString dbFile = dataModel->data(dataModel->index(row, 1)).toString();
    QString dateString = dataModel->data(dataModel->index(row, 2)).toString();
    QDate date = QDate::fromString(dateString, "yyyy-MM-dd");

    static const QString templ = tr("-- Queries executed on database %1 (%2)\n"
                                    "-- Date and time of execution: %3\n"
                                    "%4");

    QStringList contentEntries;
    QList<Config::DdlHistoryEntryPtr> entries = CFG->getDdlHistoryFor(dbName, dbFile, date);
    for (Config::DdlHistoryEntryPtr& entry : entries)
        contentEntries << templ.arg(entry->dbName, entry->dbFile, entry->timestamp.toString("yyyy-MM-dd HH:mm:ss"), entry->queries);

    ui->ddlEdit->setPlainText(contentEntries.join("\n\n"));
}

void DdlHistoryWindow::applyFilter(const QString& filterValue)
{
    dataModel->setDbNameForFilter(filterValue);
}

void DdlHistoryWindow::refreshDbList()
{
    QStringList dbList = dataModel->getDbNames();
    dbList.prepend("");
    dbListModel->setStringList(dbList);
}

void DdlHistoryWindow::clearHistory()
{
    QMessageBox::StandardButton result = QMessageBox::question(this, tr("Clear history"), tr("Are you sure you want to erase entire DDL history?"));
    if (result != QMessageBox::Yes)
        return;

    CFG->clearDdlHistory();
    dataModel->refresh();
    ui->ddlEdit->setPlainText("");
}

bool DdlHistoryWindow::restoreSessionNextTime()
{
    return false;
}

QVariant DdlHistoryWindow::saveSession()
{
    return QVariant();
}

bool DdlHistoryWindow::restoreSession(const QVariant& sessionValue)
{
    UNUSED(sessionValue);
    return true;
}

Icon* DdlHistoryWindow::getIconNameForMdiWindow()
{
    return ICONS.DDL_HISTORY;
}

QString DdlHistoryWindow::getTitleForMdiWindow()
{
    return tr("DDL history");
}

void DdlHistoryWindow::createActions()
{
}

void DdlHistoryWindow::setupDefShortcuts()
{
}

QToolBar* DdlHistoryWindow::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return nullptr;
}


bool DdlHistoryWindow::isUncommitted() const
{
    return false;
}

QString DdlHistoryWindow::getQuitUncommittedConfirmMessage() const
{
    return QString();
}
