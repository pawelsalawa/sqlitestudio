#ifndef SQLEDITOR_H
#define SQLEDITOR_H

#include "common/extactioncontainer.h"
#include "db/db.h"
#include "sqlitesyntaxhighlighter.h"
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QFont>
#include <QHash>

class CompleterWindow;
class QTimer;
class Parser;
class SqlEditor;
class SearchTextDialog;
class SearchTextLocator;

CFG_KEY_LIST(SqlEditor, QObject::tr("SQL editor input field"),
    CFG_KEY_ENTRY(CUT,             QKeySequence::Cut,                 QObject::tr("Cut selected text"))
    CFG_KEY_ENTRY(COPY,            QKeySequence::Copy,                QObject::tr("Copy selected text"))
    CFG_KEY_ENTRY(PASTE,           QKeySequence::Paste,               QObject::tr("Paste from clipboard"))
    CFG_KEY_ENTRY(DELETE,          QKeySequence::Delete,              QObject::tr("Delete selected text"))
    CFG_KEY_ENTRY(SELECT_ALL,      QKeySequence::SelectAll,           QObject::tr("Select whole editor contents"))
    CFG_KEY_ENTRY(UNDO,            QKeySequence::Undo,                QObject::tr("Undo"))
    CFG_KEY_ENTRY(REDO,            QKeySequence::Redo,                QObject::tr("Redo"))
    CFG_KEY_ENTRY(SAVE_SQL_FILE,   QKeySequence::Save,                QObject::tr("Save contents into a file"))
    CFG_KEY_ENTRY(OPEN_SQL_FILE,   QKeySequence::Open,                QObject::tr("Load contents from a file"))
    CFG_KEY_ENTRY(FIND,            QKeySequence::Find,                QObject::tr("Find in text"))
    CFG_KEY_ENTRY(FIND_NEXT,       QKeySequence::FindNext,            QObject::tr("Find next"))
    CFG_KEY_ENTRY(FIND_PREV,       QKeySequence::FindPrevious,        QObject::tr("Find previous"))
    CFG_KEY_ENTRY(REPLACE,         QKeySequence::Replace,             QObject::tr("Replace in text"))
    CFG_KEY_ENTRY(DELETE_LINE,     Qt::CTRL + Qt::Key_D,              QObject::tr("Delete current line"))
    CFG_KEY_ENTRY(COMPLETE,        Qt::CTRL + Qt::Key_Space,          QObject::tr("Request code assistant"))
    CFG_KEY_ENTRY(FORMAT_SQL,      Qt::ALT + Qt::Key_F,               QObject::tr("Format contents"))
    CFG_KEY_ENTRY(MOVE_BLOCK_DOWN, Qt::ALT + Qt::Key_Down,            QObject::tr("Move selected block of text one line down"))
    CFG_KEY_ENTRY(MOVE_BLOCK_UP,   Qt::ALT + Qt::Key_Up,              QObject::tr("Move selected block of text one line up"))
    CFG_KEY_ENTRY(COPY_BLOCK_DOWN, Qt::ALT + Qt::CTRL + Qt::Key_Down, QObject::tr("Copy selected block of text and paste it a line below"))
    CFG_KEY_ENTRY(COPY_BLOCK_UP,   Qt::ALT + Qt::CTRL + Qt::Key_Up,   QObject::tr("Copy selected block of text and paste it a line above"))
)

class SqlEditor : public QPlainTextEdit, public ExtActionContainer
{
        Q_OBJECT
        Q_ENUMS(Action)

    public:
        enum Action
        {
            COPY,
            PASTE,
            CUT,
            UNDO,
            REDO,
            DELETE,
            SELECT_ALL,
            FORMAT_SQL,
            OPEN_SQL_FILE,
            SAVE_SQL_FILE,
            DELETE_LINE,
            COMPLETE,
            MOVE_BLOCK_DOWN,
            MOVE_BLOCK_UP,
            COPY_BLOCK_DOWN,
            COPY_BLOCK_UP,
            FIND,
            FIND_NEXT,
            FIND_PREV,
            REPLACE
        };

        explicit SqlEditor(QWidget *parent = 0);
        ~SqlEditor();

        Db* getDb() const;
        void setDb(Db* value);
        void setAutoCompletion(bool enabled);
        QString getVirtualSqlExpression() const;
        void setVirtualSqlExpression(const QString& value);
        bool haveErrors();
        bool isSyntaxChecked();
        bool getShowLineNumbers() const;
        void setShowLineNumbers(bool value);
        void checkSyntaxNow();
        void saveSelection();
        void restoreSelection();

        bool getVirtualSqlCompleteSemicolon() const;
        void setVirtualSqlCompleteSemicolon(bool value);

    protected:
        void setupDefShortcuts();
        void createActions();
        void keyPressEvent(QKeyEvent* e);
        void keyReleaseEvent(QKeyEvent* e);
        void focusOutEvent(QFocusEvent* e);
        void focusInEvent(QFocusEvent* e);
        void mouseMoveEvent(QMouseEvent* e);
        void mousePressEvent(QMouseEvent* e);
        void resizeEvent(QResizeEvent *e);
        void changeEvent(QEvent*e);

    private:
        class LineNumberArea : public QWidget
        {
            public:
                explicit LineNumberArea(SqlEditor *editor);
                QSize sizeHint() const;

            protected:
                void paintEvent(QPaintEvent *event);

            private:
                SqlEditor *codeEditor;
        };

        struct DbObject
        {
            DbObject(int from, int to, const QString& dbName);

