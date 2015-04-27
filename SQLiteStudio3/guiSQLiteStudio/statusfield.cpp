#include "statusfield.h"
#include "ui_statusfield.h"
#include "mainwindow.h"
#include "uiconfig.h"
#include "iconmanager.h"
#include "themetuner.h"
#include "common/tablewidget.h"
#include "services/notifymanager.h"
#include <QMenu>
#include <QAction>
#include <QDateTime>
#include <QLabel>
#include <QVariantAnimation>
#include <QDebug>

StatusField::StatusField(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::StatusField)
{
    ui->setupUi(this);
    setupMenu();
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    NotifyManager* nm = NotifyManager::getInstance();
    connect(nm, SIGNAL(notifyInfo(QString)), this, SLOT(info(QString)));
    connect(nm, SIGNAL(notifyError(QString)), this, SLOT(error(QString)));
    connect(nm, SIGNAL(notifyWarning(QString)), this, SLOT(warn(QString)));
    connect(CFG_UI.Fonts.StatusField, SIGNAL(changed(QVariant)), this, SLOT(fontChanged(QVariant)));

    THEME_TUNER->manageCompactLayout(widget());

    readRecentMessages();
}

bool StatusField::hasMessages() const
{
    return ui->tableWidget->rowCount() > 0;
}

StatusField::~StatusField()
{
    delete ui;
}

void StatusField::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void StatusField::info(const QString &text)
{
    addEntry(ICONS.STATUS_INFO, text, CFG_UI.Colors.StatusFieldInfoFg.get());
}

void StatusField::warn(const QString &text)
{
    addEntry(ICONS.STATUS_WARNING, text, CFG_UI.Colors.StatusFieldWarnFg.get());
}

void StatusField::error(const QString &text)
{
    addEntry(ICONS.STATUS_ERROR, text, CFG_UI.Colors.StatusFieldErrorFg.get());
}

void StatusField::addEntry(const QIcon &icon, const QString &text, const QColor& color)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(row+1);

    if (row > itemCountLimit)
    {
        ui->tableWidget->removeRow(0);
        row--;
    }

    QList<QTableWidgetItem*> itemsCreated;
    QTableWidgetItem* item = nullptr;

    item = new QTableWidgetItem();
    item->setIcon(icon);
    ui->tableWidget->setItem(row, 0, item);
    itemsCreated << item;

    QFont font = CFG_UI.Fonts.StatusField.get();

    QString timeStr = "[" + QDateTime::currentDateTime().toString(timeStampFormat) + "]";
    item = new QTableWidgetItem(timeStr);
    item->setForeground(QBrush(color));
    item->setFont(font);
    ui->tableWidget->setItem(row, 1, item);
    itemsCreated << item;

    item = new QTableWidgetItem();
    item->setForeground(QBrush(color));
    item->setFont(font);
    ui->tableWidget->setItem(row, 2, item);
    itemsCreated << item;

    static_qstring(colorTpl, "QLabel {color: %1}");
    // While QLabel does detect if the text is rich automatically, we don't want to use qlabel for plain text,
    // because it's not wrapped correctly if the text is longer.
    if (text.contains("<"))
    {
        QLabel* label = new QLabel(text);
        QMargins margin = label->contentsMargins();
        margin.setLeft(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));
        label->setContentsMargins(margin);
        label->setFont(font);
        label->setStyleSheet(colorTpl.arg(color.name()));
        connect(label, SIGNAL(linkActivated(QString)), this, SIGNAL(linkActivated(QString)));
        ui->tableWidget->setCellWidget(row, 2, label);
    }
    else
    {
        item->setText(text);
    }

    setVisible(true);

    ui->tableWidget->scrollToBottom();

    if (!noFlashing)
        flashItems(itemsCreated, color);
}

void StatusField::flashItems(const QList<QTableWidgetItem*>& items, const QColor& color)
{
    QColor alphaColor = color;
    alphaColor.setAlpha(0);

    QColor finalColor = color;
    finalColor.setAlpha(150);

    QVariantAnimation* anim = new QVariantAnimation();
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->setStartValue(finalColor);
    anim->setEndValue(alphaColor);

    itemAnimations << anim;
    connect(anim, &QObject::destroyed, [this, anim]() {itemAnimations.removeOne(anim);});

    connect(anim, &QVariantAnimation::valueChanged, [items](const QVariant& value)
    {
        for (QTableWidgetItem* item : items)
            item->setBackground(value.value<QColor>());
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void StatusField::setupMenu()
{
    menu = new QMenu(this);

    copyAction = new QAction(ICONS.ACT_COPY, tr("Copy"), ui->tableWidget);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, ui->tableWidget, &TableWidget::copy);
    menu->addAction(copyAction);

    menu->addSeparator();

    clearAction = new QAction(ICONS.ACT_CLEAR, tr("Clear"), ui->tableWidget);
    connect(clearAction, &QAction::triggered, this, &StatusField::reset);
    menu->addAction(clearAction);

    connect(ui->tableWidget, &QWidget::customContextMenuRequested, this, &StatusField::customContextMenuRequested);
}

void StatusField::readRecentMessages()
{
    noFlashing = true;
    foreach (const QString& msg, NotifyManager::getInstance()->getRecentInfos())
        info(msg);

    foreach (const QString& msg, NotifyManager::getInstance()->getRecentWarnings())
        warn(msg);

    foreach (const QString& msg, NotifyManager::getInstance()->getRecentErrors())
        error(msg);

    noFlashing = false;
}

void StatusField::customContextMenuRequested(const QPoint &pos)
{
    copyAction->setEnabled(ui->tableWidget->selectionModel()->selectedRows().size() > 0);

    menu->popup(ui->tableWidget->mapToGlobal(pos));
}

void StatusField::reset()
{
    for (QAbstractAnimation* anim : itemAnimations)
        anim->stop();

    itemAnimations.clear();
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
}

void StatusField::fontChanged(const QVariant& variant)
{
    QFont newFont = variant.value<QFont>();
    QFont font;
    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        font = ui->tableWidget->item(row, 1)->font();
        font = newFont.resolve(font);
        for (int col = 1; col <= 2; col++)
            ui->tableWidget->item(row, col)->setFont(font);
    }
}
