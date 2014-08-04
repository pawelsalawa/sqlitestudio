#include "debugconsole.h"
#include "ui_debugconsole.h"
#include <QPushButton>

DebugConsole::DebugConsole(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DebugConsole)
{
    ui->setupUi(this);
    ui->textEdit->setReadOnly(true);

    QPushButton* resetBtn = ui->buttonBox->button(QDialogButtonBox::Reset);
    connect(resetBtn, SIGNAL(clicked()), this, SLOT(reset()));

    initFormats();
}

DebugConsole::~DebugConsole()
{
    delete ui;
}

void DebugConsole::debug(const QString &msg)
{
    message(msg, dbgFormat);
}

void DebugConsole::warning(const QString &msg)
{
    message(msg, wrnFormat);
}

void DebugConsole::critical(const QString &msg)
{
    message(msg, criFormat);
}

void DebugConsole::fatal(const QString &msg)
{
    message(msg, fatFormat);
}

void DebugConsole::initFormats()
{
    dbgFormat.setForeground(Qt::blue);
    wrnFormat.setForeground(Qt::darkRed);
    criFormat.setForeground(Qt::red);
    criFormat.setFontUnderline(true);
    fatFormat.setForeground(Qt::red);
    fatFormat.setFontUnderline(true);

    QFontMetrics fm(ui->textEdit->font());
    int indent = fm.width(QString("X").repeated(25));
    ui->textEdit->document()->setIndentWidth(indent);

    blockFormat.setIndent(1);
    blockFormat.setTextIndent(-indent);
}

void DebugConsole::message(const QString &msg, const QTextCharFormat &format)
{
    ui->textEdit->setCurrentCharFormat(format);
    QTextCursor cur = ui->textEdit->textCursor();

    cur.insertText(msg);
    cur.mergeBlockFormat(blockFormat);
    cur.insertBlock(blockFormat);
}

void DebugConsole::reset()
{
    ui->textEdit->clear();
}
