#include "formmanager.h"
#include "services/config.h"
#include "services/pluginmanager.h"
#include "sqlitestudio.h"
#include "uiloader.h"
#include "common/configradiobutton.h"
#include "common/fileedit.h"
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QApplication>
#include <QDebug>

FormManager::FormManager()
{
    init();
}

FormManager::~FormManager()
{
    if (uiLoader)
    {
        delete uiLoader;
        uiLoader = nullptr;
    }
}

QWidget* FormManager::createWidget(const QString& name)
{
    if (!widgetNameToFullPath.contains(name))
    {
        qCritical() << "Asked for widget name which isn't managed by FormManager:" << name << ", while available widgets are:"
                    << widgetNameToFullPath.keys();
        return nullptr;
    }
    return createWidgetByFullPath(widgetNameToFullPath[name]);
}

bool FormManager::hasWidget(const QString& name)
{
    return widgetNameToFullPath.contains(name);
}

QStringList FormManager::getAvailableForms() const
{
    return widgetNameToFullPath.keys();
}

QWidget* FormManager::createWidgetByFullPath(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "FormManager was unable to open ui file:" << path;
        return nullptr;
    }

    QWidget* widget = uiLoader->load(&file);
    if (!widget)
    {
        qCritical() << "Error occured while loading ui file:" << path << ". Error message: "
                    << uiLoader->errorString();
        return nullptr;
    }

    return widget;
}

void FormManager::init()
{
    initUiLoader();

    QStringList dirs;
    dirs += qApp->applicationDirPath() + "/forms";
    dirs += QDir(CFG->getConfigDir()).absoluteFilePath("forms");

    QString envDirs = SQLITESTUDIO->getEnv("SQLITESTUDIO_FORMS");
    if (!envDirs.isNull())
        dirs += envDirs.split(PATH_LIST_SEPARATOR);

    dirs += PLUGINS->getPluginDirs();

#ifdef FORMS_DIR
    dirs += FORMS_DIR;
#endif

    foreach (QString dirPath, dirs)
    {
        loadRecurently(dirPath, "");
    }
}

void FormManager::initUiLoader()
{
    uiLoader = new UiLoader();
    REGISTER_WIDGET(uiLoader, ConfigRadioButton);
    REGISTER_WIDGET(uiLoader, FileEdit);
}

void FormManager::loadRecurently(const QString& path, const QString& prefix)
{
    static const QStringList fileExtensions = {"*.ui", "*.UI"};

    QDir dir(path);
    QString fullPath;
    QString widgetName;
    foreach (QFileInfo entry, dir.entryInfoList(fileExtensions, QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot|QDir::Readable))
    {
        fullPath = entry.absoluteFilePath();
        if (entry.isDir())
        {
            loadRecurently(fullPath, prefix+entry.fileName()+"_");
            continue;
        }

        qDebug() << "Loading form file:" << fullPath;

        widgetName = getWidgetName(fullPath);
        if (widgetName.isNull())
            continue;

        if (widgetNameToFullPath.contains(widgetName))
        {
            qCritical() << "Widget named" << widgetName << "was already loaded by FormManager from file" << widgetNameToFullPath[widgetName]
                        << "therefore file" << fullPath << "will be ignored";
            continue;
        }

        widgetNameToFullPath[widgetName] = fullPath;
    }
}

QString FormManager::getWidgetName(const QString& path)
{
    static const QRegularExpression re(R"(<widget class\=\"\w+\" name\=\"(\w+)\">)");

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Could not open" << path << "for reading. Form file ignored.";
        return QString::null;
    }

    QString contents = file.readAll();
    file.close();

    QRegularExpressionMatch match = re.match(contents);
    if (!match.hasMatch())
    {
        qWarning() << "Could not match widget in" << path << " document. File ignored.";
        return QString::null;
    }

    QString widgetName = match.captured(1);

    return widgetName;
}
