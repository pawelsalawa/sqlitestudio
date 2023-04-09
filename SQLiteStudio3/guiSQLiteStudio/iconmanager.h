#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include "icon.h"
#include "guiSQLiteStudio_global.h"
#include <QStringList>
#include <QHash>
#include <QIcon>
#include <QVariant>

class QMovie;
class PluginType;
class Plugin;

class GUI_API_EXPORT IconManager : public QObject
{
        Q_OBJECT

    public:
        DEF_ICONS(Icons, iconEnums,
            DEF_ICON(ACT_ABORT,                         "act_abort")
            DEF_ICON(ACT_CLEAR,                         "act_clear")
            DEF_ICON(ACT_COPY,                          "act_copy")
            DEF_ICON(ACT_CUT,                           "act_cut")
            DEF_ICON(ACT_DEL_LINE,                      "act_del_line")
            DEF_ICON(ACT_DELETE,                        "act_delete")
            DEF_ICON(ACT_PASTE,                         "act_paste")
            DEF_ICON(ACT_REDO,                          "act_redo")
            DEF_ICON(ACT_SELECT_ALL,                    "act_select_all")
            DEF_ICON(ACT_UNDO,                          "act_undo")
            DEF_ICON(APPLY_FILTER,                      "apply_filter")
            DEF_ICON(APPLY_FILTER_RE,                   "apply_filter_re")
            DEF_ICON(APPLY_FILTER_SQL,                  "apply_filter_sql")
            DEF_ICON(APPLY_FILTER_TXT,                  "apply_filter_txt")
            DEF_ICON(APPLY_FILTER_TXT_STRICT,           "apply_filter_txt_strict")
            DEF_ICON(BUG,                               "bug")
            DEF_ICON(BUG_LIST,                          "bug_list")
            DEF_ICON(CLEAR_HISTORY,                     "clear_history")
            DEF_ICON(CLEAR_LINEEDIT,                    "clear_lineedit")
            DEF_ICON(CLOSE,                             "close")
            DEF_ICON(CODE_ASSISTANT,                    "code_assistant")
            DEF_ICON(CODE_SNIPPET,                      "code_snippet")
            DEF_ICON(COLUMN,                            "column")
            DEF_ICON(COLUMN_CONSTRAINT,                 "column_constraint")
            DEF_ICO2(COLUMN_CONSTRAINT_ADD,             COLUMN_CONSTRAINT, PLUS)
            DEF_ICO2(COLUMN_CONSTRAINT_DEL,             COLUMN_CONSTRAINT, MINUS)
            DEF_ICO2(COLUMN_CONSTRAINT_EDIT,            COLUMN_CONSTRAINT, EDIT)
            DEF_ICON(COLUMNS,                           "columns")
            DEF_ICON(COMMIT,                            "commit")
            DEF_ICON(COMPLETE,                          "complete")
            DEF_ICON(COMPLETER_BLOB,                    "completer_blob")
            DEF_ICON(COMPLETER_NO_VALUE,                "completer_no_value")
            DEF_ICON(COMPLETER_NUMBER,                  "completer_number")
            DEF_ICON(COMPLETER_OPERATOR,                "completer_operator")
            DEF_ICON(COMPLETER_OTHER,                   "completer_other")
            DEF_ICON(COMPLETER_PRAGMA,                  "completer_pragma")
            DEF_ICON(COMPLETER_STRING,                  "completer_string")
            DEF_ICON(CONFIGURE,                         "configure")
            DEF_ICON(CONSTRAINT_CHECK,                  "check")
            DEF_ICO2(CONSTRAINT_CHECK_ADD,              CONSTRAINT_CHECK, PLUS)
            DEF_ICON(CONSTRAINT_COLLATION,              "collation")
            DEF_ICO2(CONSTRAINT_COLLATION_ADD,          CONSTRAINT_COLLATION, PLUS)
            DEF_ICON(CONSTRAINT_DEFAULT,                "default")
            DEF_ICO2(CONSTRAINT_DEFAULT_ADD,            CONSTRAINT_DEFAULT, PLUS)
            DEF_ICON(CONSTRAINT_FOREIGN_KEY,            "fk")
            DEF_ICO2(CONSTRAINT_FOREIGN_KEY_ADD,        CONSTRAINT_FOREIGN_KEY, PLUS)
            DEF_ICON(CONSTRAINT_GENERATED,              "generated")
            DEF_ICO2(CONSTRAINT_GENERATED_STORED,       CONSTRAINT_GENERATED, DISK)
            DEF_ICO2(CONSTRAINT_GENERATED_VIRTUAL,      CONSTRAINT_GENERATED, LIGHTENING)
            DEF_ICO2(CONSTRAINT_GENERATED_ADD,          CONSTRAINT_GENERATED, PLUS)
            DEF_ICON(CONSTRAINT_NOT_NULL,               "not_null")
            DEF_ICO2(CONSTRAINT_NOT_NULL_ADD,           CONSTRAINT_NOT_NULL, PLUS)
            DEF_ICON(CONSTRAINT_PRIMARY_KEY,            "pk")
            DEF_ICO2(CONSTRAINT_PRIMARY_KEY_ADD,        CONSTRAINT_PRIMARY_KEY, PLUS)
            DEF_ICON(CONSTRAINT_UNIQUE,                 "unique")
            DEF_ICO2(CONSTRAINT_UNIQUE_ADD,             CONSTRAINT_UNIQUE, PLUS)
            DEF_ICON(VACUUM_DB,                         "vacuum_db")
            DEF_ICON(INTEGRITY_CHECK,                   "integrity_check")
            DEF_ICON(DATABASE,                          "database")
            DEF_ICO2(DATABASE_ADD,                      DATABASE, PLUS)
            DEF_ICON(DATABASE_CONNECT,                  "database_connect")
            DEF_ICO2(DATABASE_DEL,                      DATABASE, MINUS)
            DEF_ICON(DATABASE_DISCONNECT,               "database_disconnect")
            DEF_ICO2(DATABASE_EDIT,                     DATABASE, EDIT)
            DEF_ICON(DATABASE_EXPORT,                   "database_export")
            DEF_ICON(DATABASE_EXPORT_WIZARD,            "database_export_wizard")
            DEF_ICON(DATABASE_IMPORT_WIZARD,            "database_import_wizard")
            DEF_ICON(DATABASE_OFFLINE,                  "database_offline")
            DEF_ICO2(DATABASE_INVALID,                  DATABASE_OFFLINE, WARNING)
            DEF_ICON(DATABASE_ONLINE,                   "database_online")
            DEF_ICON(DATABASE_RELOAD,                   "database_reload")
            DEF_ICON(DDL_HISTORY,                       "ddl_history")
            DEF_ICON(DELETE,                            "delete")
            DEF_ICON(DELETE_ROW,                        "delete_row")
            DEF_ICO3(DELETE_COLLATION,                  DELETE_ROW)
            DEF_ICO3(DELETE_DATATYPE,                   DELETE_ROW)
            DEF_ICO3(DELETE_FN_ARG,                     DELETE_ROW)
            DEF_ICO3(DELETE_FUNCTION,                   DELETE_ROW)
            DEF_ICON(DELETE_SELECTED,                   "delete_selected")
            DEF_ICON(DIRECTORY,                         "directory")
            DEF_ICO2(DIRECTORY_ADD,                     DIRECTORY, PLUS)
            DEF_ICO2(DIRECTORY_DEL,                     DIRECTORY, MINUS)
            DEF_ICO2(DIRECTORY_EDIT,                    DIRECTORY, EDIT)
            DEF_ICON(DIRECTORY_OPEN,                    "directory_open")
            DEF_ICON(DIRECTORY_OPEN_WITH_DB,            "directory_open_with_db")
            DEF_ICO3(DONATE,                            TEST_CONN_OK)
            DEF_ICON(ERASE,                             "erase")
            DEF_ICON(ERASE_TABLE_DATA,                  "erase_table_data")
            DEF_ICON(EXEC_QUERY,                        "exec_query")
            DEF_ICON(EXEC_SQL_FROM_FILE,                "execute_sql_from_file")
            DEF_ICON(EXPLAIN_QUERY,                     "explain_query")
            DEF_ICON(EXPORT,                            "export")
            DEF_ICON(EXPORT_FILE_BROWSE,                "export_file_browse")
            DEF_ICON(EXTENSION,                         "brick")
            DEF_ICON(EXTENSION_ADD,                     "brick_add")
            DEF_ICON(EXTENSION_DELETE,                  "brick_delete")
            DEF_ICON(EXTENSION_ERROR,                   "brick_error")
            DEF_ICON(FEATURE_REQUEST,                   "feature_request")
            DEF_ICON(FONT_BROWSE,                       "font_browse")
            DEF_ICON(FORMAT_SQL,                        "format_sql")
            DEF_ICON(FUNCTION,                          "function")
            DEF_ICON(GET_UPDATE,                        "get_update")
            DEF_ICON(GO_BACK,                           "go_back")
            DEF_ICON(GENERATE_COLUMNS,                  "wand")
            DEF_ICO3(GENERATE_QUERY,                    GENERATE_COLUMNS)
            DEF_ICON(HELP,                              "help")
            DEF_ICON(HOMEPAGE,                          "homepage")
            DEF_ICON(IMPORT,                            "import")
            DEF_ICON(INDEX,                             "index")
            DEF_ICO2(INDEX_ADD,                         INDEX, PLUS)
            DEF_ICO2(INDEX_DEL,                         INDEX, MINUS)
            DEF_ICO2(INDEX_EDIT,                        INDEX, EDIT)
            DEF_ICON(INDEXES,                           "indexes")
            DEF_ICON(INDICATOR_ERROR,                   "indicator_error")
            DEF_ICON(INDICATOR_HINT,                    "indicator_hint")
            DEF_ICON(INDICATOR_INFO,                    "indicator_info")
            DEF_ICON(INDICATOR_WARN,                    "indicator_warn")
            DEF_ICON(INDEX_EXPR_ADD,                    "tag_hash_add")
            DEF_ICON(INDEX_EXPR_EDIT,                   "tag_hash_edit")
            DEF_ICON(INDEX_EXPR_DEL,                    "tag_hash_del")
            DEF_ICON(INSERT_ROW,                        "insert_row")
            DEF_ICON(INSERT_ROWS,                       "insert_rows")
            DEF_ICO3(INSERT_FN_ARG,                     INSERT_ROW)
            DEF_ICO3(INSERT_DATATYPE,                   INSERT_ROW)
            DEF_ICON(KEYWORD,                           "keyword")
            DEF_ICON(LICENSES,                          "licenses")
            DEF_ICON(LOADING,                           "loading")
            DEF_ICON(MOVE_DOWN,                         "move_down")
            DEF_ICON(MOVE_UP,                           "move_up")
            DEF_ICO3(NEW_COLLATION,                     INSERT_ROW)
            DEF_ICO3(NEW_FUNCTION,                      INSERT_ROW)
            DEF_ICON(OPEN_FILE,                         "open_sql_file")
            DEF_ICON(OPEN_SQL_EDITOR,                   "open_sql_editor")
            DEF_ICO3(OPEN_SQL_FILE,                     OPEN_FILE)
            DEF_ICON(OPEN_VALUE_EDITOR,                 "open_value_editor")
            DEF_ICON(PAGE_FIRST,                        "page_first")
            DEF_ICON(PAGE_LAST,                         "page_last")
            DEF_ICON(PAGE_NEXT,                         "page_next")
            DEF_ICON(PAGE_PREV,                         "page_prev")
            DEF_ICO3(MOVE_LEFT,                         PAGE_PREV)
            DEF_ICO3(MOVE_RIGHT,                        PAGE_NEXT)
            DEF_ICON(PLUS,                              "plus")
            DEF_ICON(QUIT,                              "quit")
            DEF_ICON(RELOAD,                            "reload")
            DEF_ICON(RENAME_FN_ARG,                     "rename_fn_arg")
            DEF_ICO3(RENAME_DATATYPE,                   RENAME_FN_ARG)
            DEF_ICON(RESET_AUTOINCREMENT,               "reset_autoincrement")
            DEF_ICON(RESTORE_DEFAULT,                   "restore_default")
            DEF_ICON(RESULTS_BELOW,                     "results_below")
            DEF_ICON(RESULTS_IN_TAB,                    "results_in_tab")
            DEF_ICON(ROLLBACK,                          "rollback")
            DEF_ICON(SAVE_FILE,                         "save_file")
            DEF_ICON(SAVE_SQL_FILE,                     "save_sql_file")
            DEF_ICON(SEARCH,                            "search")
            DEF_ICON(SEARCH_AND_REPLACE,                "search_and_replace")
            DEF_ICON(SELECTION_INVERT,                  "selection_invert")
            DEF_ICON(SET_NULL,                          "set_null")
            DEF_ICON(SORT_COLUMNS,                      "sort_columns")
            DEF_ICON(SORT_COUNT_01,                     "sort_cnt_01")
            DEF_ICON(SORT_COUNT_02,                     "sort_cnt_02")
            DEF_ICON(SORT_COUNT_03,                     "sort_cnt_03")
            DEF_ICON(SORT_COUNT_04,                     "sort_cnt_04")
            DEF_ICON(SORT_COUNT_05,                     "sort_cnt_05")
            DEF_ICON(SORT_COUNT_06,                     "sort_cnt_06")
            DEF_ICON(SORT_COUNT_07,                     "sort_cnt_07")
            DEF_ICON(SORT_COUNT_08,                     "sort_cnt_08")
            DEF_ICON(SORT_COUNT_09,                     "sort_cnt_09")
            DEF_ICON(SORT_COUNT_10,                     "sort_cnt_10")
            DEF_ICON(SORT_COUNT_11,                     "sort_cnt_11")
            DEF_ICON(SORT_COUNT_12,                     "sort_cnt_12")
            DEF_ICON(SORT_COUNT_13,                     "sort_cnt_13")
            DEF_ICON(SORT_COUNT_14,                     "sort_cnt_14")
            DEF_ICON(SORT_COUNT_15,                     "sort_cnt_15")
            DEF_ICON(SORT_COUNT_16,                     "sort_cnt_16")
            DEF_ICON(SORT_COUNT_17,                     "sort_cnt_17")
            DEF_ICON(SORT_COUNT_18,                     "sort_cnt_18")
            DEF_ICON(SORT_COUNT_19,                     "sort_cnt_19")
            DEF_ICON(SORT_COUNT_20,                     "sort_cnt_20")
            DEF_ICON(SORT_COUNT_20_PLUS,                "sort_cnt_20p")
            DEF_ICON(SORT_RESET,                        "sort_reset")
            DEF_ICON(SQLITE_DOCS,                       "sqlite_docs")
            DEF_ICON(SQLITESTUDIO_APP,                  "sqlitestudio")
            DEF_ICON(SQLITESTUDIO_APP16,                "sqlitestudio_16")
            DEF_ICON(STATUS_ERROR,                      "status_error")
            DEF_ICON(STATUS_INFO,                       "status_info")
            DEF_ICON(STATUS_WARNING,                    "status_warn")
            DEF_ICON(TABLE,                             "table")
            DEF_ICO2(TABLE_ADD,                         TABLE, PLUS)
            DEF_ICON(TABLE_COLUMN_ADD,                  "table_column_add")
            DEF_ICON(TABLE_COLUMN_DELETE,               "table_column_delete")
            DEF_ICON(TABLE_COLUMN_EDIT,                 "table_column_edit")
            DEF_ICON(TABLE_CONSTRAINT,                  "table_constraint")
            DEF_ICO2(TABLE_CONSTRAINT_ADD,              TABLE_CONSTRAINT, PLUS)
            DEF_ICO2(TABLE_CONSTRAINT_DELETE,           TABLE_CONSTRAINT, MINUS)
            DEF_ICO2(TABLE_CONSTRAINT_EDIT,             TABLE_CONSTRAINT, EDIT)
            DEF_ICON(TABLE_CREATE_SIMILAR,              "table_create_similar")
            DEF_ICO2(TABLE_DEL,                         TABLE, MINUS)
            DEF_ICO2(TABLE_EDIT,                        TABLE, EDIT)
            DEF_ICON(TABLE_EXPORT,                      "table_export")
            DEF_ICON(TABLE_IMPORT,                      "table_import")
            DEF_ICON(TABLE_POPULATE,                    "table_populate")
            DEF_ICON(TABLES,                            "tables")
            DEF_ICON(TABS_AT_BOTTOM,                    "tabs_at_bottom")
            DEF_ICON(TABS_ON_TOP,                       "tabs_on_top")
            DEF_ICON(TEST_CONN_ERROR,                   "test_conn_error")
            DEF_ICON(TEST_CONN_OK,                      "test_conn_ok")
            DEF_ICON(TRIGGER,                           "trigger")
            DEF_ICO2(TRIGGER_ADD,                       TRIGGER, PLUS)
            DEF_ICON(TRIGGER_COLUMNS,                   "trigger_columns")
            DEF_ICO2(TRIGGER_COLUMNS_INVALID,           TRIGGER_COLUMNS, WARNING)
            DEF_ICO2(TRIGGER_DEL,                       TRIGGER, MINUS)
            DEF_ICO2(TRIGGER_EDIT,                      TRIGGER, EDIT)
            DEF_ICON(TRIGGERS,                          "triggers")
            DEF_ICON(USER_MANUAL,                       "user_manual")
            DEF_ICON(VIEW,                              "view")
            DEF_ICO2(VIEW_ADD,                          VIEW, PLUS)
            DEF_ICO2(VIEW_DEL,                          VIEW, MINUS)
            DEF_ICO2(VIEW_EDIT,                         VIEW, EDIT)
            DEF_ICON(VIEWS,                             "views")
            DEF_ICON(VIRTUAL_TABLE,                     "virtual_table")
            DEF_ICON(WIN_CASCADE,                       "win_cascade")
            DEF_ICON(WIN_TILE,                          "win_tile")
            DEF_ICON(WIN_TILE_HORIZONTAL,               "win_tile_horizontal")
            DEF_ICON(WIN_TILE_VERTICAL,                 "win_tile_vertical")
            DEF_ICON(WIN_CLOSE,                         "window_close")
            DEF_ICON(WIN_CLOSE_ALL,                     "window_close_all")
            DEF_ICON(WIN_CLOSE_ALL_LEFT,                "window_close_all_left")
            DEF_ICON(WIN_CLOSE_ALL_RIGHT,               "window_close_all_right")
            DEF_ICON(WIN_CLOSE_OTHER,                   "window_close_other")
            DEF_ICON(WIN_RESTORE,                       "window_restore")
            DEF_ICON(WIN_RENAME,                        "window_rename")
            DEF_ICON(ZOOM_RESET,                        "zoom")
            DEF_ICON(ZOOM_IN,                           "zoom_in")
            DEF_ICON(ZOOM_OUT,                          "zoom_out")
        )

        static IconManager* getInstance();

        QString getFilePathForName(const QString& name);
        bool isMovie(const QString& name);
        QMovie* getMovie(const QString& name);
        QIcon* getIcon(const QString& name);
        void init();
        QStringList getIconDirs() const;

    private:
        IconManager();
        void loadRecurently(QString dirPath, const QString& prefix, bool movie);
        void enableRescanning();

        static IconManager* instance;
        QHash<QString,QIcon*> icons;
        QHash<QString,QMovie*> movies;
        QHash<QString,QString> paths;
        QStringList iconDirs;
        QStringList iconFileExtensions;
        QStringList movieFileExtensions;
        QStringList resourceIcons;
        QStringList resourceMovies;

    private slots:
        void rescanResources(Plugin* plugin, PluginType* pluginType);
        void pluginsAboutToMassUnload();
        void pluginsInitiallyLoaded();

    public slots:
        void rescanResources(const QString& pluginName = QString());

    signals:
        void rescannedFor(const QString& pluginName);
};

#define ICONMANAGER IconManager::getInstance()
#define ICONS ICONMANAGER->iconEnums

#endif // ICONMANAGER_H
