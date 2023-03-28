#include "sqleditor.h"
#include "common/mouseshortcut.h"
#include "sqlitesyntaxhighlighter.h"
#include "db/db.h"
#include "uiconfig.h"
#include "uiutils.h"
#include "services/codesnippetmanager.h"
#include "iconmanager.h"
#include "completer/completerwindow.h"
#include "completionhelper.h"
#include "common/utils_sql.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "parser/parsererror.h"
#include "common/unused.h"
#include "services/notifymanager.h"
#include "dialogs/searchtextdialog.h"
#include "dbobjectdialogs.h"
#include "searchtextlocator.h"
#include "services/codeformatter.h"
#include "style.h"
#include "dbtree/dbtreeitem.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include "dbtree/dbtreeview.h"
#include "common/lazytrigger.h"
#include "common/extaction.h"
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
#include <QStyle>

CFG_KEYS_DEFINE(SqlEditor)

QHash<SqlEditor::Action, QAction*> SqlEditor::staticActions;
bool SqlEditor::wrapWords = false;

void SqlEditor::createStaticActions()
{
    staticActions[WORD_WRAP] = new ExtAction(tr("Wrap words", "sql editor"), MainWindow::getInstance());

    staticActions[WORD_WRAP]->setCheckable(true);
    staticActions[WORD_WRAP]->setChecked(wrapWords);
    connect(staticActions[WORD_WRAP], &QAction::toggled, [=](bool value)
    {
        wrapWords = value;
        CFG_UI.General.SqlEditorWrapWords.set(value);
    });
}

void SqlEditor::staticInit()
{
    wrapWords = CFG_UI.General.SqlEditorWrapWords.get();
    createStaticActions();
}

SqlEditor::SqlEditor(QWidget *parent) :
    QPlainTextEdit(parent)
{
    init();
}

SqlEditor::~SqlEditor()
{
    if (objectsInNamedDbWatcher->isRunning())
        objectsInNamedDbWatcher->waitForFinished();

    if (queryParser)
    {
        delete queryParser;
        queryParser = nullptr;
    }
}

void SqlEditor::init()
{
    highlighter = new SqliteSyntaxHighlighter(document());
    initActions();
    setupMenu();

    objectsInNamedDbWatcher = new QFutureWatcher<QHash<QString,QStringList>>(this);
    connect(objectsInNamedDbWatcher, SIGNAL(finished()), this, SLOT(scheduleQueryParserForSchemaRefresh()));

    textLocator = new SearchTextLocator(document(), this);
    connect(textLocator, SIGNAL(found(int,int)), this, SLOT(found(int,int)));
    connect(textLocator, SIGNAL(reachedEnd()), this, SLOT(reachedEnd()));
    connect(textLocator, SIGNAL(newCursorPositionAfterAllReplaced(int)), this, SLOT(moveCursorTo(int)));

    lineNumberArea = new LineNumberArea(this);
    changeFont(CFG_UI.Fonts.SqlEditor.get());

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth()));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(textChanged()), this, SLOT(checkContentSize()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorMoved()));
    MouseShortcut::forWheel(Qt::ControlModifier, this, SLOT(fontSizeChangeRequested(int)), viewport());

    updateLineNumberAreaWidth();
    highlightCurrentCursorContext();

    completer = new CompleterWindow(this);
    connect(completer, SIGNAL(accepted()), this, SLOT(completeSelected()));
    connect(completer, SIGNAL(textTyped(QString)), this, SLOT(completerTypedText(QString)));
    connect(completer, SIGNAL(backspacePressed()), this, SLOT(completerBackspacePressed()));
    connect(completer, SIGNAL(leftPressed()), this, SLOT(completerLeftPressed()));
    connect(completer, SIGNAL(rightPressed()), this, SLOT(completerRightPressed()));

    autoCompleteTrigger = new LazyTrigger(autoCompleterDelay,
                                          [this]() -> bool {return autoCompletion && !deletionKeyPressed;},
                                          this);
    connect(autoCompleteTrigger, SIGNAL(triggered()), this, SLOT(checkForAutoCompletion()));

    queryParserTrigger = new LazyTrigger(queryParserDelay, this);
    connect(queryParserTrigger, SIGNAL(triggered()), this, SLOT(parseContents()));

    connect(this, SIGNAL(textChanged()), this, SLOT(scheduleQueryParser()));

    queryParser = new Parser();

    connect(this, &QWidget::customContextMenuRequested, this, &SqlEditor::customContextMenuRequested);
    connect(CFG_UI.Fonts.SqlEditor, SIGNAL(changed(QVariant)), this, SLOT(changeFont(QVariant)));
    connect(CFG, SIGNAL(massSaveCommitted()), this, SLOT(configModified()));
}

void SqlEditor::removeErrorMarkers()
{
    highlighter->clearErrors();
}

bool SqlEditor::haveErrors()
{
    return highlighter->haveErrors();
}

bool SqlEditor::isSyntaxChecked()
{
    return syntaxValidated;
}

void SqlEditor::markErrorAt(int start, int end, bool limitedDamage)
{
    highlighter->addError(start, end, limitedDamage);
}

