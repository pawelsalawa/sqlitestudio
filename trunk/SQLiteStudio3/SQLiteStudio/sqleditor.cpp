#include "sqleditor.h"
#include "uiconfig.h"
#include "services/config.h"
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
#include "sqlitestudio.h"
#include "dbtree/dbtreeitem.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QFileDialog>

CFG_KEYS_DEFINE(SqlEditor)

SqlEditor::SqlEditor(QWidget *parent) :
    QPlainTextEdit(parent)
{
    init();
}

SqlEditor::~SqlEditor()
{
    if (queryParser)
    {
        delete queryParser;
        queryParser = nullptr;
    }
}

void SqlEditor::init()
{
    highlighter = new SqliteSyntaxHighlighter(document());
    setFont(CFG_UI.Fonts.SqlEditor.get());
    initActions();
    setupMenu();

    textLocator = new SearchTextLocator(document(), this);
    connect(textLocator, SIGNAL(found(int,int)), this, SLOT(found(int,int)));
    connect(textLocator, SIGNAL(reachedEnd()), this, SLOT(reachedEnd()));

    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth()));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorMoved()));

    updateLineNumberAreaWidth();
    highlightCurrentLine();

    completer = new CompleterWindow(this);
    connect(completer, SIGNAL(accepted()), this, SLOT(completeSelected()));
    connect(completer, SIGNAL(textTyped(QString)), this, SLOT(completerTypedText(QString)));
    connect(completer, SIGNAL(backspacePressed()), this, SLOT(completerBackspacePressed()));
    connect(completer, SIGNAL(leftPressed()), this, SLOT(completerLeftPressed()));
    connect(completer, SIGNAL(rightPressed()), this, SLOT(completerRightPressed()));

    autoCompleteTimer = new QTimer(this);
    autoCompleteTimer->setSingleShot(true);
    autoCompleteTimer->setInterval(autoCompleterDelay);
    connect(autoCompleteTimer, SIGNAL(timeout()), this, SLOT(checkForAutoCompletion()));
    //connect(this, SIGNAL(textChanged()), this, SLOT(scheduleAutoCompletion()));

    queryParserTimer = new QTimer(this);
    queryParserTimer->setSingleShot(true);
    queryParserTimer->setInterval(queryParserDelay);
    connect(queryParserTimer, SIGNAL(timeout()), this, SLOT(parseContents()));
    connect(this, SIGNAL(textChanged()), this, SLOT(scheduleQueryParser()));

    queryParser = new Parser(Dialect::Sqlite3);

    connect(this, &QWidget::customContextMenuRequested, this, &SqlEditor::customContextMenuRequested);
    connect(CFG_UI.Fonts.SqlEditor, SIGNAL(changed(QVariant)), this, SLOT(changeFont(QVariant)));
    connect(CFG, SIGNAL(massSaveCommited()), this, SLOT(configModified()));
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
    createAction(OPEN_SQL_FILE, ICONS.OPEN_SQL_FILE, tr("Load SQL from file", "sql editor"), this, SLOT(loadFromFile()), this);
    createAction(DELETE_LINE, ICONS.ACT_DEL_LINE, tr("Delete line", "sql editor"), this, SLOT(deleteLine()), this);
    createAction(MOVE_BLOCK_DOWN, tr("Move block down", "sql editor"), this, SLOT(moveBlockDown()), this);
    createAction(MOVE_BLOCK_UP, tr("Move block up", "sql editor"), this, SLOT(moveBlockUp()), this);
    createAction(COPY_BLOCK_DOWN, tr("Copy block down", "sql editor"), this, SLOT(copyBlockDown()), this);
    createAction(COPY_BLOCK_UP, tr("Copy up down", "sql editor"), this, SLOT(copyBlockUp()), this);
    createAction(FIND, ICONS.ACT_SEARCH, tr("Find", "sql editor"), this, SLOT(find()), this);
    createAction(FIND_NEXT, tr("Find next", "sql editor"), this, SLOT(findNext()), this);
    createAction(FIND_PREV, tr("Find previous", "sql editor"), this, SLOT(findPrevious()), this);
    createAction(REPLACE, tr("Replace", "sql editor"), this, SLOT(replace()), this);

    actionMap[CUT]->setEnabled(false);
    actionMap[COPY]->setEnabled(false);
    actionMap[UNDO]->setEnabled(false);
    actionMap[REDO]->setEnabled(false);
    actionMap[DELETE]->setEnabled(false);

    connect(this, &QPlainTextEdit::undoAvailable, this, &SqlEditor::updateUndoAction);
    connect(this, &QPlainTextEdit::redoAvailable, this, &SqlEditor::updateRedoAction);
    connect(this, &QPlainTextEdit::copyAvailable, this, &SqlEditor::updateCopyAction);
}

