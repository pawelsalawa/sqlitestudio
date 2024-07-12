#ifndef SQLEDITOR_H
#define SQLEDITOR_H

#include "common/strhash.h"
#include "guiSQLiteStudio_global.h"
#include "common/extactioncontainer.h"
#include "sqlitesyntaxhighlighter.h"
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QFont>
#include <QHash>
#include <QMutex>
#include <QFuture>

class CompleterWindow;
class Parser;
class SqlEditor;
class SearchTextDialog;
class SearchTextLocator;
class LazyTrigger;
class Db;
class QTimer;

#ifdef Q_OS_OSX
#  define COMPLETE_REQ_KEY Qt::META
#else
#  define COMPLETE_REQ_KEY Qt::CTRL
#endif

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
    CFG_KEY_ENTRY(DELETE_LINE,     Qt::CTRL | Qt::Key_D,              QObject::tr("Delete current line"))
    CFG_KEY_ENTRY(COMPLETE,        COMPLETE_REQ_KEY | Qt::Key_Space,  QObject::tr("Request code assistant"))
    CFG_KEY_ENTRY(FORMAT_SQL,      Qt::CTRL | Qt::Key_T,              QObject::tr("Format contents"))
    CFG_KEY_ENTRY(MOVE_BLOCK_DOWN, Qt::ALT | Qt::Key_Down,            QObject::tr("Move selected block of text one line down"))
    CFG_KEY_ENTRY(MOVE_BLOCK_UP,   Qt::ALT | Qt::Key_Up,              QObject::tr("Move selected block of text one line up"))
    CFG_KEY_ENTRY(COPY_BLOCK_DOWN, Qt::ALT | Qt::CTRL | Qt::Key_Down, QObject::tr("Copy selected block of text and paste it a line below"))
    CFG_KEY_ENTRY(COPY_BLOCK_UP,   Qt::ALT | Qt::CTRL | Qt::Key_Up,   QObject::tr("Copy selected block of text and paste it a line above"))
    CFG_KEY_ENTRY(TOGGLE_COMMENT,  Qt::CTRL | Qt::Key_Slash,          QObject::tr("Toggle comment"))
    CFG_KEY_ENTRY(INCR_FONT_SIZE,  Qt::CTRL | Qt::Key_Plus,           QObject::tr("Increase font size", "sql editor"))
    CFG_KEY_ENTRY(DECR_FONT_SIZE,  Qt::CTRL | Qt::Key_Minus,          QObject::tr("Decrease font size", "sql editor"))
)

