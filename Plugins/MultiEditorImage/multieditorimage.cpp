#include "multieditorimage.h"
#include "common/unused.h"
#include "iconmanager.h"
#include "uiconfig.h"
#include "services/notifymanager.h"
#include <QBuffer>
#include <QImageReader>
#include <QLabel>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QToolButton>
#include <QToolBar>
#include <QFileDialog>

MultiEditorImage::MultiEditorImage()
{
    setLayout(new QHBoxLayout());
    scrollArea = new QScrollArea();
    scrollArea->setBackgroundRole(QPalette::Dark);
    layout()->addWidget(scrollArea);

    QToolBar* tb = new QToolBar();
    tb->setOrientation(Qt::Vertical);
    loadAction = tb->addAction(ICONS.OPEN_FILE, tr("Load from file"), this, SLOT(openFile()));
    tb->addAction(ICONS.SAVE_FILE, tr("Store in a file"), this, SLOT(saveFile()));
    tb->addAction(ICONS.ZOOM_IN, tr("Zoom in by 25%"), this, SLOT(zoomIn()));
    tb->addAction(ICONS.ZOOM_OUT, tr("Zoom out by 25%"), this, SLOT(zoomOut()));
    tb->addAction(ICONS.ZOOM_RESET, tr("Reset zoom"), this, SLOT(resetZoom()));
    layout()->addWidget(tb);

    imgLabel = new QLabel();
    imgLabel->setBackgroundRole(QPalette::Base);
    imgLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
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
    loadAction->setEnabled(!boolValue);
}

QList<QWidget*> MultiEditorImage::getNoScrollWidgets()
{
    QList<QWidget*> list;
    list << scrollArea << imgLabel;
    return list;
}

QString MultiEditorImage::getTabLabel()
{
    return tr("Image");
}

void MultiEditorImage::focusThisWidget()
{
}

void MultiEditorImage::notifyAboutUnload()
{
    emit aboutToBeDeleted();
}

void MultiEditorImage::openFile()
{
    QString dir = getFileDialogInitPath();
    QString filter = tr("Images (*.jpeg *.jpg *.png *.bmp *.gif *.tiff *.jp2 *.svg *.tga *.icns *.webp *.wbmp *.mng);;All files (*)");
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open image"), dir, filter);
    if (fileName.isNull())
        return;

    setFileDialogInitPathByFile(fileName);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        notifyError(tr("Could not nopen file %1 for reading.").arg(fileName));
        return;
    }

    QByteArray newData = file.readAll();
    file.close();

    setValue(newData);
    emit valueModified();
}

void MultiEditorImage::saveFile()
{
    QString dir = getFileDialogInitPath();
    QString filter;
    QString selectedFilter;
    QString format = QString::fromLatin1(imgFormat);
    if (!format.isEmpty())
    {
        selectedFilter = QString("%1 (*.%2)").arg(format, format.toLower());
        filter = QString("%1;;%3").arg(selectedFilter, tr("All files (*)"));
    }
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"), dir, filter, selectedFilter.isEmpty() ? nullptr : &selectedFilter);
    if (fileName.isNull())
        return;

    setFileDialogInitPathByFile(fileName);

    if (!format.isEmpty() && !fileName.endsWith(format, Qt::CaseInsensitive) && imgLabel->pixmap())
    {
        if (!imgLabel->pixmap()->save(fileName))
        {
            QString requestedFormat = QFileInfo(fileName).completeSuffix();
            notifyWarn(tr("Tried to save image under different format (%1) than original (%2), "
                          "but application failed to convert it. The image with unchanged format (%3) "
                          "will be saved under the given name (%4)").arg(requestedFormat, format, format, fileName));
        }
        else
            return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        notifyError(tr("Could not nopen file %1 for writting.").arg(fileName));
        return;
    }

    if (file.write(imgData) < imgData.size())
        notifyError(tr("Could not write image into the file %1").arg(fileName));

    file.close();
}

void MultiEditorImage::zoomIn()
{

}

void MultiEditorImage::zoomOut()
{

}

void MultiEditorImage::resetZoom()
{

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

int MultiEditorImagePlugin::getPriority(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BLOB:
            return 10;
        case DataType::NONE:
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

bool MultiEditorImagePlugin::init()
{
    Q_INIT_RESOURCE(multieditorimage);
    return GenericPlugin::init();
}

void MultiEditorImagePlugin::deinit()
{
    for (MultiEditorImage* editor : instances)
    {
        editor->notifyAboutUnload();
        delete editor;
    }

    Q_CLEANUP_RESOURCE(multieditorimage);
}
