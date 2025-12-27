#include "sqliteextensioneditor.h"
#include "sqliteextensioneditormodel.h"
#include "ui_sqliteextensioneditor.h"
#include "selectabledbmodel.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include "common/unused.h"
#include "uiutils.h"
#include "uiconfig.h"
#include "db/db.h"
#include "services/dbmanager.h"
#include "common/lazytrigger.h"
#include "common/userinputfilter.h"
#include <QDesktopServices>
#include <QFileDialog>
#include <QSortFilterProxyModel>

CFG_KEYS_DEFINE(SqliteExtensionEditor)

SqliteExtensionEditor::SqliteExtensionEditor(QWidget *parent) :
    MdiChild(parent),
    ui(new Ui::SqliteExtensionEditor)
{
    init();
}

SqliteExtensionEditor::~SqliteExtensionEditor()
{
    delete ui;
    probingDb->closeQuiet();
}

bool SqliteExtensionEditor::restoreSessionNextTime()
{
    return false;
}

bool SqliteExtensionEditor::isUncommitted() const
{
    return model->isModified() || currentModified;
}

QString SqliteExtensionEditor::getQuitUncommittedConfirmMessage() const
{
    return tr("Extension manager window has uncommitted modifications.");
}

QVariant SqliteExtensionEditor::saveSession()
{
    return QVariant();
}

bool SqliteExtensionEditor::restoreSession(const QVariant& sessionValue)
{
    UNUSED(sessionValue);
    return true;
}

Icon*SqliteExtensionEditor::getIconNameForMdiWindow()
{
    return ICONS.EXTENSION_EDITOR;
}

QString SqliteExtensionEditor::getTitleForMdiWindow()
{
    return tr("Extension manager");
}

void SqliteExtensionEditor::createActions()
{
    createAction(COMMIT, ICONS.COMMIT, tr("Commit all extension changes"), this, SLOT(commit()), ui->toolbar, this);
    createAction(ROLLBACK, ICONS.ROLLBACK, tr("Rollback all extension changes"), this, SLOT(rollback()), ui->toolbar, this);
    ui->toolbar->addSeparator();
    createAction(ADD, ICONS.EXTENSION_ADD, tr("Add new extension"), this, SLOT(newExtension()), ui->toolbar, this);
    createAction(DELETE, ICONS.EXTENSION_DELETE, tr("Remove selected extension"), this, SLOT(deleteExtension()), ui->toolbar, this);
    ui->toolbar->addSeparator();
    createAction(HELP, ICONS.HELP, tr("Editing extensions manual"), this, SLOT(help()), ui->toolbar, this);
}

void SqliteExtensionEditor::setupDefShortcuts()
{
    // Widget context
    setShortcutContext({COMMIT, ROLLBACK}, Qt::WidgetWithChildrenShortcut);
    BIND_SHORTCUTS(SqliteExtensionEditor, Action);
}

QToolBar* SqliteExtensionEditor::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return ui->toolbar;
}

void SqliteExtensionEditor::init()
{
    ui->setupUi(this);
    initActions();

    statusUpdateTrigger = new LazyTrigger(500, this, SLOT(updateCurrentExtensionState()));

    model = new SqliteExtensionEditorModel(this);
    extensionFilterModel = new QSortFilterProxyModel(this);
    extensionFilterModel->setSourceModel(model);
    ui->extensionList->setModel(extensionFilterModel);

    new UserInputFilter(ui->extensionFilterEdit, this, SLOT(applyFilter(QString)));
    extensionFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    dbListModel = new SelectableDbModel(this);
    dbListModel->setDisabledVersion(2);
    dbListModel->setSourceModel(DBTREE->getModel());
    ui->databaseList->setModel(dbListModel);
    ui->databaseList->expandAll();

    model->setData(SQLITE_EXTENSIONS->getAllExtensions());

    connect(ui->extensionList->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(extensionSelected(QItemSelection,QItemSelection)));
    connect(ui->extensionList->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateState()));
    connect(ui->fileEdit, SIGNAL(textChanged(QString)), this, SLOT(updateModified()));
    connect(ui->initEdit, SIGNAL(textChanged(QString)), this, SLOT(updateModified()));
    connect(ui->allDatabasesRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->selectedDatabasesRadio, SIGNAL(clicked()), this, SLOT(updateModified()));
    connect(ui->fileBrowse, SIGNAL(clicked()), this, SLOT(browseForFile()));
    connect(ui->fileEdit, SIGNAL(textChanged(QString)), statusUpdateTrigger, SLOT(schedule()));
    connect(ui->fileEdit, SIGNAL(textChanged(QString)), this, SLOT(generateName()));
    connect(ui->initEdit, SIGNAL(textChanged(QString)), statusUpdateTrigger, SLOT(schedule()));
    connect(dbListModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateModified()));

    probingDb = DBLIST->createInMemDb(true);

    if (!probingDb->openQuiet())
        qWarning() << "Could not open in-memory dtabase for Extension manager window. Probing files will be impossible.";

    initStateForAll();
    updateState();
    updateCurrentExtensionState();
}