            int from;
            int to;

            /**
             * @brief dbName
             * Attach name for the db that object belongs to.
             * If the object is database itself, then this variable is null.
             */
            QString dbName;
        };

        void setupMenu();
        void updateCompleterPosition();
        void init();
        void removeErrorMarkers();
        void deleteCurrentLine();
        void deleteSelectedLines();

        /**
         * @brief markErrorAt Mark error range.
         * @param start Start index of error.
         * @param end End index of error.
         * @param limitedDamage true if error is just invalid token, that didn't cause parser to fail.
         */
        void markErrorAt(int start, int end, bool limitedDamage = false);
        void deletePreviousChars(int length = 1);
        void refreshValidObjects();
        void checkForSyntaxErrors();
        void checkForValidObjects();
        Dialect getDialect();
        void setObjectLinks(bool enabled);
        void addDbObject(int from, int to, const QString& dbName);
        void clearDbObjects();
        void lineNumberAreaPaintEvent(QPaintEvent* event);
        int lineNumberAreaWidth();
        void highlightParenthesis();
        const TextBlockData::Parenthesis* matchParenthesis(QList<const TextBlockData::Parenthesis*> parList, const TextBlockData::Parenthesis* thePar);
        void markMatchedParenthesis(int pos1, int pos2, QList<QTextEdit::ExtraSelection>& selections);
        void doBackspace(int repeats = 1);
        void indentSelected(bool shiftPressed);
        void indentBlock(const QTextBlock& block);
        void unindentBlock(const QTextBlock& block);
        void indentNewLine();
        void showSearchDialog();
        int sqlIndex(int idx);
        void updateLineNumberArea();
        bool hasSelection() const;
        void replaceSelectedText(const QString& newText);
        QString getSelectedText() const;
        void openObject(const QString& database, const QString& name);

        /**
         * @brief getValidObjectForPosition
         * @param position Cursor text position determinated by Qt mouse event.
         * @param movedLeft true if Qt moved cursor left from click point, which means that user clicked closer to left border of character. Otherwise cursor was moved towards right.
         * @return Object identified under given text position, or null if there was no valid object under that position.
         */
        const DbObject* getValidObjectForPosition(int position, bool movedLeft);
        void handleValidObjectCursor(const QPoint& point);

        SqliteSyntaxHighlighter* highlighter;
        QMenu* contextMenu;
        Db* db = nullptr;
        CompleterWindow* completer = nullptr;
        QTimer* autoCompleteTimer = nullptr;
        bool autoCompletion = true;
        bool deletionKeyPressed = false;
        QTimer* queryParserTimer = nullptr;
        Parser* queryParser = nullptr;
        QHash<QString,QStringList> objectsInNamedDb;
        bool objectLinksEnabled = false;
        QList<DbObject> validDbObjects;
        QWidget* lineNumberArea = nullptr;
        SearchTextDialog* searchDialog = nullptr;
        SearchTextLocator* textLocator = nullptr;
        bool cursorMovingByLocator = false;
        bool syntaxValidated = false;
        bool showLineNumbers = true;
        int storedSelectionStart = 0;
        int storedSelectionEnd = 0;

        /**
         * @brief virtualSqlExpression
         * It has to be an SQL string containing exactly one argument %1 (as Qt string arguments).
         * It will be used in every syntax completion request as a template, as if user
         * wrote this entire SQL statement, plus his own code in place of %1 and then the completer is invoked.
         * User never sees this SQL expression, it's hidden from him.
         * The expression will also be used for syntax error checking the same was as for completer.
         *
         * This is useful for example when we want to have a context for completion in CHECK() constraint,
         * but user has only input edit for the CHECK expression itself, so for completer to work correctly
         * it needs to be lied that there is entire "CREATE TABLE...CHECK(" before the users code. In that
         * case you would set this variable to something like this: "CREATE TABLE x (c CHECK(%1))".
         * The SqlEditor is smart enough to do all the magic given the above expression.
         */
        QString virtualSqlExpression;
        int virtualSqlOffset = 0;
        int virtualSqlRightOffset = 0;
        bool virtualSqlCompleteSemicolon = false;

        static const int autoCompleterDelay = 300;
        static const int queryParserDelay = 500;

    private slots:
        void customContextMenuRequested(const QPoint& pos);
        void updateUndoAction(bool enabled);
        void updateRedoAction(bool enabled);
        void updateCopyAction(bool enabled);
        void deleteSelected();
        void homePressed(Qt::KeyboardModifiers modifiers);
        void tabPressed(bool shiftPressed);
        void backspacePressed();
        void complete();
        void completeSelected();
        void scheduleAutoCompletion();
        void checkForAutoCompletion();
        void completerTypedText(const QString& text);
        void completerBackspacePressed();
        void completerLeftPressed();
        void completerRightPressed();
        void parseContents();
        void scheduleQueryParser();
        void updateLineNumberAreaWidth();
        void highlightCurrentLine();
        void updateLineNumberArea(const QRect&rect, int dy);
        void cursorMoved();
        void formatSql();
        void saveToFile();
        void loadFromFile();
        void deleteLine();
        void moveBlockDown(bool deleteOld = true);
        void moveBlockUp(bool deleteOld = true);
        void copyBlockDown();
        void copyBlockUp();
        void find();
        void findNext();
        void findPrevious();
        void replace();
        void found(int start, int end);
        void reachedEnd();
        void changeFont(const QVariant& font);
        void configModified();

    signals:
        void errorsChecked(bool haveErrors);
};


#endif // SQLEDITOR_H
