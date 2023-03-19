#ifndef ERDEDITORWINDOW_H
#define ERDEDITORWINDOW_H

#include "mdichild.h"
#include "erdeditor_global.h"
#include <QWidget>

namespace Ui {
    class ErdEditorWindow;
}

class ERDEDITORSHARED_EXPORT ErdEditorWindow : public MdiChild
{
    Q_OBJECT

    public:
        explicit ErdEditorWindow(QWidget *parent = nullptr);
        ~ErdEditorWindow();

    private:
        Ui::ErdEditorWindow *ui;
};

#endif // ERDEDITORWINDOW_H
