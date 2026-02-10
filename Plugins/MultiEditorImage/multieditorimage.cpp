#include "multieditorimage.h"
#include "iconmanager.h"
#include "uiconfig.h"
#include "services/notifymanager.h"
#include "multieditor/multieditor.h"
#include <QBuffer>
#include <QImageReader>
#include <QLabel>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QToolButton>
#include <QToolBar>
#include <QFileDialog>
#include <QScrollBar>
#include <QAction>

MultiEditorImage::MultiEditorImage()
{
    setLayout(new QHBoxLayout());
    scrollArea = new QScrollArea();
    scrollArea->setBackgroundRole(QPalette::Dark);
    layout()->addWidget(scrollArea);

    QToolBar* tb = new QToolBar();
    tb->setOrientation(Qt::Vertical);
    zoomInAct = tb->addAction(ICONS.ZOOM_IN, tr("Zoom in by 25%"), this, SLOT(zoomIn()));
    zoomOutAct = tb->addAction(ICONS.ZOOM_OUT, tr("Zoom out by 25%"), this, SLOT(zoomOut()));
    tb->addAction(ICONS.ZOOM_RESET, tr("Reset zoom"), this, SLOT(resetZoom()));
    layout()->addWidget(tb);

    imgLabel = new QLabel();
    imgLabel->setBackgroundRole(QPalette::Base);
    imgLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imgLabel->setScaledContents(true);
    scrollArea->setWidget(imgLabel);
    imgLabel->show();
}

void MultiEditorImage::setValue(const QVariant &value)
{
    this->imgData = value.toByteArray();

    QPixmap imgPixmap;
    if (imgPixmap.loadFromData(this->imgData))
    {
        imgLabel->setPixmap(imgPixmap);

        QBuffer buf(&(this->imgData));
        QImageReader ir(&buf);
        imgFormat = ir.format();
    }
    else
    {
        imgLabel->clear();
        imgFormat.clear();
    }

    imgLabel->adjustSize();
}

QVariant MultiEditorImage::getValue()
{
    return imgData;
}

void MultiEditorImage::setReadOnly(bool boolValue)
{
    Q_UNUSED(boolValue);
}

QList<QWidget*> MultiEditorImage::getNoScrollWidgets()
{
    QList<QWidget*> list;
    list << scrollArea << imgLabel;
    return list;
}

void MultiEditorImage::focusThisWidget()
{
}

QString MultiEditorImage::getPreferredFileFilter()
{
    return MultiEditor::getFileDialogFilter(MultiEditor::IMAGES);
}

void MultiEditorImage::scale(double factor)
{
    currentZoom *= factor;
    imgLabel->resize(currentZoom * imgLabel->pixmap(Qt::ReturnByValue).size());
    zoomInAct->setEnabled(currentZoom < 10.0);
    zoomOutAct->setEnabled(currentZoom > 0.1);
}

void MultiEditorImage::zoomIn()
{
    scale(1.25);
}

void MultiEditorImage::zoomOut()
{
    scale(0.8);
}

void MultiEditorImage::resetZoom()
{
    imgLabel->adjustSize();
    currentZoom = 1.0;
}

MultiEditorWidget* MultiEditorImagePlugin::getInstance()
{
    MultiEditorImage* instance = new MultiEditorImage();
    instances << instance;
    connect(instance, &QObject::destroyed, [this, instance]()
    {
       instances.removeOne(instance);
    });
    return instance;
}

bool MultiEditorImagePlugin::validFor(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BLOB:
        case DataType::ANY:
        case DataType::NONE:
        case DataType::unknown:
            return true;
        case DataType::BOOLEAN:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::STRING:
        case DataType::TEXT:
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::TIME:
            break;
    }
    return false;
}

int MultiEditorImagePlugin::getPriority(const QVariant& value, const DataType& dataType)
{
    if (value.userType() == QMetaType::QByteArray)
    {
        QByteArray bytes = value.toByteArray();
        if (bytes.size() >= 32)
        {
            QBuffer buffer;
            buffer.setData(bytes);
            buffer.open(QIODevice::ReadOnly);

            QImageReader reader(&buffer);
            bool isImg = reader.canRead();
            buffer.close();
            if (isImg)
                return 1; // High priority for valid images
        }
    }

    switch (dataType.getType())
    {
        case DataType::BLOB:
            return 10;
        case DataType::NONE:
        case DataType::ANY:
        case DataType::unknown:
            return 50;
        case DataType::BOOLEAN:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::STRING:
        case DataType::TEXT:
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::TIME:
            break;
    }
    return 100;
}

QString MultiEditorImagePlugin::getTabLabel()
{
    return tr("Image");
}

bool MultiEditorImagePlugin::init()
{
    SQLS_INIT_RESOURCE(multieditorimage);
    return GenericPlugin::init();
}

void MultiEditorImagePlugin::deinit()
{
    for (MultiEditorImage*& editor : instances)
    {
        editor->notifyAboutUnload();
        delete editor;
    }

    SQLS_CLEANUP_RESOURCE(multieditorimage);
}
