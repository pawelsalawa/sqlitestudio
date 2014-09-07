#ifndef extaction_H
#define extaction_H

#include "guiSQLiteStudio_global.h"
#include <QAction>

class GUI_API_EXPORT ExtAction : public QAction
{
        Q_OBJECT

    public:
        explicit ExtAction(QObject *parent = 0);
        ExtAction(const QString& text, QObject* parent = 0);
        ExtAction(const QIcon& icon, const QString& text, QObject* parent = 0);

    protected:
        bool event(QEvent* e);
};

#endif // extaction_H