void SqlEditor::createActions()
{
    createAction(CUT, ICONS.ACT_CUT, tr("Cut", "sql editor"), this, SLOT(cut()), this);
    createAction(COPY, ICONS.ACT_COPY, tr("Copy", "sql editor"), this, SLOT(copy()), this);
    createAction(PASTE, ICONS.ACT_PASTE, tr("Paste", "sql editor"), this, SLOT(paste()), this);
    createAction(DELETE, ICONS.ACT_DELETE, tr("Delete", "sql editor"), this, SLOT(deleteSelected()), this);
    createAction(SELECT_ALL, ICONS.ACT_SELECT_ALL, tr("Select all", "sql editor"), this, SLOT(selectAll()), this);
    createAction(UNDO, ICONS.ACT_UNDO, tr("Undo", "sql editor"), this, SLOT(undo()), this);
    createAction(REDO, ICONS.ACT_REDO, tr("Redo", "sql editor"), this, SLOT(redo()), this);
    createAction(COMPLETE, ICONS.COMPLETE, tr("Complete", "sql editor"), this, SLOT(complete()), this);
    createAction(FORMAT_SQL, ICONS.FORMAT_SQL, tr("Format SQL", "sql editor"), this, SLOT(formatSql()), this);
    createAction(SAVE_SQL_FILE, ICONS.SAVE_SQL_FILE, tr("Save SQL to file", "sql editor"), this, SLOT(saveToFile()), this);
    createAction(SAVE_AS_SQL_FILE, ICONS.SAVE_SQL_FILE, tr("Select file to save SQL", "sql editor"), this, SLOT(saveAsToFile()), this);
    createAction(OPEN_SQL_FILE, ICONS.OPEN_SQL_FILE, tr("Load SQL from file", "sql editor"), this, SLOT(loadFromFile()), this);
    createAction(DELETE_LINE, ICONS.ACT_DEL_LINE, tr("Delete line", "sql editor"), this, SLOT(deleteLine()), this);
    createAction(MOVE_BLOCK_DOWN, tr("Move block down", "sql editor"), this, SLOT(moveBlockDown()), this);
    createAction(MOVE_BLOCK_UP, tr("Move block up", "sql editor"), this, SLOT(moveBlockUp()), this);
    createAction(COPY_BLOCK_DOWN, tr("Copy block down", "sql editor"), this, SLOT(copyBlockDown()), this);
    createAction(COPY_BLOCK_UP, tr("Copy up down", "sql editor"), this, SLOT(copyBlockUp()), this);
    createAction(FIND, ICONS.SEARCH, tr("Find", "sql editor"), this, SLOT(find()), this);
    createAction(FIND_NEXT, tr("Find next", "sql editor"), this, SLOT(findNext()), this);
    createAction(FIND_PREV, tr("Find previous", "sql editor"), this, SLOT(findPrevious()), this);
    createAction(REPLACE, ICONS.SEARCH_AND_REPLACE, tr("Replace", "sql editor"), this, SLOT(replace()), this);
    createAction(TOGGLE_COMMENT, tr("Toggle comment", "sql editor"), this, SLOT(toggleComment()), this);
    createAction(INCR_FONT_SIZE, tr("Increase font size", "sql editor"), this, SLOT(incrFontSize()), this);
    createAction(DECR_FONT_SIZE, tr("Decrease font size", "sql editor"), this, SLOT(decrFontSize()), this);

    actionMap[CUT]->setEnabled(false);
    actionMap[COPY]->setEnabled(false);
    actionMap[UNDO]->setEnabled(false);
    actionMap[REDO]->setEnabled(false);
    actionMap[DELETE]->setEnabled(false);

    connect(this, &QPlainTextEdit::undoAvailable, this, &SqlEditor::updateUndoAction);
    connect(this, &QPlainTextEdit::redoAvailable, this, &SqlEditor::updateRedoAction);
    connect(this, &QPlainTextEdit::copyAvailable, this, &SqlEditor::updateCopyAction);

    connect(CFG_UI.General.SqlEditorWrapWords, SIGNAL(changed(QVariant)), this, SLOT(wordWrappingChanged(QVariant)));
}

void SqlEditor::setupDefShortcuts()
{
    setShortcutContext({CUT, COPY, PASTE, DELETE, SELECT_ALL, UNDO, REDO, COMPLETE, FORMAT_SQL, SAVE_SQL_FILE, OPEN_SQL_FILE,
                        DELETE_LINE, INCR_FONT_SIZE, DECR_FONT_SIZE}, Qt::WidgetWithChildrenShortcut);

    BIND_SHORTCUTS(SqlEditor, Action);
}

void SqlEditor::setupMenu()
{
    contextMenu = new QMenu(this);
    contextMenu->addAction(actionMap[FORMAT_SQL]);
    contextMenu->addAction(staticActions[WORD_WRAP]);
    contextMenu->addSeparator();
    contextMenu->addAction(actionMap[SAVE_SQL_FILE]);
    contextMenu->addAction(actionMap[OPEN_SQL_FILE]);
    contextMenu->addSeparator();
    contextMenu->addAction(actionMap[UNDO]);
    contextMenu->addAction(actionMap[REDO]);
    contextMenu->addSeparator();
    contextMenu->addAction(actionMap[FIND]);
    contextMenu->addAction(actionMap[CUT]);
    contextMenu->addAction(actionMap[COPY]);
    contextMenu->addAction(actionMap[PASTE]);
    contextMenu->addAction(actionMap[DELETE]);
    contextMenu->addSeparator();
    contextMenu->addAction(actionMap[SELECT_ALL]);

    validObjContextMenu = new QMenu(this);
}

Db* SqlEditor::getDb() const
{
    return db;
}

void SqlEditor::setDb(Db* value)
{
    db = value;
    refreshValidObjects();
    scheduleQueryParser(true, true);
}

void SqlEditor::setAutoCompletion(bool enabled)
{
    autoCompletion = enabled;
}

void SqlEditor::customContextMenuRequested(const QPoint &pos)
{
    if (objectLinksEnabled && handleValidObjectContextMenu(pos))
        return;

    contextMenu->popup(mapToGlobal(pos));
}

bool SqlEditor::handleValidObjectContextMenu(const QPoint& pos)
{
    const DbObject* obj = getValidObjectForPosition(pos);
    if (!obj)
        return false;

    QString objName = stripObjName(toPlainText().mid(obj->from, (obj->to - obj->from + 1)));

    validObjContextMenu->clear();

    DbTreeItem* item = nullptr;
    for (DbTreeItem::Type type : {DbTreeItem::Type::TABLE, DbTreeItem::Type::INDEX, DbTreeItem::Type::TRIGGER, DbTreeItem::Type::VIEW})
    {
        item = DBTREE->getModel()->findItem(type, objName);
        if (item)
            break;
    }

    if (!item)
        return false;

    DBTREE->setSelectedItem(item);
    DBTREE->setupActionsForMenu(item, validObjContextMenu);
    if (validObjContextMenu->actions().size() == 0)
        return false;

    DBTREE->updateActionStates(item);
    validObjContextMenu->popup(mapToGlobal(pos));
    return true;
}

void SqlEditor::saveToFile(const QString &fileName)
{
    if (!openSaveActionsEnabled)
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        notifyError(tr("Could not open file '%1' for writing: %2").arg(fileName, file.errorString()));
        return;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << toPlainText();
    stream.flush();
    file.close();

    notifyInfo(tr("Saved SQL contents to file: %1").arg(fileName));
}

void SqlEditor::toggleLineCommentForLine(const QTextBlock& block)
{
    QTextCursor cur = textCursor();
    QString line = block.text();
    cur.setPosition(block.position());
    if (line.startsWith("--"))
    {
        cur.deleteChar();
        cur.deleteChar();
    }
    else
        cur.insertText("--");
}

bool SqlEditor::getAlwaysEnforceErrorsChecking() const
{
    return alwaysEnforceErrorsChecking;
}

void SqlEditor::setAlwaysEnforceErrorsChecking(bool newAlwaysEnforceErrorsChecking)
{
    alwaysEnforceErrorsChecking = newAlwaysEnforceErrorsChecking;
}

bool SqlEditor::getHighlightingSyntax() const
{
    return highlightingSyntax;
}