int SqliteExtensionEditor::getCurrentExtensionRow() const
{
    QModelIndexList idxList = ui->extensionList->selectionModel()->selectedIndexes();
    if (idxList.size() == 0)
        return -1;

    return extRowToSrc(idxList.first()).row();
}

QModelIndex SqliteExtensionEditor::extRowToSrc(const QModelIndex &idx) const
{
    return extensionFilterModel->mapToSource(idx);
}

void SqliteExtensionEditor::extensionDeselected(int srcRow)
{
    statusUpdateTrigger->cancel();

    model->setFilePath(srcRow, ui->fileEdit->text());
    model->setInitFunction(srcRow, ui->initEdit->text());
    model->setAllDatabases(srcRow, ui->allDatabasesRadio->isChecked());
    model->setModified(srcRow, currentModified);

    if (ui->selectedDatabasesRadio->isChecked())
        model->setDatabases(srcRow, getCurrentDatabases());

    model->setValid(srcRow, validateExtension(srcRow));
}

void SqliteExtensionEditor::extensionSelected(int srcRow)
{
    updatesForSelection = true;
    ui->fileEdit->setText(model->getFilePath(srcRow));
    ui->initEdit->setText(model->getInitFunction(srcRow));

    // Databases
    dbListModel->setDatabases(model->getDatabases(srcRow));
    ui->databaseList->expandAll();

    if (model->getAllDatabases(srcRow))
        ui->allDatabasesRadio->setChecked(true);
    else
        ui->selectedDatabasesRadio->setChecked(true);

    updatesForSelection = false;
    currentModified = model->isModified(srcRow);

    updateCurrentExtensionState();
}

void SqliteExtensionEditor::clearEdits()
{
    ui->fileEdit->setText(QString());
    ui->initEdit->setText(QString());
    ui->allDatabasesRadio->setChecked(true);
}

void SqliteExtensionEditor::selectExtension(int srcRow)
{
    if (!model->isValidRowIndex(srcRow))
        return;

    ui->extensionList->selectionModel()->setCurrentIndex(extensionFilterModel->mapFromSource(model->index(srcRow)), QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
}

QStringList SqliteExtensionEditor::getCurrentDatabases() const
{
    return dbListModel->getDatabases();
}

bool SqliteExtensionEditor::tryToLoad(const QString& filePath, const QString& initFunc, QString* resultError)
{
    if (!probingDb->isOpen())
    {
        qWarning() << "Probing database is closed. Cannot evaluate if file" << filePath << "is loadable.";
        return true;
    }

    bool loadedOk = probingDb->loadExtension(filePath, initFunc.isEmpty() ? QString() : initFunc);
    if (!loadedOk && resultError)
        *resultError = probingDb->getErrorText();

    probingDb->closeQuiet();
    probingDb->openQuiet();

    return loadedOk;
}

bool SqliteExtensionEditor::validateExtension(bool* fileOk, bool* initOk, QString* fileError)
{
    QString filePath = ui->fileEdit->text();
    QString initFunc = ui->initEdit->text();
    return validateExtension(filePath, initFunc, fileOk, initOk, fileError);
}

bool SqliteExtensionEditor::validateExtension(int row)
{
    QString filePath = model->getFilePath(row);
    QString initFunc = model->getInitFunction(row);
    return validateExtension(filePath, initFunc, nullptr, nullptr, nullptr);
}

bool SqliteExtensionEditor::validateCurrentExtension()
{
    QString filePath = ui->fileEdit->text();
    QString initFunc = ui->initEdit->text();
    return validateExtension(filePath, initFunc, nullptr, nullptr, nullptr);
}

bool SqliteExtensionEditor::validateExtension(const QString& filePath, const QString& initFunc, bool* fileOk, bool* initOk, QString* fileError)
{
    bool localFileOk = true;
    bool localInitOk = true;

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isReadable())
    {
        localFileOk = false;
        if (fileError)
            *fileError = tr("File with given path does not exist or is not readable.");
    }
    else
        localFileOk = tryToLoad(filePath, initFunc, fileError);

    if (!localFileOk && fileError && fileError->isEmpty())
        *fileError = tr("Unable to load extension: %1").arg(filePath);

    static const QRegularExpression initFuncRegExp("^[a-zA-Z0-9_]*$");
    localInitOk = initFuncRegExp.match(initFunc).hasMatch();

    if (fileOk)
        *fileOk = localFileOk;

    if (initOk)
        *initOk = localInitOk;

    return localFileOk && localInitOk;
}

void SqliteExtensionEditor::initStateForAll()
{
    for (int i = 0, total = model->rowCount(); i < total; ++i)
    {
        model->setName(i, QFileInfo(model->getFilePath(i)).baseName());
        model->setValid(i, validateExtension(i));
    }
}

void SqliteExtensionEditor::help()
{
    static const QString url = QStringLiteral("https://github.com/pawelsalawa/sqlitestudio/wiki/User_Manual#sqlite-extensions");
    QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode));
}

