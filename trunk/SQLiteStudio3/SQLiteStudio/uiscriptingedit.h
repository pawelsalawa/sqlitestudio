#ifndef UISCRIPTINGEDIT_H
#define UISCRIPTINGEDIT_H

#include "uiloaderpropertyhandler.h"
#include "common/global.h"
#include <QObject>

class QSyntaxHighlighter;

class UiScriptingEdit : public UiLoaderPropertyHandler
{
    public:
        UiScriptingEdit();

        const char* getPropertyName() const;
        void handle(QWidget* widget, const QVariant& value);

    private:
        class EditUpdater : public QObject
        {
            public:
                EditUpdater(QWidget* widget);

                bool eventFilter(QObject* obj, QEvent* e);

            private:
                void installNewHighlighter(const QVariant& prop);

                QWidget* watchedWidget;
                QString currentLang;
                QSyntaxHighlighter* currentHighlighter = nullptr;
                bool changingHighlighter = false;
        };
};

#endif // UISCRIPTINGEDIT_H