void SqlEditor::setupDefShortcuts()
{
    setShortcutContext({CUT, COPY, PASTE, DELETE, SELECT_ALL, UNDO, REDO, COMPLETE, FORMAT_SQL, SAVE_SQL_FILE, OPEN_SQL_FILE,
                        DELETE_LINE}, Qt::WidgetWithChildrenShortcut);

    BIND_SHORTCUTS(SqlEditor, Action);
}

void SqlEditor::setupMenu()
{
    contextMenu = new QMenu(this);
    contextMenu->addAction(actionMap[FORMAT_SQL]);
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
}

void SqlEditor::setAutoCompletion(bool enabled)
{
    autoCompletion = enabled;
}

void SqlEditor::customContextMenuRequested(const QPoint &pos)
{
    if (objectLinksEnabled)
    {
        const DbObject* obj = getValidObjectForPosition(pos);
        QString objName = toPlainText().mid(obj->from, (obj->to - obj->from + 1));

        validObjContextMenu->clear();

        DbTreeItem* item = nullptr;
        for (DbTreeItem::Type type : {DbTreeItem::Type::TABLE, DbTreeItem::Type::INDEX, DbTreeItem::Type::TRIGGER, DbTreeItem::Type::VIEW})
        {
            item = DBTREE->getModel()->findItem(type, objName);
            if (item)
                break;
        }

        if (item)
        {
            DBTREE->setSelectedItem(item);
            DBTREE->setupActionsForMenu(item, validObjContextMenu);
            if (validObjContextMenu->actions().size() == 0)
                return;

            DBTREE->updateActionStates(item);
            validObjContextMenu->popup(mapToGlobal(pos));
        }
        return;
    }
    contextMenu->popup(mapToGlobal(pos));
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
    deletePreviousChars(completer->getNumberOfCharsToRemove());

    ExpectedTokenPtr token = completer->getSelected();
    QString value = token->value;
    if (token->needsWrapping())
        value = wrapObjIfNeeded(value, getDialect());

    if (!token->prefix.isNull())
    {
        value.prepend(".");
        value.prepend(wrapObjIfNeeded(token->prefix, getDialect()));
    }

    insertPlainText(value);
}

void SqlEditor::scheduleAutoCompletion()
{
    autoCompleteTimer->stop();

    if (autoCompletion && !deletionKeyPressed)
        autoCompleteTimer->start();
}

void SqlEditor::checkForAutoCompletion()
{
    if (!db || !autoCompletion || deletionKeyPressed)
        return;

    Lexer lexer(getDialect());
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

    objectsInNamedDb.clear();

    SchemaResolver resolver(db);
    QSet<QString> databases = resolver.getDatabases();
    databases << "main";
    QStringList objects;
    foreach (const QString& dbName, databases)
    {
        objects = resolver.getAllObjects();
        objectsInNamedDb[dbName] << objects;
    }
}

Dialect SqlEditor::getDialect()
{
    return !db ? Dialect::Sqlite3 : db->getDialect();
}