void SqlEditor::setOpenSaveActionsEnabled(bool value)
{
    openSaveActionsEnabled = value;
    if (value)
    {
        noConfigShortcutActions.remove(SAVE_SQL_FILE);
        noConfigShortcutActions.remove(SAVE_AS_SQL_FILE);
        noConfigShortcutActions.remove(OPEN_SQL_FILE);
    }
    else
        noConfigShortcutActions << SAVE_SQL_FILE << SAVE_AS_SQL_FILE << OPEN_SQL_FILE;
}

void SqlEditor::updateUndoAction(bool enabled)
{
    actionMap[UNDO]->setEnabled(enabled);
}

void SqlEditor::updateRedoAction(bool enabled)
{
    actionMap[REDO]->setEnabled(enabled);
}

void SqlEditor::updateCopyAction(bool enabled)
{
    actionMap[COPY]->setEnabled(enabled);
    actionMap[CUT]->setEnabled(enabled);
    actionMap[DELETE]->setEnabled(enabled);
}

void SqlEditor::deleteSelected()
{
    textCursor().removeSelectedText();
}

void SqlEditor::homePressed(Qt::KeyboardModifiers modifiers)
{
    QTextCursor cursor = textCursor();

    bool shift = modifiers.testFlag(Qt::ShiftModifier);
    QTextCursor::MoveMode mode = shift ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
    if (modifiers.testFlag(Qt::ControlModifier))
    {
        cursor.setPosition(0, mode);
        setTextCursor(cursor);
        return;
    }

    int curPos = cursor.positionInBlock();
    QString line = cursor.block().text();
    int firstPrintable = line.indexOf(QRegExp("\\S"));

    if (firstPrintable <= 0)
    {
        // If first printable character is the first character in line,
        // or there's no printable characters at all, move to start of line.
        cursor.movePosition(QTextCursor::StartOfLine, mode);
    }
    else if (curPos == 0 || curPos < firstPrintable)
    {
        // If cursor is at the line begining, or it's still before first printable character.
        // Move to first printable character.
        cursor.movePosition(QTextCursor::NextWord, mode);
    }
    else if (curPos == firstPrintable)
    {
        // If cursor is already at first printable character, now is the time to move it
        // to start of the line.
        cursor.movePosition(QTextCursor::StartOfLine, mode);
    }
    else
    {
        // Cursor is somewhere in the middle of printable text. Move it to the begining
        // of line and then to first printable character.
        cursor.movePosition(QTextCursor::StartOfLine, mode);
        cursor.movePosition(QTextCursor::NextWord, mode);
    }

    setTextCursor(cursor);
}

void SqlEditor::tabPressed(bool shiftPressed)
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
    {
        indentSelected(shiftPressed);
        return;
    }

    // Get current line, its first printable character
    int curPos = cursor.positionInBlock();
    QString line = cursor.block().text();
    int firstPrintable = line.indexOf(QRegExp("\\S"));

    // Handle shift+tab (unindent)
    if (shiftPressed)
    {
        cursor.movePosition(QTextCursor::StartOfLine);

        if (firstPrintable > 0)
            cursor.movePosition(QTextCursor::NextWord);

        setTextCursor(cursor);
        backspacePressed();
        return;
    }

    // If we're past any printable character (and there was any), insert a tab
    if (curPos > firstPrintable && firstPrintable >= 0)
    {
        insertPlainText("    ");
        return;
    }

    // If there is no previous block to refer to, insert a tab
    QTextBlock previousBlock = document()->findBlockByNumber(cursor.blockNumber() - 1);
    if (!previousBlock.isValid())
    {
        insertPlainText("    ");
        return;
    }

    // If previous block has first pritable character further than current cursor position, insert spaces to meet above position
    int previousFirstPrintable = previousBlock.text().indexOf(QRegExp("\\S"));
    if (curPos < previousFirstPrintable)
    {
        insertPlainText(QString(" ").repeated(previousFirstPrintable - curPos));
        return;
    }

    // At this point we know that previous block don't have first printable character further than the cursor. Insert tab.
    insertPlainText("    ");
}

void SqlEditor::backspacePressed()
{
    // If we have any selection, delete it and that's all.
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
    {
        deleteSelected();
        return;
    }

    // No selection. Collect line, cursor position, first and last printable characters in line.
    int curPos = cursor.positionInBlock();
    QString line = cursor.block().text();
    int firstPrintable = line.indexOf(QRegExp("\\S"));

    // If there is any printable character (which means that line length is greater than 0) and cursor is after first character,
    // or when cursor is at the begining of line, delete previous character, always.
    if ((firstPrintable > -1 && curPos > firstPrintable) || curPos == 0)
    {
        cursor.deletePreviousChar();
        return;
    }

    // Define number of spaces available for deletion.
    int spaces = firstPrintable;
    if (spaces < 0)
        spaces = curPos;

    // Get previous block. If there was none, then delete up to 4 previous spaces.
    QTextBlock previousBlock = document()->findBlockByNumber(cursor.blockNumber() - 1);
    if (!previousBlock.isValid())
    {
        doBackspace(spaces < 4 ? spaces : 4);
        return;
    }

    // If first printable character in previous block is prior to the current cursor position (but not first in the line),
    // delete as many spaces, as necessary to reach the same position, but never more than defined spaces number earlier.
    int previousFirstPrintable = previousBlock.text().indexOf(QRegExp("\\S"));
    if (curPos > previousFirstPrintable && previousFirstPrintable > 0)
    {
        int spacesToDelete = curPos - previousFirstPrintable;
        doBackspace(spaces < spacesToDelete ? spaces : spacesToDelete);
        return;
    }

    // There is no character to back off to, so we simply delete up to 4 previous spaces.
    doBackspace(spaces < 4 ? spaces : 4);
}

void SqlEditor::complete()
{
    if (!db || !db->isValid())
    {
        notifyWarn(tr("Syntax completion can be used only when a valid database is set for the SQL editor."));
        return;
    }

    QString sql = toPlainText();
    int curPos = textCursor().position();

    if (!virtualSqlExpression.isNull())
    {
        sql = virtualSqlExpression.arg(sql);
        curPos += virtualSqlOffset;
    }

    CompletionHelper completionHelper(sql, curPos, db);
    completionHelper.setCreateTriggerTable(createTriggerTable);
    CompletionHelper::Results result = completionHelper.getExpectedTokens();
    if (result.filtered().size() == 0)
        return;

    completer->setData(result);
    completer->setDb(db);
    if (completer->immediateResolution())
        return;

    updateCompleterPosition();
    completer->show();
}

void SqlEditor::updateCompleterPosition()
{
    QPoint pos = cursorRect().bottomRight();
    pos += QPoint(1, fontMetrics().descent());
    completer->move(mapToGlobal(pos));
}

