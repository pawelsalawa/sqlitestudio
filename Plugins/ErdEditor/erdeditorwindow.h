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
            NEW_TABLE,
            ARRANGE_FDP,
            ARRANGE_NEATO
        };
        Q_ENUM(Action)

        explicit ErdEditorWindow();
        ErdEditorWindow(QWidget *parent, Db* db);
        ErdEditorWindow(const ErdEditorWindow& other);
        ~ErdEditorWindow();

        static void staticInit();

        bool isUncommitted() const override;
        QString getQuitUncommittedConfirmMessage() const override;
        void setMdiWindow(MdiWindow* value) override;
        bool shouldReuseForArgs(int argCount, ...) override;

    protected:
        void createActions() override;
        void setupDefShortcuts() override;
        QToolBar* getToolBar(int toolbar) const override;
        QVariant saveSession() override;
        bool restoreSession(const QVariant& sessionValue) override;
        Icon* getIconNameForMdiWindow() override;
        QString getTitleForMdiWindow() override;

    private:
        void init();

        Ui::ErdEditorWindow *ui;
        Db* db = nullptr;
        Icon* windowIcon = nullptr;
        Icon* fdpIcon = nullptr;
        Icon* neatoIcon = nullptr;
        ErdScene* scene = nullptr;

    private slots:
        void checkIfActivated(Qt::WindowStates oldState, Qt::WindowStates newState);
        void uiPaletteChanged();
};

#endif // ERDEDITORWINDOW_H
