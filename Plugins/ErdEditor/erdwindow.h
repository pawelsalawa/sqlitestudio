#ifndef ERDWINDOW_H
#define ERDWINDOW_H

#include "mdichild.h"
#include "erdeditor_global.h"
#include "erdarrowitem.h"
#include <QWidget>

namespace Ui {
    class ErdWindow;
}

class ErdScene;
class ErdEntity;
class QShortcut;
class ErdChangeRegistry;

class ERDEDITORSHARED_EXPORT ErdWindow : public MdiChild
{
    Q_OBJECT

    public:
        enum Action
        {
            COMMIT,
            ROLLBACK,
            NEW_TABLE,
            ARRANGE_FDP,
            ARRANGE_NEATO,
            ADD_CONNECTION,
            LINE_STRAIGHT,
            LINE_CURVY,
            LINE_SQUARE,
            CANCEL_CURRENT
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
        void applyArrowType(ErdArrowItem::Type arrowType);
        void updateArrowTypeButtons();
        bool tryToApplyConfig(const QVariant& value, const QSet<QString>& tableNames);
        void selectItem(QGraphicsItem* item);
        void parseAndRestore();
        void clearSidePanel();
        void setSidePanelWidget(QWidget* widget);
        void showSidePanelPropertiesFor(QGraphicsItem* item);

        static constexpr const char* ERD_CFG_GROUP = "ErdPluginConfig";

        Ui::ErdWindow *ui;
        Db* db = nullptr;
        Db* memDb = nullptr; // a volatile copy of the db, excluding data, just schema - for immediate modifications
        Icon* windowIcon = nullptr;
        Icon* fdpIcon = nullptr;
        Icon* neatoIcon = nullptr;
        Icon* lineCurvyIcon = nullptr;
        Icon* lineStraightIcon = nullptr;
        Icon* lineSquareIcon = nullptr;
        ErdScene* scene = nullptr;
        QShortcut* escHotkey = nullptr;
        ErdChangeRegistry* changeRegistry = nullptr;
        QWidget* currentSideWidget = nullptr;
        QWidget* noSideWidgetContents = nullptr;

        bool initMemDb();

    private slots:
        void checkIfActivated(Qt::WindowStates oldState, Qt::WindowStates newState);
        void uiPaletteChanged();
        void useStraightLine();
        void useCurvyLine();
        void useSquareLine();
        void cancelCurrentAction();
        void newTable();
        void itemSelectionChanged();
        void commitPendingChanges();
        void rollbackPendingChanges();
};

#endif // ERDWINDOW_H