void SqlEditor::completeSelected()
{
    if (completer->getMode() == CompleterWindow::SNIPPETS)
    {
        insertPlainText(CODESNIPPETS->getCodeByName(completer->getSnippetName()));
        return;
    }

    deletePreviousChars(completer->getNumberOfCharsToRemove());

    ExpectedTokenPtr token = completer->getSelected();
    QString value = token->value;
    if (token->needsWrapping())
        value = wrapObjIfNeeded(value);

    if (!token->prefix.isNull())
    {
        value.prepend(".");
        value.prepend(wrapObjIfNeeded(token->prefix));
    }

    insertPlainText(value);
}

void SqlEditor::checkForAutoCompletion()
{
    if (!db || !autoCompletion || deletionKeyPressed || !richFeaturesEnabled || !CFG_CORE.CodeAssistant.AutoTrigger.get())
        return;

    Lexer lexer;
    QString sql = toPlainText();
    int curPos = textCursor().position();
    TokenList tokens = lexer.tokenize(sql.left(curPos));

    if (tokens.size() > 0 && tokens.last()->type == Token::OPERATOR && tokens.last()->value == ".")
        complete();
}

void SqlEditor::deletePreviousChars(int length)
{
    QTextCursor cursor = textCursor();
    for (int i = 0; i < length; i++)
        cursor.deletePreviousChar();
}

void SqlEditor::refreshValidObjects()
{
    if (!db || !db->isValid())
        return;

    Db* dbClone = db->clone();
    QFuture<QHash<QString,QStringList>> objectsInNamedDbFuture = QtConcurrent::run([dbClone]()
    {
        dbClone->openQuiet();
        QHash<QString,QStringList> objectsByDbName;
        SchemaResolver resolver(dbClone);
        QSet<QString> databases = resolver.getDatabases();
        databases << "main";
        QStringList objects;
        for (const QString& dbName : qAsConst(databases))
        {
            objects = resolver.getAllObjects(dbName);
            objectsByDbName[dbName] << objects;
        }
        dbClone->closeQuiet();
        delete dbClone;
        return objectsByDbName;
    });
    objectsInNamedDbWatcher->setFuture(objectsInNamedDbFuture);
}

void SqlEditor::setObjectLinks(bool enabled)
{
    objectLinksEnabled = enabled;
    setMouseTracking(enabled);
    highlighter->setObjectLinksEnabled(enabled);
    highlightSyntax();

    if (enabled)
        handleValidObjectCursor(mapFromGlobal(QCursor::pos()));
    else
        viewport()->setCursor(Qt::IBeamCursor);
}

void SqlEditor::addDbObject(int from, int to, const QString& dbName)
{
    validDbObjects << DbObject(from, to, dbName);
    highlighter->addDbObject(from, to);
}

void SqlEditor::clearDbObjects()
{
    validDbObjects.clear();
    highlighter->clearDbObjects();
}

void SqlEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), STYLE->extendedPalette().editorLineNumberBase());
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(style()->standardPalette().text().color());
            painter.drawText(0, top, lineNumberArea->width()-2, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        blockNumber++;
    }
}

int SqlEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, document()->blockCount());
    while (max >= 10)
    {
        max /= 10;
        digits++;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void SqlEditor::highlightParenthesis(QList<QTextEdit::ExtraSelection>& selections)
{
    if (!richFeaturesEnabled)
        return;

    // Find out parenthesis under the cursor
    int curPos = textCursor().position();
    TextBlockData* data = dynamic_cast<TextBlockData*>(textCursor().block().userData());
    if (!data)
        return;

    const TextBlockData::Parenthesis* parOnRight = data->parenthesisForPosision(curPos);
    const TextBlockData::Parenthesis* parOnLeft = data->parenthesisForPosision(curPos - 1);
    const TextBlockData::Parenthesis* thePar = parOnRight;
    if (parOnLeft && !parOnRight) // go with parenthesis on left only when there's no parenthesis on right
        thePar = parOnLeft;

    if (!thePar)
        return;

    // Getting all parenthesis in the entire document
    QList<const TextBlockData::Parenthesis*> allParenthesis;
    for (QTextBlock block = document()->begin(); block.isValid(); block = block.next())
    {
        data = dynamic_cast<TextBlockData*>(block.userData());
        if (!data)
            continue;

        allParenthesis += data->parentheses();
    }

    // Matching the parenthesis
    const TextBlockData::Parenthesis* matchedPar = matchParenthesis(allParenthesis, thePar);
    if (!matchedPar)
        return;

    // Mark new match
    markMatchedParenthesis(thePar->position, matchedPar->position, selections);
}

void SqlEditor::highlightCurrentQuery(QList<QTextEdit::ExtraSelection>& selections)
{
    QTextCursor cursor = textCursor();
    int curPos = cursor.position();
    QString contents = cursor.document()->toPlainText();
    QPair<int,int> boundries = getQueryBoundriesForPosition(contents, curPos, true);
    if (boundries.second < 0)
        return;

    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(CFG_UI.Colors.SyntaxCurrentQueryBg.get());

    cursor.setPosition(boundries.first);
    cursor.setPosition(boundries.second, QTextCursor::KeepAnchor);
    selection.cursor = cursor;
    selections.append(selection);
}

void SqlEditor::highlightCurrentCursorContext(bool delayedCall)
{
    QList<QTextEdit::ExtraSelection> selections;
    if (delayedCall)
        highlightCurrentQuery(selections);
    else if (currentQueryTimer)
        currentQueryTimer->start();

    highlightCurrentLine(selections);
    highlightParenthesis(selections);
    setExtraSelections(selections);
}

void SqlEditor::markMatchedParenthesis(int pos1, int pos2, QList<QTextEdit::ExtraSelection>& selections)
{
    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(CFG_UI.Colors.SyntaxParenthesisBg.get());
    selection.format.setForeground(CFG_UI.Colors.SyntaxParenthesisFg.get());

    QTextCursor cursor = textCursor();

    cursor.setPosition(pos1);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;
    selections.append(selection);

    cursor.setPosition(pos2);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;
    selections.append(selection);
}

void SqlEditor::doBackspace(int repeats)
{
    QTextCursor cursor = textCursor();
    for (int i = 0; i < repeats; i++)
        cursor.deletePreviousChar();
}

void SqlEditor::indentSelected(bool shiftPressed)
{
    QTextCursor cursor = textCursor();
    QTextDocument* doc = document();
    QTextBlock startBlock = doc->findBlock(cursor.selectionStart());
    QTextBlock endBlock = doc->findBlock(cursor.selectionEnd());

    if (cursor.selectionEnd() > endBlock.position())
    {
        QTextBlock afterEndBlock = endBlock.next();
        if (afterEndBlock.isValid())
            endBlock = afterEndBlock;
    }

    for (QTextBlock it = startBlock; it != endBlock; it = it.next())
    {
        if (shiftPressed)
            unindentBlock(it);
        else
            indentBlock(it);
    }
}

void SqlEditor::indentBlock(const QTextBlock& block)
{
    QTextCursor cursor = textCursor();
    cursor.setPosition(block.position());
    cursor.insertText("    ");
}

void SqlEditor::unindentBlock(const QTextBlock& block)
{
    QString str = block.text();
    if (!str.startsWith(" "))
        return;

    int spaces = 0;
    int firstPrintable = str.indexOf(QRegExp("\\S"));
    if (firstPrintable == -1)
        spaces = str.length();
    else
        spaces = firstPrintable;

    QTextCursor cursor = textCursor();
    cursor.setPosition(block.position());
    for (int i = 0; i < 4 && i < spaces; i++)
        cursor.deleteChar();
}

void SqlEditor::indentNewLine()
{
    QTextCursor cursor = textCursor();

    // If there is no previous block to refer to, do nothing
    QTextBlock previousBlock = document()->findBlockByNumber(cursor.blockNumber() - 1);
    if (!previousBlock.isValid())
        return;

    // If previous block has first pritable character further than current cursor position, insert spaces to meet above position
    int previousFirstPrintable = previousBlock.text().indexOf(QRegExp("\\S"));
    if (previousFirstPrintable > 0)
    {
        insertPlainText(QString(" ").repeated(previousFirstPrintable));
        return;
    }

}

void SqlEditor::showSearchDialog()
{
    if (!searchDialog)
        searchDialog = new SearchTextDialog(textLocator, this);

    if (searchDialog->isVisible())
        searchDialog->hide();

    searchDialog->show();
}

const TextBlockData::Parenthesis* SqlEditor::matchParenthesis(QList<const TextBlockData::Parenthesis*> parList,
                                                              const TextBlockData::Parenthesis* thePar)
{
    bool matchLeftPar = (thePar->character == ')');
    char parToMatch = matchLeftPar ? '(' : ')';
    int parListSize = parList.size();
    int theParIdx = parList.indexOf(thePar);
    int counter = 0;
    for (int i = theParIdx; (matchLeftPar ? i >= 0 : i < parListSize); (matchLeftPar ? i-- : i++))
    {
        if (parList[i]->character == parToMatch)
            counter--;
        else
            counter++;

        if (counter == 0)
            return parList[i];
    }
    return nullptr;
}

void SqlEditor::completerTypedText(const QString& text)
{
    insertPlainText(text);
    completer->extendFilterBy(text);
    updateCompleterPosition();
}

void SqlEditor::completerBackspacePressed()
{
    deletionKeyPressed = true;
    textCursor().deletePreviousChar();
    completer->shringFilterBy(1);
    updateCompleterPosition();
    deletionKeyPressed = false;
}

void SqlEditor::completerLeftPressed()
{
    completer->shringFilterBy(1);
    moveCursor(QTextCursor::Left);
    updateCompleterPosition();
}

void SqlEditor::completerRightPressed()
{
    // Last character seems to be virtual in QPlainTextEdit, so that QTextCursor can be at its position
    int charCnt = document()->characterCount() - 1;
    int curPos = textCursor().position();

    if (curPos >= charCnt)
    {
        completer->reject();
        return;
    }

    QChar c = document()->characterAt(curPos);
    if (!c.isNull())
        completer->extendFilterBy(QString(c));

    moveCursor(QTextCursor::Right);
    updateCompleterPosition();
}

void SqlEditor::parseContents()
{
    if (!richFeaturesEnabled && !alwaysEnforceErrorsChecking)
        return;

    QString sql = toPlainText();
    if (!virtualSqlExpression.isNull())
    {
        if (virtualSqlCompleteSemicolon && !sql.trimmed().endsWith(";"))
            sql += ";";

        sql = virtualSqlExpression.arg(sql);
    }

    queryParser->parse(sql);
    if (richFeaturesEnabled)
        checkForValidObjects();

    checkForSyntaxErrors();

    if (richFeaturesEnabled)
        highlightSyntax();
}

void SqlEditor::scheduleQueryParserForSchemaRefresh()
{
    objectsInNamedDb = objectsInNamedDbWatcher->future().result();
    scheduleQueryParser(true, true);
}

void SqlEditor::checkForSyntaxErrors()
{
    syntaxValidated = true;

    removeErrorMarkers();

    // Marking invalid tokens, like in "SELECT * from test] t" - the "]" token is invalid.
    // Such tokens don't cause parser to fail.
    for (const SqliteQueryPtr& query : queryParser->getQueries())
    {
        for (TokenPtr& token : query->tokens)
        {
            if (token->type == Token::INVALID)
                markErrorAt(token->start, token->end, true);
        }
    }

    if (queryParser->isSuccessful())
    {
        emit errorsChecked(false);
        return;
    }

    // Setting new markers when errors were detected
    for (ParserError* error : queryParser->getErrors())
        markErrorAt(sqlIndex(error->getFrom()), sqlIndex(error->getTo()));

    emit errorsChecked(true);
}

void SqlEditor::checkForValidObjects()
{
    clearDbObjects();
    if (!db || !db->isValid())
        return;

    QList<SqliteStatement::FullObject> fullObjects;
    QString dbName;
    for (const SqliteQueryPtr& query : queryParser->getQueries())
    {
        fullObjects = query->getContextFullObjects();
        for (SqliteStatement::FullObject& fullObj : fullObjects)
        {
            dbName = fullObj.database ? stripObjName(fullObj.database->value) : "main";
            if (!objectsInNamedDb.contains(dbName, Qt::CaseInsensitive))
                continue;

            if (fullObj.type == SqliteStatement::FullObject::DATABASE)
            {
                // Valid db name
                addDbObject(sqlIndex(fullObj.database->start), sqlIndex(fullObj.database->end), QString());
                continue;
            }

            if (!objectsInNamedDb[dbName].contains(stripObjName(fullObj.object->value), Qt::CaseInsensitive))
                continue;

            // Valid object name
            addDbObject(sqlIndex(fullObj.object->start), sqlIndex(fullObj.object->end), dbName);
        }
    }
}

void SqlEditor::scheduleQueryParser(bool force, bool skipCompleter)
{
    if (!document()->isModified() && !force)
        return;

    syntaxValidated = false;

    document()->setModified(false);
    queryParserTrigger->schedule();
    if (!skipCompleter)
        autoCompleteTrigger->schedule();
}

int SqlEditor::sqlIndex(int idx)
{
    if (virtualSqlExpression.isNull())
        return idx;

    if (idx < virtualSqlOffset)
        return virtualSqlOffset;

    idx -= virtualSqlOffset;

    int lastIdx = toPlainText().length() - 1;
    if (idx > lastIdx)
        return lastIdx;

    return idx;
}

void SqlEditor::updateLineNumberArea()
{
    updateLineNumberArea(viewport()->rect(), viewport()->y());
}

bool SqlEditor::hasSelection() const
{
    return textCursor().hasSelection();
}

void SqlEditor::replaceSelectedText(const QString &newText)
{
    textCursor().insertText(newText);
}

QString SqlEditor::getSelectedText() const
{
    QString txt = textCursor().selectedText();
    fixTextCursorSelectedText(txt);
    return txt;
}

void SqlEditor::openObject(const QString& database, const QString& name)
{
    DbObjectDialogs dialogs(db);
    dialogs.editObject(DbObjectDialogs::Type::UNKNOWN, database, name);
}

void SqlEditor::highlightSyntax()
{
    highlightingSyntax = true;
    highlighter->rehighlight();
    highlightingSyntax = false;
}

void SqlEditor::updateLineNumberAreaWidth()
{
    if (!showLineNumbers)
    {
        setViewportMargins(0, 0, 0, 0);
        return;
    }

    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void SqlEditor::highlightCurrentLine(QList<QTextEdit::ExtraSelection>& selections)
{
    if (!isReadOnly() && isEnabled())
    {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(CFG_UI.Colors.SyntaxCurrentLineBg.get());
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }
}

void SqlEditor::updateLineNumberArea(const QRect& rect, int dy)
{
    if (!showLineNumbers)
    {
        updateLineNumberAreaWidth();
        return;
    }

    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth();
}

void SqlEditor::cursorMoved()
{
    highlightCurrentCursorContext();
    if (!cursorMovingByLocator)
    {
        textLocator->setStartPosition(textCursor().position());
        textLocator->cursorMoved();
    }
}

void SqlEditor::checkContentSize()
{
    if (document()->characterCount() > SqliteSyntaxHighlighter::MAX_QUERY_LENGTH)
    {
        if (richFeaturesEnabled)
            notifyWarn(tr("Contents of the SQL editor are huge, so errors detecting and existing objects highlighting are temporarily disabled."));

        richFeaturesEnabled = false;
    }
    else if (!richFeaturesEnabled)
    {
        richFeaturesEnabled = true;
    }
}

void SqlEditor::formatSql()
{
    QString sql = hasSelection() ? getSelectedText() : toPlainText();
    sql = SQLITESTUDIO->getCodeFormatter()->format("sql", sql, db);

    if (!hasSelection())
        selectAll();

    replaceSelectedText(sql);
}

void SqlEditor::saveToFile()
{
    if (loadedFile.isNull())
        saveAsToFile();
    else
        saveToFile(loadedFile);
}

void SqlEditor::saveAsToFile()
{
    if (!openSaveActionsEnabled)
        return;

    QString dir = getFileDialogInitPath();
    QString fName = QFileDialog::getSaveFileName(this, tr("Save to file"), dir);
    if (fName.isNull())
        return;

    setFileDialogInitPathByFile(fName);
    loadedFile = fName;
    saveToFile(loadedFile);
}

void SqlEditor::loadFromFile()
{
    if (!openSaveActionsEnabled)
        return;

    QString dir = getFileDialogInitPath();
    QString filters = tr("SQL scripts (*.sql);;All files (*)");
    QString fName = QFileDialog::getOpenFileName(this, tr("Open file"), dir, filters);
    if (fName.isNull())
        return;

    setFileDialogInitPathByFile(fName);

    QString err;
    QString sql = readFileContents(fName, &err);
    if (sql.isNull() && !err.isNull())
    {
        notifyError(tr("Could not open file '%1' for reading: %2").arg(fName, err));
        return;
    }

    setPlainText(sql);

    loadedFile = fName;
}

void SqlEditor::deleteLine()
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
        deleteSelectedLines();
    else
        deleteCurrentLine();
}

void SqlEditor::deleteCurrentLine()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    QTextDocument* doc = document();
    QTextBlock block = doc->findBlock(cursor.position());
    if (block.next().isValid())
        cursor.deleteChar();
    else
    {
        cursor.deletePreviousChar();
        cursor.movePosition(QTextCursor::StartOfLine);
    }
    setTextCursor(cursor);
}