void SqliteExtensionEditor::commit()
{
    int srcRow = getCurrentExtensionRow();
    if (model->isValidRowIndex(srcRow))
        extensionDeselected(srcRow);

    QList<SqliteExtensionManager::ExtensionPtr> extensions = model->getExtensions();

    SQLITE_EXTENSIONS->setExtensions(extensions);
    model->clearModified();
    currentModified = false;

    if (model->isValidRowIndex(srcRow))
        selectExtension(srcRow);

    updateState();
}

void SqliteExtensionEditor::rollback()
{
    int selectedBefore = getCurrentExtensionRow();

    model->setData(SQLITE_EXTENSIONS->getAllExtensions());
    currentModified = false;
    clearEdits();

    if (model->isValidRowIndex(selectedBefore))
        selectExtension(selectedBefore);

    initStateForAll();
    updateState();
}

void SqliteExtensionEditor::newExtension()
{
    model->addExtension(SqliteExtensionManager::ExtensionPtr::create());
    selectExtension(model->rowCount() - 1);
}

void SqliteExtensionEditor::deleteExtension()
{
    nameGenerationActive = false;
    int srcRow = getCurrentExtensionRow();
    model->deleteExtension(srcRow);
    clearEdits();

    srcRow = getCurrentExtensionRow();
    if (model->isValidRowIndex(srcRow))
        extensionSelected(srcRow);
    else
        updateCurrentExtensionState();

    nameGenerationActive = true;
    updateState();
}

void SqliteExtensionEditor::updateState()
{
    bool modified = model->isModified() || currentModified;
    bool valid = model->isValid() && (getCurrentExtensionRow() == -1 || validateCurrentExtension());

    actionMap[COMMIT]->setEnabled(modified && valid);
    actionMap[ROLLBACK]->setEnabled(modified);
    actionMap[DELETE]->setEnabled(ui->extensionList->selectionModel()->selectedIndexes().size() > 0);
    ui->databaseList->setEnabled(ui->selectedDatabasesRadio->isChecked());
}

void SqliteExtensionEditor::updateCurrentExtensionState()
{
    int srcRow = getCurrentExtensionRow();
    bool validRow = model->isValidRowIndex(srcRow);
    ui->rightWidget->setEnabled(validRow);
    if (!validRow)
    {
        setValidState(ui->fileEdit, true);
        setValidState(ui->initEdit, true);
        return;
    }

    bool fileOk = true;
    bool initOk = true;
    QString fileError;
    bool allOk = validateExtension(&fileOk, &initOk, &fileError);

    // Display results
    setValidState(ui->fileEdit, fileOk, fileError);
    setValidState(ui->initEdit, initOk, tr("Invalid initialization function name. Function name can contain only alpha-numeric characters and underscore."));
    ui->databasesGroup->setEnabled(allOk);
    model->setValid(srcRow, allOk);

    updateState();
}

void SqliteExtensionEditor::extensionSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    int deselCnt = deselected.indexes().size();
    int selCnt = selected.indexes().size();

    if (deselCnt > 0)
        extensionDeselected(extRowToSrc(deselected.indexes().first()).row());

    if (selCnt > 0)
        extensionSelected(extRowToSrc(selected.indexes().first()).row());

    if (deselCnt > 0 && selCnt == 0)
    {
        currentModified = false;
        clearEdits();
    }
}

void SqliteExtensionEditor::updateModified()
{
    if (updatesForSelection)
        return;

    int srcRow = getCurrentExtensionRow();
    if (model->isValidRowIndex(srcRow))
    {
        bool fileDiff = model->getFilePath(srcRow) != ui->fileEdit->text();
        bool initDiff = model->getInitFunction(srcRow) != ui->initEdit->text();
        bool allDatabasesDiff = model->getAllDatabases(srcRow) != ui->allDatabasesRadio->isChecked();
        bool dbDiff = toSet(getCurrentDatabases()) != toSet(model->getDatabases(srcRow)); // QSet to ignore order

        currentModified = (fileDiff || initDiff || allDatabasesDiff || dbDiff);
    }

    statusUpdateTrigger->schedule();
}

void SqliteExtensionEditor::generateName()
{
    if (!nameGenerationActive)
        return;

    int srcRow = getCurrentExtensionRow();
    if (model->isValidRowIndex(srcRow))
        model->setName(srcRow, QFileInfo(ui->fileEdit->text()).baseName());
}

void SqliteExtensionEditor::applyFilter(const QString& value)
{
    int srcRow = getCurrentExtensionRow();
    ui->extensionList->selectionModel()->clearSelection();

    extensionFilterModel->setFilterFixedString(value);

    selectExtension(srcRow);
}

void SqliteExtensionEditor::browseForFile()
{
    QString dir = getFileDialogInitPath();
    QString filter =
#if defined(Q_OS_WIN)
            tr("Dynamic link libraries (*.dll);;All files (*)");
#elif defined(Q_OS_LINUX)
            tr("Shared objects (*.so);;All files (*)");
#elif defined(Q_OS_OSX)
            tr("Dynamic libraries (*.dylib);;All files (*)");
#else
            tr("All files (*)");
#endif
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), dir, filter);
    if (filePath.isNull())
        return;

    setFileDialogInitPathByFile(filePath);

    ui->fileEdit->setText(filePath);
}
