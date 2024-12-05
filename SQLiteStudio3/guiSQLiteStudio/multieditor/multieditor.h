#ifndef MULTIEDITOR_H
#define MULTIEDITOR_H

#include "datatype.h"
#include "guiSQLiteStudio_global.h"
#include <QWidget>
#include <QVariant>

class Db;
class SqlQueryModelColumn;
class QCheckBox;
class QTabWidget;
class MultiEditorWidget;
class QLabel;
class MultiEditorWidgetPlugin;
class QToolButton;
class QMenu;

class GUI_API_EXPORT MultiEditor : public QWidget
{
        Q_OBJECT

        Q_PROPERTY(QVariant value READ getValue WRITE setValue)

    public:
        enum BuiltInEditor
        {
            TEXT,
            NUMERIC,
            BOOLEAN,
            DATE,
            TIME,
            DATETIME,
            HEX
        };

        enum TabsMode {
            CONFIGURABLE,       /**< Tabs are loaded from datatype and also have configure button visible. */
            PRECONFIGURED,      /**< Tabs are loaded from datatype. No config button is present. */
            DYNAMIC             /**< No tabs are loaded, but user has button to add new tabs, can close them and reorder them. */
        };

        explicit MultiEditor(QWidget *parent = nullptr, TabsMode tabsMode = CONFIGURABLE);

        void addEditor(MultiEditorWidget* editorWidget);
        void showTab(int idx);

        void setValue(const QVariant& value);
        QVariant getValue() const;
        bool isModified() const;
        bool eventFilter(QObject* obj, QEvent* event);
        bool getReadOnly() const;
        void setReadOnly(bool value);
        void setDeletedRow(bool value);
        void setDataType(const DataType& dataType);
        void enableFk(Db* db, SqlQueryModelColumn* column);
        void focusThisEditor();
        void setCornerLabel(const QString& label);

        template <class T>
        T* getEditorWidget() const
        {
            QListIterator<MultiEditorWidget*> it(editors);
            while (it.hasNext())
            {
                T* casted = dynamic_cast<T*>(it.next());
                if (casted)
                    return casted;
            }
            return nullptr;
        }

        static void loadBuiltInEditors();

    private:
        void init(TabsMode tabsMode);
        void updateVisibility();
        void updateNullEffect();
        void updateValue(const QVariant& newValue);
        void setValueToWidget(MultiEditorWidget* editorWidget, const QVariant& newValue);
        void updateLabel();
        QVariant getValueOmmitNull() const;
        void initAddTabMenu();
        void addPluginToMenu(MultiEditorWidgetPlugin* plugin);
        void sortAddTabMenu();

        static QList<MultiEditorWidget*> getEditorTypes(const DataType& dataType);

        static const int margins = 2;
        static const int spacing = 2;

        QLabel* cornerLabel = nullptr;
        QCheckBox* nullCheck = nullptr;
        QTabWidget* tabs = nullptr;
        QList<MultiEditorWidget*> editors;
        QLabel* stateLabel = nullptr;
        bool readOnly = false;
        bool deleted = false;
        bool invalidatingDisabled = false;
        QGraphicsEffect* nullEffect = nullptr;
        bool valueModified = false;
        QVariant valueBeforeNull;
        QToolButton* configBtn = nullptr;
        QToolButton* addTabBtn = nullptr;
        QMenu* addTabMenu = nullptr;
        DataType dataType;

        /**
         * @brief currentTab
         * Hold current tab index. It might seem as duplicate for tabs->currentIndex,
         * but this is necessary when we want to know what was the previous tab,
         * while being in tabChanged() slot.
         */
        int currentTab = -1;

    private slots:
        void configClicked();
        void tabChanged(int idx);
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
        void nullStateChanged(int state);
#else
        void nullStateChanged(Qt::CheckState state);
#endif
        void invalidateValue();
        void setModified();
        void removeTab(int idx);

    signals:
        void modified();
};

#endif // MULTIEDITOR_H