class GUI_API_EXPORT SqlEditor : public QPlainTextEdit, public ExtActionContainer
{
    Q_OBJECT

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
            SAVE_AS_SQL_FILE,
            DELETE_LINE,
            COMPLETE,
            MOVE_BLOCK_DOWN,
            MOVE_BLOCK_UP,
            COPY_BLOCK_DOWN,
            COPY_BLOCK_UP,
            FIND,
            FIND_NEXT,
            FIND_PREV,
            REPLACE,
            TOGGLE_COMMENT,
            WORD_WRAP,
            INCR_FONT_SIZE,
            DECR_FONT_SIZE
        };
        Q_ENUM(Action)

        enum ToolBar
        {
        };

        static void createStaticActions();
        static void staticInit();

        explicit SqlEditor(QWidget *parent = 0);
        ~SqlEditor();

        Db* getDb() const;
        void setDb(Db* value);
        void setAutoCompletion(bool enabled);
        QString getVirtualSqlExpression() const;
        void setVirtualSqlExpression(const QString& value);
        void setTriggerContext(const QString& table);
        bool haveErrors();
        bool isSyntaxChecked();
        bool getShowLineNumbers() const;
        void setShowLineNumbers(bool value);
        void checkSyntaxNow();
        void saveSelection();
        void restoreSelection();
        QToolBar* getToolBar(int toolbar) const;
        void setCurrentQueryHighlighting(bool enabled);
        bool getVirtualSqlCompleteSemicolon() const;
        void setVirtualSqlCompleteSemicolon(bool value);
        bool getHighlightingSyntax() const;
        void setOpenSaveActionsEnabled(bool value);

        static QHash<Action, QAction*> staticActions;
        static bool wrapWords;

        bool getAlwaysEnforceErrorsChecking() const;
        void setAlwaysEnforceErrorsChecking(bool newAlwaysEnforceErrorsChecking);

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
        void changeEvent(QEvent* e);
        void showEvent(QShowEvent* event);
        void dropEvent(QDropEvent* e);

    private:
        class LineNumberArea : public QWidget
        {
            public:
                explicit LineNumberArea(SqlEditor *editor);
                QSize sizeHint() const;

            protected:
                void paintEvent(QPaintEvent *event);

            private:
                SqlEditor *codeEditor = nullptr;
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
        void checkForSyntaxErrors();
        void checkForValidObjects();
        void setObjectLinks(bool enabled);
        void addDbObject(int from, int to, const QString& dbName);
        void clearDbObjects();
        void lineNumberAreaPaintEvent(QPaintEvent* event);
        int lineNumberAreaWidth();
        void highlightParenthesis(QList<QTextEdit::ExtraSelection>& selections);
        void highlightCurrentQuery(QList<QTextEdit::ExtraSelection>& selections);
        void highlightCurrentLine(QList<QTextEdit::ExtraSelection>& selections);
        void highlightCurrentCursorContext(bool delayedCall = false);
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
        const DbObject* getValidObjectForPosition(const QPoint& point);
        void handleValidObjectCursor(const QPoint& point);
        bool handleValidObjectContextMenu(const QPoint& pos);
        void saveToFile(const QString& fileName);
        void toggleLineCommentForLine(const QTextBlock& block);

        SqliteSyntaxHighlighter* highlighter = nullptr;
        QMenu* contextMenu = nullptr;
        QMenu* validObjContextMenu = nullptr;
        Db* db = nullptr;
        CompleterWindow* completer = nullptr;
        LazyTrigger* autoCompleteTrigger = nullptr;
        bool autoCompletion = true;
        bool deletionKeyPressed = false;
        LazyTrigger* queryParserTrigger = nullptr;
        Parser* queryParser = nullptr;
        StrHash<QStringList> objectsInNamedDb;
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
        bool richFeaturesEnabled = true;
        bool alwaysEnforceErrorsChecking = false;
        bool highlightingSyntax = true;
        QBrush currentQueryBrush;
        QTimer* currentQueryTimer = nullptr;
        bool openSaveActionsEnabled = true;

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
        QString createTriggerTable;
        QString loadedFile;
        QFutureWatcher<QHash<QString,QStringList>>* objectsInNamedDbWatcher = nullptr;
        void changeFontSize(int factor);

        static const int autoCompleterDelay = 300;
        static const int queryParserDelay = 500;

    private slots:
        void highlightSyntax();
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
//        void scheduleAutoCompletion();
        void checkForAutoCompletion();
        void completerTypedText(const QString& text);
        void completerBackspacePressed();
        void completerLeftPressed();
        void completerRightPressed();
        void parseContents();
        void scheduleQueryParserForSchemaRefresh();
        void scheduleQueryParser(bool force = false, bool skipCompleter = false);
        void updateLineNumberAreaWidth();
        void updateLineNumberArea(const QRect&rect, int dy);
        void cursorMoved();
        void checkContentSize();
        void formatSql();
        void saveToFile();
        void saveAsToFile();
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
        void toggleComment();
        void wordWrappingChanged(const QVariant& value);
        void currentCursorContextDelayedHighlight();
        void fontSizeChangeRequested(int delta);
        void incrFontSize();
        void decrFontSize();
        void moveCursorTo(int pos);

    public slots:
        void colorsConfigChanged();
        void refreshValidObjects();

    signals:
        void errorsChecked(bool haveErrors);
};


#endif // SQLEDITOR_H
