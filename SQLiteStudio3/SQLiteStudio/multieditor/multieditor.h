#ifndef MULTIEDITOR_H
#define MULTIEDITOR_H

#include "datagrid/sqlquerymodelcolumn.h"
#include <QWidget>
#include <QVariant>

class QCheckBox;
class QTabWidget;
class MultiEditorWidget;
class QLabel;

class MultiEditor : public QWidget
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
            BLOB
        };

        explicit MultiEditor(QWidget *parent = 0);

        void addEditor(BuiltInEditor editor);
        void showTab(int idx);

        void setValue(const QVariant& value);
        QVariant getValue() const;
        bool isModified() const;

        bool eventFilter(QObject* obj, QEvent* event);

        bool getReadOnly() const;
        void setReadOnly(bool value);
        void setDeletedRow(bool value);

        void setDataType(const SqlQueryModelColumn::DataType& dataType);

        static QList<MultiEditor::BuiltInEditor> getEditorTypes(const SqlQueryModelColumn::DataType& dataType);

    private:
        void init();
        void updateVisibility();
        void updateNullEffect();
        void updateValue(const QVariant& newValue);
        QVariant getValueOmmitNull() const;

        static const int margins = 2;
        static const int spacing = 2;

        QCheckBox* nullCheck;
        QTabWidget* tabs;
        QHash<BuiltInEditor,MultiEditorWidget*> builtInEditorsMap;
        QList<MultiEditorWidget*> editors;
        QLabel* stateLabel;
        bool readOnly = false;
        bool invalidatingDisabled = false;
        QGraphicsEffect* nullEffect;
        bool valueModified = false;
        QVariant valueBeforeNull;

        /**
         * @brief currentTab
         * Hold current tab index. It might seem as duplicate for tabs->currentIndex,
         * but this is necessary when we want to know what was the previous tab,
         * while being in tabChanged() slot.
         */
        int currentTab = -1;

    private slots:
        void tabChanged(int idx);
        void nullStateChanged(int state);
        void invalidateValue();
        void setModified();

    signals:
        void modified();
};

#endif // MULTIEDITOR_H