void SqlEditor::deleteSelectedLines()
{
    QTextCursor cursor = textCursor();
    QTextDocument* doc = document();
    QTextBlock startBlock = doc->findBlock(cursor.selectionStart());
    QTextBlock endBlock = doc->findBlock(cursor.selectionEnd() - 1);
    int idxMod = 0;
    if (!endBlock.next().isValid()) // no newline at the end
        idxMod = -1;

    cursor.setPosition(startBlock.position());
    cursor.setPosition(endBlock.position() + endBlock.length() + idxMod, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}
void SqlEditor::moveBlockDown(bool deleteOld)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
    {
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    }

    QTextDocument* doc = document();
    QTextBlock startBlock = doc->findBlock(cursor.selectionStart());
    QTextBlock endBlock = doc->findBlock(cursor.selectionEnd() - 1);

    QTextBlock nextBlock = endBlock.next();
    QTextBlock blockBeforeNewText = endBlock;

    // When moving text, we next block to be valid and operate on one after that
    if (deleteOld)
    {
        if (!nextBlock.isValid())
            return;

        blockBeforeNewText = nextBlock;
        nextBlock = nextBlock.next();
    }

    // If next block is invalid, we need to create it
    bool removeLastNewLine = false;
    if (!nextBlock.isValid())
    {
        cursor.setPosition(blockBeforeNewText.position());
        cursor.movePosition(QTextCursor::EndOfLine);
        cursor.insertBlock();
        nextBlock = blockBeforeNewText.next();
        removeLastNewLine = true;
    }

    int textLength = endBlock.position() + endBlock.length() - startBlock.position();

    // Collecting text and removing text from old position (if not copying)
    cursor.setPosition(startBlock.position());
    cursor.setPosition(startBlock.position() + textLength, QTextCursor::KeepAnchor);
    QString text = cursor.selectedText();
    fixTextCursorSelectedText(text);
    if (deleteOld) // this is false when just copying
        cursor.removeSelectedText();

    // Pasting text at new position and reselecting it
    cursor.setPosition(nextBlock.position());
    cursor.insertText(text);
    cursor.setPosition(nextBlock.position() + textLength);
    if (removeLastNewLine) // this is done when we moved to the last line, created block and copied another \n to it
        cursor.deletePreviousChar();

    cursor.setPosition(nextBlock.position(), QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

void SqlEditor::moveBlockUp(bool deleteOld)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
    {
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    }

    QTextDocument* doc = document();
    QTextBlock startBlock = doc->findBlock(cursor.selectionStart());
    QTextBlock endBlock = doc->findBlock(cursor.selectionEnd() - 1);
    bool hasNewLineChar = endBlock.next().isValid();

    QTextBlock insertingBlock = startBlock;
    if (deleteOld)
    {
        insertingBlock = startBlock.previous();
        if (!insertingBlock.isValid())
            return;
    }

    // We will operate on full line length, unless next block was invalid, thus at the end there's no new line.
    int textLength = endBlock.position() + endBlock.length() - startBlock.position();
    if (!hasNewLineChar)
        textLength--;

    // Collecting text and removing text from old position (if not copying)
    cursor.setPosition(startBlock.position());
    cursor.setPosition(startBlock.position() + textLength, QTextCursor::KeepAnchor);
    QString text = cursor.selectedText();
    fixTextCursorSelectedText(text);
    if (deleteOld) // this is false when just copying
        cursor.removeSelectedText();

    // Pasting text at new position
    cursor.setPosition(insertingBlock.position());
    cursor.insertText(text);
    if (!hasNewLineChar)
    {
        cursor.insertBlock();
        cursor.setPosition(insertingBlock.next().next().position());
        cursor.deletePreviousChar();
        textLength++; // we will need to include "new line" when reselecting text
    }

    // Reselecting new text
    cursor.setPosition(insertingBlock.position() + textLength);
    cursor.setPosition(insertingBlock.position(), QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

void SqlEditor::copyBlockDown()
{
    moveBlockDown(false);
}

void SqlEditor::copyBlockUp()
{
    moveBlockUp(false);
}

void SqlEditor::find()
{
    textLocator->setStartPosition(textCursor().position());
    showSearchDialog();
}

void SqlEditor::findNext()
{
    textLocator->findNext();
}

void SqlEditor::findPrevious()
{
    textLocator->findPrev();
}

void SqlEditor::replace()
{
    showSearchDialog();
}

void SqlEditor::found(int start, int end)
{
    QTextCursor cursor = textCursor();
    cursor.setPosition(end);
    cursor.setPosition(start, QTextCursor::KeepAnchor);
    cursorMovingByLocator = true;
    setTextCursor(cursor);
    cursorMovingByLocator = false;
    ensureCursorVisible();
}

void SqlEditor::reachedEnd()
{
    notifyInfo(tr("Reached the end of document. Hit the find again to restart the search."));
}

void SqlEditor::changeFont(const QVariant& font)
{
    auto f = font.value<QFont>();
    setFont(f);
    lineNumberArea->setFont(f);
}

void SqlEditor::configModified()
{
    colorsConfigChanged();
}

void SqlEditor::toggleComment()
{
    // Handle no selection - toggle single line
    QTextCursor cur = textCursor();
    int start = cur.selectionStart();
    int end = cur.selectionEnd();

    if (start == end)
    {
        toggleLineCommentForLine(cur.block());
        return;
    }

    // Handle multiline selection - from begin of the line to begin of the line
    QTextDocument* doc = document();

    QTextBlock startBlock = doc->findBlock(start);
    bool startAtLineBegining = startBlock.position() == start;

    QTextBlock endBlock = doc->findBlock(end);
    bool endAtLineBegining = endBlock.position() == end;

    if (startAtLineBegining && endAtLineBegining)
    {
        // Check if all lines where commented previously
        bool allCommented = true;
        for (QTextBlock theBlock = startBlock; theBlock != endBlock; theBlock = theBlock.next())
        {
            if (!theBlock.text().startsWith("--"))
            {
                allCommented = false;
                break;
            }
        }

        // Apply comment toggle
        cur.beginEditBlock();
        for (QTextBlock theBlock = startBlock; theBlock != endBlock; theBlock = theBlock.next())
        {
            cur.setPosition(theBlock.position());
            if (allCommented)
            {
                cur.deleteChar();
                cur.deleteChar();
            }
            else
                cur.insertText("--");
        }

        cur.setPosition(start);
        cur.setPosition(endBlock.position(), QTextCursor::KeepAnchor);
        cur.endEditBlock();
        setTextCursor(cur);
        return;
    }

    // Handle custom selection
    QString txt = cur.selectedText().trimmed();
    cur.beginEditBlock();
    if (txt.startsWith("/*") && txt.endsWith("*/"))
    {
        cur.setPosition(end);
        cur.deletePreviousChar();
        cur.deletePreviousChar();
        cur.setPosition(start);
        cur.deleteChar();
        cur.deleteChar();

        cur.setPosition(start);
        cur.setPosition(end - 4, QTextCursor::KeepAnchor);
    }
    else
    {
        cur.setPosition(end);
        cur.insertText("*/");
        cur.setPosition(start);
        cur.insertText("/*");

        cur.setPosition(start);
        cur.setPosition(end + 4, QTextCursor::KeepAnchor);
    }
    cur.endEditBlock();
    setTextCursor(cur);
}

void SqlEditor::wordWrappingChanged(const QVariant& value)
{
    setLineWrapMode(value.toBool() ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

void SqlEditor::currentCursorContextDelayedHighlight()
{
    highlightCurrentCursorContext(true);
}

void SqlEditor::fontSizeChangeRequested(int delta)
{
    changeFontSize(delta >= 0 ? 1 : -1);
}

void SqlEditor::incrFontSize()
{
    changeFontSize(1);
}

void SqlEditor::decrFontSize()
{
    changeFontSize(-1);
}

void SqlEditor::moveCursorTo(int pos)
{
    QTextCursor cur = textCursor();
    cur.setPosition(pos);
    setTextCursor(cur);
}

void SqlEditor::changeFontSize(int factor)
{
    auto f = font();
    f.setPointSize(f.pointSize() + factor);
    CFG_UI.Fonts.SqlEditor.set(f);
}

void SqlEditor::colorsConfigChanged()
{
    highlightSyntax();
    highlightCurrentCursorContext();
}

void SqlEditor::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Backspace:
        {
            deletionKeyPressed = true;
            if (e->modifiers().testFlag(Qt::NoModifier))
                backspacePressed();
            else
                QPlainTextEdit::keyPressEvent(e);
            deletionKeyPressed = false;
            return;
        }
        case Qt::Key_Delete:
        {
            deletionKeyPressed = true;
            QPlainTextEdit::keyPressEvent(e);
            deletionKeyPressed = false;
            return;
        }
        case Qt::Key_Home:
        {
            homePressed(e->modifiers());
            return;
        }
        case Qt::Key_Tab:
        {
            tabPressed(e->modifiers().testFlag(Qt::ShiftModifier));
            return;
        }
        case Qt::Key_Backtab:
        {
            tabPressed(true);
            return;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            QPlainTextEdit::keyPressEvent(e);
            indentNewLine();
            return;
        }
        case Qt::Key_Control:
            setObjectLinks(true);
            break;
        default:
            break;
    }
    QPlainTextEdit::keyPressEvent(e);
}

void SqlEditor::keyReleaseEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Control)
        setObjectLinks(false);

    QPlainTextEdit::keyReleaseEvent(e);
}

void SqlEditor::focusOutEvent(QFocusEvent* e)
{
    UNUSED(e);
    setObjectLinks(false);
    QPlainTextEdit::focusOutEvent(e);
}

void SqlEditor::focusInEvent(QFocusEvent* e)
{
    if (completer->isVisible())
    {
        // Sometimes, when switching to other application window and then getting back to SQLiteStudio,
        // the completer loses focus, but doesn't close. In that case, the SqlEditor gets focused,
        // while completer still exists. Here we fix this case.
        completer->reject();
        return;
    }

    QPlainTextEdit::focusInEvent(e);
}

void SqlEditor::mouseMoveEvent(QMouseEvent* e)
{
    handleValidObjectCursor(e->pos());
    QPlainTextEdit::mouseMoveEvent(e);
}

void SqlEditor::mousePressEvent(QMouseEvent* e)
{
    if (objectLinksEnabled)
    {
        const DbObject* obj = getValidObjectForPosition(e->pos());
        if (obj && e->button() == Qt::LeftButton)
        {
            QString objName = toPlainText().mid(obj->from, (obj->to - obj->from + 1));
            openObject(obj->dbName, stripObjName(objName));
        }
    }

    QPlainTextEdit::mousePressEvent(e);
}

void SqlEditor::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void SqlEditor::handleValidObjectCursor(const QPoint& point)
{
    if (!objectLinksEnabled)
        return;

    QTextCursor cursor = cursorForPosition(point);
    int position = cursor.position();
    QRect curRect = cursorRect(cursor);
    bool isValid = false;
    if (point.y() >= curRect.top() && point.y() <= curRect.bottom())
    {
        // Mouse pointer is at the same line as cursor, so cursor was returned for actual character under mouse
        // and not just first/last character of the line, because mouse was out of text.
        bool movedLeft = (curRect.x() - point.x()) < 0;
        isValid = (getValidObjectForPosition(position, movedLeft) != nullptr);
    }
    viewport()->setCursor(isValid ? Qt::PointingHandCursor : Qt::IBeamCursor);
}

bool SqlEditor::getVirtualSqlCompleteSemicolon() const
{
    return virtualSqlCompleteSemicolon;
}

void SqlEditor::setVirtualSqlCompleteSemicolon(bool value)
{
    virtualSqlCompleteSemicolon = value;
}

bool SqlEditor::getShowLineNumbers() const
{
    return showLineNumbers;
}

void SqlEditor::setShowLineNumbers(bool value)
{
    showLineNumbers = value;
    lineNumberArea->setVisible(value);
    updateLineNumberArea();
}

void SqlEditor::checkSyntaxNow()
{
    queryParserTrigger->cancel();
    parseContents();
}

void SqlEditor::saveSelection()
{
    QTextCursor cur = textCursor();
    storedSelectionStart = cur.selectionStart();
    storedSelectionEnd = cur.selectionEnd();
}

void SqlEditor::restoreSelection()
{
    QTextCursor cur = textCursor();
    cur.setPosition(storedSelectionStart);
    cur.setPosition(storedSelectionEnd, QTextCursor::KeepAnchor);
}

QToolBar* SqlEditor::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return nullptr;
}

