#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "dblistmodel.h"
#include "dbobjlistmodel.h"
#include "services/dbmanager.h"
#include "uiutils.h"
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QTextCodec>

ExportDialog::ExportDialog(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::ExportDialog)
{
    init();
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::setTableMode(Db* db, const QString& table)
{
    if (!db->isOpen())
    {
        qWarning() << "Cannot export from closed database.";
        return;
    }

    exportMode = ExportManager::TABLE;
    this->db = db;
    this->table = table;
}

void ExportDialog::setQueryMode(Db* db, const QString& query)
{
    if (!db->isOpen())
    {
        qWarning() << "Cannot export from closed database.";
        return;
    }

    exportMode = ExportManager::RESULTS;
    this->db = db;
    this->query = query;
}

void ExportDialog::setDatabaseMode(Db* db)
{
    if (!db->isOpen())
    {
        qWarning() << "Cannot export from closed database.";
        return;
    }

    exportMode = ExportManager::DATABASE;
    this->db = db;
}

void ExportDialog::init()
{
    ui->setupUi(this);
    initPageOrder();

    initModePage();
    initTablePage();
    initFormatPage();

    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));

}

void ExportDialog::initModePage()
{
    connect(ui->subjectDatabaseRadio, SIGNAL(clicked()), this, SLOT(updateExportMode()));
    connect(ui->subjectTableRadio, SIGNAL(clicked()), this, SLOT(updateExportMode()));
    connect(ui->subjectQueryRadio, SIGNAL(clicked()), this, SLOT(updateExportMode()));
}

void ExportDialog::initTablePage()
{
    ui->tablePage->setValidator([=]() -> bool
    {
        return ui->exportTableNameCombo->currentIndex() > -1;
    });

    dbListModel = new DbListModel(this);
    dbListModel->setCombo(ui->exportTableDbNameCombo);
    dbListModel->setSortMode(DbListModel::SortMode::Alphabetical);
    ui->exportTableDbNameCombo->setModel(dbListModel);
    connect(ui->exportTableDbNameCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDbTables()));

    dbObjListModel = new DbObjListModel(this);
    dbObjListModel->setType(DbObjListModel::ObjectType::TABLE);
    ui->exportTableNameCombo->setModel(dbObjListModel);
    connect(ui->exportTableNameCombo, SIGNAL(currentTextChanged(QString)), ui->tablePage, SIGNAL(completeChanged()));
}

void ExportDialog::initFormatPage()
{
    ui->formatAndOptionsPage->setValidator([=]() -> bool
    {
        if (ui->exportFileRadio->isChecked())
        {
            QString path = ui->exportFileEdit->text();
            if (path.trimmed().isEmpty())
                return false;

            QDir dir(path);
            if (dir.exists() && QFileInfo(path).isDir())
                return false;

            if (!dir.cdUp())
                return false;
        }
        return ui->formatCombo->currentIndex() > -1 && ui->encodingCombo->currentIndex() > -1;
    });

    ui->exportFileButton->setIcon(ICONS.EXPORT_FILE_BROWSE);
    connect(ui->exportFileButton, SIGNAL(clicked()), this, SLOT(browseForExportFile()));

    connect(ui->formatCombo, SIGNAL(currentTextChanged(QString)), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->encodingCombo, SIGNAL(currentTextChanged(QString)), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->exportFileEdit, SIGNAL(textChanged(QString)), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->exportFileRadio, SIGNAL(clicked()), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->exportClipboardRadio, SIGNAL(clicked()), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
}

int ExportDialog::nextId() const
{
    if (exportMode == ExportManager::UNDEFINED)
        return pageId(ui->exportSubjectPage);

    QList<QWizardPage*> order = pageOrder[exportMode];

    int idx = order.indexOf(currentPage());
    idx++;
    if (idx < order.size())
        return pageId(order[idx]);

    return -1;
}

void ExportDialog::initPageOrder()
{
    pageOrder[ExportManager::DATABASE] = {ui->databaseObjectsPage, ui->formatAndOptionsPage};
    pageOrder[ExportManager::TABLE] = {ui->tablePage, ui->formatAndOptionsPage};
    pageOrder[ExportManager::RESULTS] = {ui->queryPage, ui->formatAndOptionsPage};
}

int ExportDialog::pageId(QWizardPage* wizardPage) const
{
    for (int id : pageIds())
    {
        if (page(id) == wizardPage)
            return id;
    }
    return -1;
}

void ExportDialog::tablePageDisplayed()
{
    if (ui->exportTableNameCombo->currentIndex() == -1)
    {
        updateDbTables();
    }
}

void ExportDialog::queryPageDisplayed()
{
    if (ui->queryEdit->toPlainText().isEmpty())
        ui->queryEdit->setPlainText(query);
}

void ExportDialog::dbObjectsPageDisplayed()
{

}

void ExportDialog::formatPageDisplayed()
{
    if (ui->encodingCombo->count() == 0)
    {
        ui->encodingCombo->addItems(textCodecNames());
        ui->encodingCombo->setCurrentText(defaultCodecName());
    }
}

void ExportDialog::updateExportMode()
{
    if (ui->subjectDatabaseRadio->isChecked())
        exportMode = ExportManager::DATABASE;
    else if (ui->subjectTableRadio->isChecked())
        exportMode = ExportManager::TABLE;
    else if (ui->subjectQueryRadio->isChecked())
        exportMode = ExportManager::RESULTS;
    else
        exportMode = ExportManager::UNDEFINED;
}

void ExportDialog::pageChanged(int pageId)
{
    QWizardPage* wizardPage = page(pageId);
    if (wizardPage == ui->tablePage)
        tablePageDisplayed();
    else if (wizardPage == ui->queryPage)
        queryPageDisplayed();
    else if (wizardPage == ui->databaseObjectsPage)
        dbObjectsPageDisplayed();
    else if (wizardPage == ui->formatAndOptionsPage)
        formatPageDisplayed();
}

void ExportDialog::updateDbTables()
{
    QString dbName = ui->exportTableDbNameCombo->currentText();
    db = DBLIST->getByName(dbName);

    dbObjListModel->setDb(db);
}

void ExportDialog::browseForExportFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Pick file to export to"), QString(), QString(), 0, QFileDialog::DontConfirmOverwrite);
    if (fileName.isNull())
        return;

    ui->exportFileEdit->setText(fileName);
}

int ExportDialog::exec()
{
    switch (exportMode)
    {
        case ExportManager::DATABASE:
            setStartId(pageId(ui->databaseObjectsPage));
            break;
        case ExportManager::TABLE:
            setStartId(pageId(ui->tablePage));
            break;
        case ExportManager::RESULTS:
            setStartId(pageId(ui->queryPage));
            break;
        case ExportManager::UNDEFINED:
            setStartId(pageId(ui->exportSubjectPage));
            updateExportMode();
            break;
    }
    return QWizard::exec();
}
