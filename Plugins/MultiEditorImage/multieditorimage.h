#ifndef MULTIEDITORIMAGE_H
#define MULTIEDITORIMAGE_H

#include "multieditor/multieditorwidget.h"
#include "multieditor/multieditorwidgetplugin.h"
#include "multieditorimage_global.h"
#include "datatype.h"
#include "plugins/genericplugin.h"
#include <QVariant>

class QLabel;
class QScrollArea;
class QScrollBar;

class MultiEditorImage : public MultiEditorWidget
{
    Q_OBJECT

    public:
        MultiEditorImage();

        void setValue(const QVariant& value);
        QVariant getValue();
        void setReadOnly(bool boolValue);
        QList<QWidget*> getNoScrollWidgets();
        void focusThisWidget();
        void notifyAboutUnload();
        QString getPreferredFileFilter();

    private:
        void scale(double factor);

        QByteArray imgData;
        QByteArray imgFormat;
        QScrollArea* scrollArea = nullptr;
        QLabel* imgLabel = nullptr;
        QAction* zoomInAct = nullptr;
        QAction* zoomOutAct = nullptr;
        double currentZoom = 1.0;

    private slots:
        void zoomIn();
        void zoomOut();
        void resetZoom();
};

class MULTIEDITORIMAGE_EXPORT MultiEditorImagePlugin : public GenericPlugin, public MultiEditorWidgetPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN("multieditorimage.json")

    public:
        MultiEditorWidget* getInstance();
        bool validFor(const DataType& dataType);
        int getPriority(const QVariant& value, const DataType& dataType);
        QString getTabLabel();
        bool init();
        void deinit();

    private:
        QList<MultiEditorImage*> instances;
};

#endif // MULTIEDITORIMAGE_H
