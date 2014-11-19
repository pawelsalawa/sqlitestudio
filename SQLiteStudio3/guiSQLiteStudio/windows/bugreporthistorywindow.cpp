#include "bugreporthistorywindow.h"
#include "ui_bugreporthistorywindow.h"
#include "common/unused.h"
#include "services/config.h"
#include <QDebug>
#include <QLabel>

CFG_KEYS_DEFINE(BugReportHistoryWindow)

BugReportHistoryWindow::BugReportHistoryWindow(QWidget *parent) :
    MdiChild(parent),
    ui(new Ui::BugReportHistoryWindow)
{
    init();
}

BugReportHistoryWindow::~BugReportHistoryWindow()
{
    delete ui;
}

bool BugReportHistoryWindow::restoreSessionNextTime()
{
    return false;
}

QVariant BugReportHistoryWindow::saveSession()
{
    return QVariant();
}

bool BugReportHistoryWindow::restoreSession(const QVariant& sessionValue)
{
    UNUSED(sessionValue);
    return false;
}

Icon* BugReportHistoryWindow::getIconNameForMdiWindow()
{
    return ICONS.BUG_LIST;
}

QString BugReportHistoryWindow::getTitleForMdiWindow()
{
    return tr("Reports history");
}

void BugReportHistoryWindow::createActions()
{
    createAction(CLEAR_HISTORY, ICONS.CLEAR_HISTORY, tr("Clear reports history"), this, SLOT(clearHistory()), ui->toolBar);
    createAction(DELETE_SELECTED, ICONS.DELETE_ROW, tr("Delete selected entry"), this, SLOT(deleteSelected()), ui->toolBar);
}

void BugReportHistoryWindow::setupDefShortcuts()
{
    setShortcutContext({
                           DELETE_SELECTED
                       },
                       Qt::WidgetWithChildrenShortcut);

    BIND_SHORTCUTS(BugReportHistoryWindow, Action);
}

QToolBar* BugReportHistoryWindow::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return ui->toolBar;
}

void BugReportHistoryWindow::init()
{
    ui->setupUi(this);
    initActions();

    reload();
    connect(ui->reportsList->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateState()));
    connect(CFG, SIGNAL(reportsHistoryRefreshNeeded()), this, SLOT(reload()));

    updateState();
}

void BugReportHistoryWindow::updateState()
{
    actionMap[DELETE_SELECTED]->setEnabled(ui->reportsList->selectedItems().size() > 0);
}

void BugReportHistoryWindow::reload()
{
    static_qstring(urlTpl, "<a href=\"%1\">%2</a>");
    QString invalidUrlTpl = tr("Invalid response from server.");

    QList<Config::ReportHistoryEntryPtr> entries = CFG->getReportHistory();
    ui->reportsList->clear();
    ui->reportsList->setRowCount(entries.size());

    QTableWidgetItem* item = nullptr;
    QLabel* urlLabel = nullptr;
    int row = 0;
    for (const Config::ReportHistoryEntryPtr& entry : entries)
    {
        item = new QTableWidgetItem((entry->isFeatureRequest ? ICONS.FEATURE_REQUEST : ICONS.BUG), entry->title);
        item->setData(ENTRY_ID, entry->id);
        ui->reportsList->setItem(row, 0, item);

        item = new QTableWidgetItem(QDateTime::fromTime_t(entry->timestamp).toString("yyyy-MM-dd HH:mm:ss"));
        ui->reportsList->setItem(row, 1, item);

        if (entry->url.startsWith("http://"))
            urlLabel = new QLabel(urlTpl.arg(entry->url, entry->url));
        else
            urlLabel = new QLabel(invalidUrlTpl);

        urlLabel->setOpenExternalLinks(true);
        ui->reportsList->setCellWidget(row, 2, urlLabel);

        row++;
    }

    ui->reportsList->setHorizontalHeaderLabels({tr("Title"), tr("Reported at"), tr("URL")});
    ui->reportsList->resizeColumnsToContents();
}

void BugReportHistoryWindow::clearHistory()
{
    CFG->clearReportHistory();
}

void BugReportHistoryWindow::deleteSelected()
{
    QList<QTableWidgetItem*> items = ui->reportsList->selectedItems();
    if (items.size() == 0)
    {
        qDebug() << "Called BugReportHistoryWindow::deleteSelected(), but there's no row selected.";
        return;
    }

    int id = items.first()->data(ENTRY_ID).toInt();
    if (id == 0)
    {
        qDebug() << "Called BugReportHistoryWindow::deleteSelected(), but there's no ID in selected row.";
        return;
    }

    CFG->deleteReport(id);
}

bool BugReportHistoryWindow::isUncommited() const
{
    return false;
}

QString BugReportHistoryWindow::getQuitUncommitedConfirmMessage() const
{
    return QString();
}
