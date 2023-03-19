#include "erdeditorwindow.h"
#include "ui_erdeditorwindow.h"

ErdEditorWindow::ErdEditorWindow(QWidget *parent) :
    MdiChild(parent),
    ui(new Ui::ErdEditorWindow)
{
    ui->setupUi(this);
}

ErdEditorWindow::~ErdEditorWindow()
{
    delete ui;
}