void SqlEditor::setCurrentQueryHighlighting(bool enabled)
{
    if (enabled && !currentQueryTimer)
    {
        currentQueryTimer = new QTimer(this);
        currentQueryTimer->setInterval(300);
        currentQueryTimer->setSingleShot(true);
        connect(currentQueryTimer, SIGNAL(timeout()), this, SLOT(currentCursorContextDelayedHighlight()));
    }
    else if (!enabled && currentQueryTimer)
    {
        safe_delete(currentQueryTimer);
    }
}

QString SqlEditor::getVirtualSqlExpression() const
{
    return virtualSqlExpression;
}

void SqlEditor::setVirtualSqlExpression(const QString& value)
{
    virtualSqlExpression = value;

    virtualSqlOffset = virtualSqlExpression.indexOf("%1");
    if (virtualSqlOffset == -1)
    {
        virtualSqlOffset = 0;
        virtualSqlExpression = QString();
        qWarning() << "Tried to set invalid virtualSqlExpression for SqlEditor. Ignored.";
        return;
    }

    virtualSqlRightOffset = virtualSqlExpression.length() - virtualSqlOffset - 2;
}

void SqlEditor::setTriggerContext(const QString& table)
{
    createTriggerTable = table;
    highlighter->setCreateTriggerContext(!table.isEmpty());
}

