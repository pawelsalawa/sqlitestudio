#ifndef ERDEDITORWINDOW_H
#define ERDEDITORWINDOW_H

#include "mdichild.h"
#include "erdeditor_global.h"
#include <QWidget>

namespace Ui {
    class ErdEditorWindow;
}

class ErdScene;
class ErdEntity;

class ERDEDITORSHARED_EXPORT ErdEditorWindow : public MdiChild
{
    Q_OBJECT

    public:
        enum Action
        {
            NEW_TABLE
        };
        Q_ENUM(Action)

        explicit ErdEditorWindow(QWidget *parent = nullptr);
        ErdEditorWindow(const ErdEditorWindow& other);
        ~ErdEditorWindow();

        static void staticInit();

        bool isUncommitted() const;
        QString getQuitUncommittedConfirmMessage() const;

    protected:
        void createActions();
        void setupDefShortcuts();
        QToolBar* getToolBar(int toolbar) const;
        QVariant saveSession();
        bool restoreSession(const QVariant& sessionValue);
        Icon* getIconNameForMdiWindow();
        QString getTitleForMdiWindow();

    private:
        void init();

        Ui::ErdEditorWindow *ui;
        Icon* windowIcon = nullptr;
        ErdScene* scene = nullptr;
        int lastCreatedX = -600;
        QList<ErdEntity*> entities;

    private slots:
        void newTable();
};

#endif // ERDEDITORWINDOW_H
