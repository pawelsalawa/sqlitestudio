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
class ErdChange;
class QShortcut;
class ErdChangeRegistry;

class ERDEDITORSHARED_EXPORT ErdWindow : public MdiChild
{
    Q_OBJECT

    public:
        enum Action
        {
            RELOAD,
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
        static void staticCleanup();

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
        bool initMemDb();

        static constexpr const char* ERD_CFG_GROUP = "ErdPluginConfig";
        static constexpr const char* CFG_KEY_SPLITTER = "splitter";
        static Icon* windowIcon;
        static Icon* fdpIcon;
        static Icon* neatoIcon;
        static Icon* lineCurvyIcon;
        static Icon* lineStraightIcon;
        static Icon* lineSquareIcon;

        Ui::ErdWindow *ui;
        Db* db = nullptr;
        Db* memDb = nullptr; // a volatile copy of the db, excluding data, just schema - for immediate modifications
        ErdScene* scene = nullptr;
        QShortcut* escHotkey = nullptr;
        ErdChangeRegistry* changeRegistry = nullptr;
        QWidget* currentSideWidget = nullptr;
        QWidget* noSideWidgetContents = nullptr;
        
    private slots:
        void checkIfActivated(Qt::WindowStates oldState, Qt::WindowStates newState);
        void uiPaletteChanged();
        void useStraightLine();
        void useCurvyLine();
        void useSquareLine();
        void cancelCurrentAction();
        void newTable();
        void itemSelectionChanged();
        void reloadSchema();
        void commitPendingChanges();
        void rollbackPendingChanges();
        void handleCreatedChange(ErdChange* change);
        void updateState();
        void updateToolbarState(int effectiveChangeCount);
};

#endif // ERDWINDOW_H