#include "multieditortext.h"
#include "common/unused.h"
#include "iconmanager.h"
#include <QPlainTextEdit>
#include <QVariant>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>
#include <QDebug>

CFG_KEYS_DEFINE(MultiEditorText)

MultiEditorText::MultiEditorText(QWidget *parent) :
    MultiEditorWidget(parent)
{
    setLayout(new QVBoxLayout());
    textEdit = new QPlainTextEdit();
    layout()->addWidget(textEdit);
    initActions();
    setupMenu();

    setFocusProxy(textEdit);
    textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    textEdit->setTabChangesFocus(true);

    connect(textEdit, &QPlainTextEdit::modificationChanged, this, &MultiEditorText::modificationChanged);
    connect(textEdit, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showCustomMenu(QPoint)));
}

void MultiEditorText::setValue(const QVariant& value)
{
    textEdit->setPlainText(value.toString());
}

QVariant MultiEditorText::getValue()
{
    return textEdit->toPlainText();
}

void MultiEditorText::setReadOnly(bool value)
{
    textEdit->setReadOnly(value);
}

QToolBar* MultiEditorText::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return nullptr;
}

void MultiEditorText::focusThisWidget()
{
    textEdit->setFocus();
}

QList<QWidget*> MultiEditorText::getNoScrollWidgets()
{
    // We don't return text, we want it to be scrolled.
    QList<QWidget*> list;
    return list;
}

void MultiEditorText::modificationChanged(bool changed)
{
    if (changed)
        emit valueModified();
}

void MultiEditorText::deleteSelected()
{
    textEdit->textCursor().removeSelectedText();
}

void MultiEditorText::showCustomMenu(const QPoint& point)
{
    contextMenu->popup(textEdit->mapToGlobal(point));
}

void MultiEditorText::updateUndoAction(bool enabled)
{
    actionMap[UNDO]->setEnabled(enabled);
}

void MultiEditorText::updateRedoAction(bool enabled)
{
    actionMap[REDO]->setEnabled(enabled);
}

void MultiEditorText::updateCopyAction(bool enabled)
{
    actionMap[CUT]->setEnabled(enabled);
    actionMap[COPY]->setEnabled(enabled);
    actionMap[DELETE]->setEnabled(enabled);
}

void MultiEditorText::toggleTabFocus()
{
    textEdit->setTabChangesFocus(actionMap[TAB_CHANGES_FOCUS]->isChecked());
}

void MultiEditorText::createActions()
{
    createAction(TAB_CHANGES_FOCUS, tr("Tab changes focus"), this, SLOT(toggleTabFocus()), this);
    createAction(CUT, ICONS.ACT_CUT, tr("Cut"), textEdit, SLOT(cut()), this);
    createAction(COPY, ICONS.ACT_COPY, tr("Copy"), textEdit, SLOT(copy()), this);
    createAction(PASTE, ICONS.ACT_PASTE, tr("Paste"), textEdit, SLOT(paste()), this);
    createAction(DELETE, ICONS.ACT_DELETE, tr("Delete"), this, SLOT(deleteSelected()), this);
    createAction(UNDO, ICONS.ACT_UNDO, tr("Undo"), textEdit, SLOT(undo()), this);
    createAction(REDO, ICONS.ACT_REDO, tr("Redo"), textEdit, SLOT(redo()), this);

    actionMap[CUT]->setEnabled(false);
    actionMap[COPY]->setEnabled(false);
    actionMap[DELETE]->setEnabled(false);
    actionMap[UNDO]->setEnabled(false);
    actionMap[REDO]->setEnabled(false);

    actionMap[TAB_CHANGES_FOCUS]->setCheckable(true);
    actionMap[TAB_CHANGES_FOCUS]->setChecked(true);

    connect(textEdit, &QPlainTextEdit::undoAvailable, this, &MultiEditorText::updateUndoAction);
    connect(textEdit, &QPlainTextEdit::redoAvailable, this, &MultiEditorText::updateRedoAction);
    connect(textEdit, &QPlainTextEdit::copyAvailable, this, &MultiEditorText::updateCopyAction);
}

void MultiEditorText::setupDefShortcuts()
{
    BIND_SHORTCUTS(MultiEditorText, Action);
}

void MultiEditorText::setupMenu()
{
    contextMenu = new QMenu(this);
    contextMenu->addAction(actionMap[TAB_CHANGES_FOCUS]);
    contextMenu->addSeparator();
    contextMenu->addAction(actionMap[UNDO]);
    contextMenu->addAction(actionMap[REDO]);
    contextMenu->addSeparator();
    contextMenu->addAction(actionMap[CUT]);
    contextMenu->addAction(actionMap[COPY]);
    contextMenu->addAction(actionMap[PASTE]);
    contextMenu->addAction(actionMap[DELETE]);
}

MultiEditorWidget* MultiEditorTextPlugin::getInstance()
{
    return new MultiEditorText();
}

bool MultiEditorTextPlugin::validFor(const DataType& dataType)
{
    UNUSED(dataType);
    return true;
}

int MultiEditorTextPlugin::getPriority(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BLOB:
        case DataType::BOOLEAN:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::TIME:
            return 10;
        case DataType::NONE:
        case DataType::STRING:
        case DataType::TEXT:
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::ANY:
        case DataType::unknown:
            break;
    }
    return 1;
}

QString MultiEditorTextPlugin::getTabLabel()
{
    return tr("Text");
}
