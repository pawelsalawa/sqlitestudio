#ifndef ERDWINDOW_H
#define ERDWINDOW_H

#include "mdichild.h"
#include "erdeditor_global.h"
#include "scene/erdarrowitem.h"
#include "erdeditorplugin.h"
#include <QWidget>


class ColorPickerPopup;

class ExtLineEdit;
namespace Ui {
    class ErdWindow;
}

class QToolButton;
class ErdScene;
class ErdEntity;
class ErdChange;
class QShortcut;
class ErdChangeRegistry;
class QGraphicsOpacityEffect;

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
            NEW_TABLE_AT_POSITION,
            ARRANGE_FDP,
            ARRANGE_NEATO,
            ADD_CONNECTION,
            LINE_STRAIGHT,
            LINE_CURVY,
            LINE_SQUARE,
            COLOR_PICK,
            CANCEL_CURRENT,
            UNDO,
            REDO,
            DELETE_SELECTED,
            FILTER_VALUE,
            SELECT_ALL
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

        static Icon* cursorAddTableIcon;
        static Icon* cursorFkIcon;

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
        void focusItem(QGraphicsItem* item);
        void parseAndRestore();
        /**
         * @return true if panel clear operation was successful and view is allowed to deselect item.
         */
        bool clearSidePanel();
        /**
         * @return true if panel replacement operation was successful and view is allowed to change item selection.
         */
        bool setSidePanelWidget(QWidget* widget);
        /**
         * @return true if panel replacement operation was successful and view is allowed to change item selection.
         */
        bool showSidePanelPropertiesFor(QGraphicsItem* item);
        bool initMemDb();
        bool storeCurrentSidePanelModifications();
        /**
         * @return true if modifications storing operation was successful and view is allowed to change item selection.
         */
        bool storeEntityModifications(QWidget* sidePanelWidget);
        QString getCurrentSidePanelModificationsEntity() const;
        QToolButton* createSetTableColorAction();
        QToolButton* createLineStyleAction();
        void applySelectedEntityColor(const QColor& color);
        void updatePickerColorFromSelected(QList<QGraphicsItem*> selectedItems);
        void initFilter();
        void initContextMenu();

        static constexpr const char* ERD_CFG_GROUP = "ErdPluginConfig";
        static constexpr const char* CFG_KEY_SPLITTER = "splitter";
        static constexpr const char* CHANGE_COUNT_DIGITS = "%04d";
        static constexpr const char* CFG_CUSTOM_COLORS = "customColors";
        static Icon* windowIcon;
        static Icon* fdpIcon;
        static Icon* neatoIcon;
        static Icon* lineCurvyIcon;
        static Icon* lineStraightIcon;
        static Icon* lineSquareIcon;
        static Icon* colorPickerIcon;

        Ui::ErdWindow *ui;
        Db* db = nullptr;
        Db* memDb = nullptr; // a volatile copy of the db, excluding data, just schema - for immediate modifications
        ErdScene* scene = nullptr;
        ErdChangeRegistry* changeRegistry = nullptr;
        QWidget* currentSideWidget = nullptr;
        QWidget* noSideWidgetContents = nullptr;
        QGraphicsOpacityEffect* noSideWidgetEffect = nullptr;
        QToolButton* changeCountLabel = nullptr;
        bool ignoreSelectionChangeEvents = false;
        QToolButton* lineTypeButton = nullptr;
        ColorPickerPopup* colorPicker = nullptr;
        ExtLineEdit* filterEdit = nullptr;
        QTimer* filterTimer = nullptr;
        QMenu* colorPickerMenu = nullptr;
        QMenu* sceneContextMenu = nullptr;

    private slots:
        void checkIfActivated(Qt::WindowStates oldState, Qt::WindowStates newState);
        void uiPaletteChanged();
        void useStraightLine();
        void useStraightLineHotKey();
        void useCurvyLine();
        void useCurvyLineHotKey();
        void useSquareLine();
        void useSquareLineHotKey();
        void cancelCurrentAction();
        void newTableToggled(bool enable);
        void addConnectionToggled(bool enable);
        void handleDraftConnectionRemoved();
        void handleTableInsertionAborted();
        void itemSelectionChanged();
        void reloadSchema();
        void commitPendingChanges();
        void rollbackPendingChanges();
        void handleCreatedChange(ErdChange* change);
        void updateState();
        void updateToolbarState(int effectiveChangeCount, bool undoAvailable, bool redoAvailable);
        void abortSidePanel();
        void showChangeRegistry();
        void refreshSidePanel();
        void undo();
        void redo();
        void createNewEntityAt(const QPointF& pos);
        void newTableAtPosition();
        void handleEntityNameEditedInline(ErdEntity* entity, const QString& newName);
        void handleEntityFieldEditedInline(ErdEntity* entity, int colIdx, const QString& newName);
        void handleEntityFieldDeletedInline(ErdEntity* entity, int colIdx);
        void updateSelectionBasedActionsState();
        void failedChangeReEditRequested(ErdEntity* entity);
        void applyItemFiltering();
        void focusFilterInput();
        void sceneContextMenuRequested(const QPoint& pos);
        void colorResetPicked();
        void colorPicked(const QColor& color);
};

#endif // ERDWINDOW_H