const SqlEditor::DbObject* SqlEditor::getValidObjectForPosition(const QPoint& point)
{
    QTextCursor cursor = cursorForPosition(point);
    int position = cursor.position();
    bool movedLeft = (cursorRect(cursor).x() - point.x()) < 0;
    return getValidObjectForPosition(position, movedLeft);
}

const SqlEditor::DbObject* SqlEditor::getValidObjectForPosition(int position, bool movedLeft)
{
    for (DbObject& obj : validDbObjects)
    {
        if ((!movedLeft && position > obj.from && position-1 <= obj.to) ||
            (movedLeft && position >= obj.from && position <= obj.to))
        {
            return &obj;
        }
    }
    return nullptr;
}

SqlEditor::DbObject::DbObject(int from, int to, const QString& dbName) :
    from(from), to(to), dbName(dbName)
{

}

SqlEditor::LineNumberArea::LineNumberArea(SqlEditor* editor) :
    QWidget(editor), codeEditor(editor)
{
}

QSize SqlEditor::LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void SqlEditor::LineNumberArea::paintEvent(QPaintEvent* event)
{
    if (codeEditor->getShowLineNumbers())
        codeEditor->lineNumberAreaPaintEvent(event);
}


void SqlEditor::changeEvent(QEvent* e)
{
//    if (e->type() == QEvent::EnabledChange)
//        highlightCurrentLine();

    QPlainTextEdit::changeEvent(e);
}

void SqlEditor::showEvent(QShowEvent* event)
{
    UNUSED(event);
    setLineWrapMode(wrapWords ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

void SqlEditor::dropEvent(QDropEvent* e)
{
    QPlainTextEdit::dropEvent(e);
    if (MAINWINDOW->getDbTree()->getModel()->hasDbTreeItem(e->mimeData()))
        e->ignore();
}
