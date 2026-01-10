#include "colorpickerpopup.h"
#include "iconmanager.h"
#include "common/global.h"
#include "uiconfig.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QToolButton>
#include <QColorDialog>
#include <QMenu>
#include <QSignalMapper>
#include <QRegularExpression>

QVector<QColor> ColorPickerPopup::baseColors = {
    // Grayscale
    QColor(QRgb(0x000000)),
    QColor(QRgb(0x434343)),
    QColor(QRgb(0x666666)),
    QColor(QRgb(0x999999)),
    QColor(QRgb(0xB7B7B7)),
    QColor(QRgb(0xCCCCCC)),
    QColor(QRgb(0xD9D9D9)),
    QColor(QRgb(0xEFEFEF)),
    QColor(QRgb(0xF3F3F3)),
    QColor(QRgb(0xFFFFFF)),

    // Base colors
    QColor(QRgb(0x980000)),
    QColor(QRgb(0xFF0000)),
    QColor(QRgb(0xFF9900)),
    QColor(QRgb(0xFFFF00)),
    QColor(QRgb(0x00FF00)),
    QColor(QRgb(0x00FFFF)),
    QColor(QRgb(0x4A86E8)),
    QColor(QRgb(0x0000FF)),
    QColor(QRgb(0x9900FF)),
    QColor(QRgb(0xFF00FF))
};

ColorPickerPopup::ColorPickerPopup(QWidget *parent)
    : QWidget(parent)
{
    init();
}

void ColorPickerPopup::init()
{
    setObjectName("ColorPickerPopup");
    setAutoFillBackground(true);
    colorButtonMapper = new QSignalMapper(this);
    connect(colorButtonMapper, &QSignalMapper::mappedInt, this, &ColorPickerPopup::handleColorRgbaClick);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    auto *resetBtn = new QPushButton(ICONS.ERASE, tr("Clear"), this);
    resetBtn->setCursor(Qt::PointingHandCursor);
    connect(resetBtn, &QPushButton::clicked, this, &ColorPickerPopup::resetRequested);
    mainLayout->addWidget(resetBtn, 0, Qt::AlignLeft);

    mainLayout->addWidget(createColorGrid(baseColors, 10));

    auto *customLabel = new QLabel(tr("Custom colors"));
    customLabel->setStyleSheet("font-weight: 600;");
    mainLayout->addWidget(customLabel);

    customColorsWidget = createColorGrid(customColors, 10);
    customColorsWidget->setMinimumHeight(cellSize);
    mainLayout->addWidget(customColorsWidget);

    auto *addCustomColorBtn = new QPushButton(ICONS.PLUS, tr("Add custom color"), this);
    connect(addCustomColorBtn, &QToolButton::clicked, this, &ColorPickerPopup::pickCustomColor);
    mainLayout->addWidget(addCustomColorBtn, 0, Qt::AlignLeft);
}

ColorPickerPopup::~ColorPickerPopup()
{
}

QWidget* ColorPickerPopup::createColorGrid(const QVector<QColor>& colors, int columns)
{
    auto *w = new QWidget(this);
    auto *grid = new QGridLayout(w);
    grid->setSpacing(0);
    grid->setAlignment(Qt::AlignLeft);
    grid->setContentsMargins(0, 0, 0, 0);

    for (int i = 0; i < colors.size(); ++i)
        grid->addWidget(createColorButton(colors[i]), i / columns, i % columns);

    return w;
}

QToolButton* ColorPickerPopup::createColorButton(const QColor& color)
{
    static_qstring(styleTpl,
                   "QToolButton { background: %1; border: 2px solid transparent; border-radius: 3px; }"
                   "QToolButton:hover { border-color: palette(highlight); }");

    QToolButton* btn = new QToolButton();
    btn->setFixedSize(cellSize, cellSize);
    btn->setAutoRaise(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(styleTpl.arg(color.name()));
    btn->setToolTip(color.name(QColor::HexRgb).toUpper());
    connect(btn, &QToolButton::pressed, colorButtonMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    colorButtonMapper->setMapping(btn, static_cast<int>(color.rgb()));
    colorButtons.insert(color.rgba(), btn);
    return btn;
}

void ColorPickerPopup::markColor(const QColor& color)
{
    clearHighlight();

    auto it = colorButtons.constFind(color.rgba());
    if (it == colorButtons.constEnd())
        return;

    highlightButton(it.value());
}

void ColorPickerPopup::clearColorMark()
{
    clearHighlight();
}

void ColorPickerPopup::clearHighlight()
{
    if (!currentHighlighted)
        return;

    currentHighlighted->setStyleSheet(
        currentHighlighted->styleSheet().replace(QRegularExpression("border:[^;]+;"), "")
    );

    currentHighlighted = nullptr;
}

void ColorPickerPopup::highlightButton(QToolButton* btn)
{
    if (!btn)
        return;

    btn->setStyleSheet(btn->styleSheet() + "QToolButton { border: 2px solid palette(highlight); }");
    currentHighlighted = btn;
}

void ColorPickerPopup::staticInit()
{
    QList<QColor> refColors = baseColors.mid(10, 10);

    auto lighter = [](const QColor& c, double factor)
    {
        return QColor(
            c.red()   + (255 - c.red())   * factor,
            c.green() + (255 - c.green()) * factor,
            c.blue()  + (255 - c.blue())  * factor
        );
    };

    int idx = 10;
    for (QColor& c : refColors)
        baseColors.insert(idx++, lighter(c, 0.6));

    for (QColor& c : refColors)
        baseColors.insert(idx++, lighter(c, 0.3));

    for (QColor& c : refColors)
        baseColors.append(c.darker(140));

    for (QColor& c : refColors)
        baseColors.append(c.darker(200));

    for (QColor& c : refColors)
        baseColors.append(c.darker(300));
}

void ColorPickerPopup::addCustomColor(const QColor& c)
{
    if (customColors.contains(c))
        return;

    customColors.prepend(c);

    constexpr int maxCustom = 10;
    if (customColors.size() > maxCustom)
        customColors.removeLast();
}

QVector<QColor> ColorPickerPopup::getCustomColors()
{
    return customColors;
}

void ColorPickerPopup::setCustomColors(const QVector<QColor>& colors)
{
    customColors = colors;
    refreshCustomColors();
}

void ColorPickerPopup::refreshCustomColors()
{
    if (!customColorsWidget)
        return;

    QWidget* newWidget = createColorGrid(customColors, 10);
    auto *layout = customColorsWidget->parentWidget()->layout();
    QLayoutItem* oldItem = layout->replaceWidget(customColorsWidget, newWidget);
    oldItem->widget()->deleteLater();
    delete oldItem;
    customColorsWidget = newWidget;
}

void ColorPickerPopup::pickCustomColor()
{
    QColor c = QColorDialog::getColor(Qt::white, this, tr("Select color"));
    if (!c.isValid())
        return;

    handleColorClick(c);
}

void ColorPickerPopup::handleColorRgbaClick(int mappedColor)
{
    handleColorClick(QColor::fromRgb(static_cast<uint>(mappedColor)));
}

void ColorPickerPopup::handleColorClick(const QColor& color)
{
    addCustomColor(color);
    refreshCustomColors();
    emit colorPicked(color);
}
