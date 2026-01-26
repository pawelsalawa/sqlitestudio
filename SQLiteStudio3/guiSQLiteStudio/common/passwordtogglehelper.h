#ifndef PASSWORDTOGGLEHELPER_H
#define PASSWORDTOGGLEHELPER_H

#include "guiSQLiteStudio_global.h"
#include <QObject>
#include <QLineEdit>
#include <QAction>

class GUI_API_EXPORT PasswordToggleHelper : public QObject
{
    Q_OBJECT

    public:
        explicit PasswordToggleHelper(QLineEdit *lineEdit);

    private:
        void updateIcon();

        QLineEdit* lineEdit = nullptr;
        QAction* toggleAction = nullptr;
        bool visible = false;

    private slots:
        void toggleEchoMode();
};

#endif // PASSWORDTOGGLEHELPER_H