void SqlEditor::setObjectLinks(bool enabled)
{
    objectLinksEnabled = enabled;
    setMouseTracking(enabled);
    highlighter->setObjectLinksEnabled(enabled);
    highlighter->rehighlight();

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
    painter.fillRect(event->rect(), CFG_UI.Colors.SqlEditorLineNumAreaBg.get());
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
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

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

void SqlEditor::highlightParenthesis()
{
    // Clear extra selections
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    // Just keep "current line" highlighting
    QMutableListIterator<QTextEdit::ExtraSelection> it(selections);
    while (it.hasNext())
    {
        if (!it.next().format.property(QTextFormat::FullWidthSelection).toBool())
            it.remove();
    }
    setExtraSelections(selections);

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
    setExtraSelections(selections);
}

void SqlEditor::markMatchedParenthesis(int pos1, int pos2, QList<QTextEdit::ExtraSelection>& selections)
{
    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(CFG_UI.Colors.SqlEditorParenthesisBg.get());

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
    {
        searchDialog = new SearchTextDialog(textLocator, this);
        searchDialog->setWindowTitle(tr("Find or replace", "sql editor find/replace dialog"));
    }

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
    // Updating dialect according to current database (if any)
    Dialect dialect = Dialect::Sqlite3;
    if (db && db->isValid())
        dialect = db->getDialect();

    QString sql = toPlainText();
    if (!virtualSqlExpression.isNull())
    {
        if (virtualSqlCompleteSemicolon && !sql.trimmed().endsWith(";"))
            sql += ";";

        sql = virtualSqlExpression.arg(sql);
    }

    queryParser->setDialect(dialect);
    queryParser->parse(sql);

    checkForValidObjects();
    checkForSyntaxErrors();
    highlighter->rehighlight();
}

void SqlEditor::checkForSyntaxErrors()
{
    syntaxValidated = true;

    removeErrorMarkers();

    // Marking invalid tokens, like in "SELECT * from test] t" - the "]" token is invalid.
    // Such tokens don't cause parser to fail.
    foreach (SqliteQueryPtr query, queryParser->getQueries())
    {
        foreach (TokenPtr token, query->tokens)
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
    foreach (ParserError* error, queryParser->getErrors())
        markErrorAt(sqlIndex(error->getFrom()), sqlIndex(error->getTo()));

    emit errorsChecked(true);
}

void SqlEditor::checkForValidObjects()
{
    clearDbObjects();
    if (!db || !db->isValid())
        return;

    Dialect dialect = db->getDialect();
    QList<SqliteStatement::FullObject> fullObjects;
    QString dbName;
    foreach (SqliteQueryPtr query, queryParser->getQueries())
    {
        fullObjects = query->getContextFullObjects();
        foreach (const SqliteStatement::FullObject& fullObj, fullObjects)
        {
            dbName = fullObj.database ? stripObjName(fullObj.database->value, dialect) : "main";
            if (!objectsInNamedDb.contains(dbName))
                continue;

            if (fullObj.type == SqliteStatement::FullObject::DATABASE)
            {
                // Valid db name
                addDbObject(sqlIndex(fullObj.database->start), sqlIndex(fullObj.database->end), QString::null);
                continue;
            }

            if (!objectsInNamedDb[dbName].contains(stripObjName(fullObj.object->value, dialect)))
                continue;

            // Valid object name
            addDbObject(sqlIndex(fullObj.object->start), sqlIndex(fullObj.object->end), dbName);
        }
    }
}

void SqlEditor::scheduleQueryParser()
{
    if (!document()->isModified())
        return;

    syntaxValidated = false;

    document()->setModified(false);
    queryParserTimer->stop();
    queryParserTimer->start();

    scheduleAutoCompletion();
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
    return textCursor().selectedText();
}

void SqlEditor::openObject(const QString& database, const QString& name)
{
    DbObjectDialogs dialogs(db);
    dialogs.editObject(database, name);
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

void SqlEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections;

    if (!isReadOnly() && isEnabled())
    {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(CFG_UI.Colors.SqlEditorCurrentLineBg.get());
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    setExtraSelections(selections);
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
    highlightParenthesis();

    if (!cursorMovingByLocator)
    {
        textLocator->setStartPosition(textCursor().position());
        textLocator->cursorMoved();
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
    QString fName = QFileDialog::getSaveFileName(this, tr("Save to file"));
    if (fName.isNull())
        return;

    QFile file(fName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        notifyError(tr("Could not open file '%1'' for writing: %2").arg(fName).arg(file.errorString()));
        return;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << toPlainText();
    stream.flush();
    file.close();
}

void SqlEditor::loadFromFile()
{
    QString filters = tr("SQL scripts (*.sql);;All files (*)");
    QString fName = QFileDialog::getOpenFileName(this, tr("Open file"), QString(), filters);
    if (fName.isNull())
        return;

    QFile file(fName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        notifyError(tr("Could not open file '%1'' for reading: %2").arg(fName).arg(file.errorString()));
        return;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString sql = stream.readAll();
    file.close();
    setPlainText(sql);
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
    setFont(font.value<QFont>());
}

void SqlEditor::configModified()
{
    highlighter->rehighlight();
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
            openObject(obj->dbName, objName);
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
    queryParserTimer->stop();
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
        virtualSqlExpression = QString::null;
        qWarning() << "Tried to set invalid virtualSqlExpression for SqlEditor. Ignored.";
        return;
    }

    virtualSqlRightOffset = virtualSqlExpression.length() - virtualSqlOffset - 2;
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
    foreach (const DbObject& obj, validDbObjects)
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
    if (e->type() == QEvent::EnabledChange)
        highlightCurrentLine();

    QPlainTextEdit::changeEvent(e);
}
