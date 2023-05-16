#ifndef ERDWINDOW_H
#define ERDWINDOW_H

#include "mdichild.h"
#include "erdeditor_global.h"
#include <QWidget>

namespace Ui {
    class ErdWindow;
}

class ErdScene;
class ErdEntity;

class ERDEDITORSHARED_EXPORT ErdWindow : public MdiChild
{
    Q_OBJECT

    public:
        enum Action
        {
            NEW_TABLE,
            ARRANGE_FDP,
            ARRANGE_NEATO,
            ADD_CONNECTION,
            LINE_STRAIGHT,
            LINE_CURVY
        };
        Q_ENUM(Action)

        explicit ErdWindow();
        ErdWindow(QWidget *parent, Db* db);
        ErdWindow(const ErdWindow& other);
        ~ErdWindow();

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

        Ui::ErdWindow *ui;
        Db* db = nullptr;
        Icon* windowIcon = nullptr;
        Icon* fdpIcon = nullptr;
        Icon* neatoIcon = nullptr;
        Icon* connectionIcon = nullptr;
        Icon* lineCurvyIcon = nullptr;
        Icon* lineStraightIcon = nullptr;
        ErdScene* scene = nullptr;

    private slots:
        void checkIfActivated(Qt::WindowStates oldState, Qt::WindowStates newState);
        void uiPaletteChanged();
        void addFk();
        void useStraightLine();
        void useCurvyLine();
};

#endif // ERDWINDOW_H
