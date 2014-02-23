#include "fontedit.h"
#include "ui_fontedit.h"
#include "iconmanager.h"
#include <QDebug>
#include <QFontDialog>

FontEdit::FontEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FontEdit)
{
    init();
}

FontEdit::~FontEdit()
{
    delete ui;
}

QFont FontEdit::getFont() const
{
    return font;
}

void FontEdit::setFont(QFont arg)
{
    font = arg;
    updateFont();
}

void FontEdit::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void FontEdit::init()
{
    ui->setupUi(this);
    ui->button->setIcon(ICON("font_browse"));
    connect(ui->button, SIGNAL(clicked()), this, SLOT(browse()));
    updateFont();
}

void FontEdit::updateFont()
{
    static const QString text = "%1, %2";
    ui->label->setFont(font);
    int size = font.pointSize() > -1 ? font.pointSize() : font.pixelSize();
    ui->label->setText(text.arg(font.family()).arg(size));
}

void FontEdit::browse()
{
    bool ok;
    QFont newFont = QFontDialog::getFont(&ok, ui->label->font(), this, tr("Choose font", "font configuration"));
    if (!ok)
        return;

    font = newFont;
    updateFont();
    emit fontChanged(newFont);
}
