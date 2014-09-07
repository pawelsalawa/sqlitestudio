#include "uiscriptingedit.h"
#include "common/unused.h"
#include "services/pluginmanager.h"
#include "syntaxhighlighterplugin.h"
#include "pluginservicebase.h"
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QCoreApplication>
#include <QSyntaxHighlighter>

UiScriptingEdit::UiScriptingEdit()
{
}

const char* UiScriptingEdit::getPropertyName() const
{
    return "scriptingEdit";
}

void UiScriptingEdit::handle(QWidget* widget, const QVariant& value)
{
    if (!value.toBool())
        return;

    new EditUpdater(widget); // widget becomes its parent and owns it
}

UiScriptingEdit::EditUpdater::EditUpdater(QWidget* widget) :
    QObject(widget), watchedWidget(widget)
{
    widget->installEventFilter(this);
}

bool UiScriptingEdit::EditUpdater::eventFilter(QObject* obj, QEvent* e)
{
    UNUSED(obj);
    if (changingHighlighter)
        return false;

    if (e->type() != QEvent::DynamicPropertyChange)
        return false;

    if (dynamic_cast<QDynamicPropertyChangeEvent*>(e)->propertyName() != PluginServiceBase::LANG_PROPERTY_NAME)
        return false;

    QVariant prop = watchedWidget->property(PluginServiceBase::LANG_PROPERTY_NAME);
    installNewHighlighter(prop);

    return false;
}

void UiScriptingEdit::EditUpdater::installNewHighlighter(const QVariant& prop)
{
    QString lang = prop.toString();
    if (lang == currentLang)
        return;

    // When highlighter is deleted, it causes textChanged() signal and so this method is called recurrently.
    // To avoid inifinite recursion, the changingHighlighter is used to ignore property changes during deletion
    // of the highlighter.
    changingHighlighter = true;
    safe_delete(currentHighlighter);
    currentLang = QString();
    changingHighlighter = false;

    for (SyntaxHighlighterPlugin* plugin : PLUGINS->getLoadedPlugins<SyntaxHighlighterPlugin>())
    {
        if (plugin->getLanguageName() != lang)
            continue;

        currentHighlighter = plugin->createSyntaxHighlighter(watchedWidget);
        currentLang = lang;
        break;
    }
}
