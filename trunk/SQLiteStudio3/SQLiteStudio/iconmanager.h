#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include "icon.h"
#include <QStringList>
#include <QHash>
#include <QIcon>
#include <QVariant>

class QMovie;

class IconManager
{
    public:
        DEF_ICONS(Icons, iconEnums,
            DEF_ICON(ACT_CLEAR,                         "act_clear")
            DEF_ICON(ACT_COPY,                          "act_copy")
            DEF_ICON(ACT_CUT,                           "act_cut")
            DEF_ICON(ACT_DEL_LINE,                      "act_del_line")
            DEF_ICON(ACT_DELETE,                        "act_delete")
            DEF_ICON(ACT_PASTE,                         "act_paste")
            DEF_ICON(ACT_REDO,                          "act_redo")
            DEF_ICON(ACT_SEARCH,                        "act_search")
            DEF_ICON(ACT_SELECT_ALL,                    "act_select_all")
            DEF_ICON(ACT_UNDO,                          "act_undo")
            DEF_ICON(APPLY_FILTER,                      "apply_filter")
            DEF_ICON(APPLY_FILTER_RE,                   "apply_filter_re")
            DEF_ICON(APPLY_FILTER_SQL,                  "apply_filter_sql")
            DEF_ICON(APPLY_FILTER_TXT,                  "apply_filter_txt")
            DEF_ICON(CLEAR_HISTORY,                     "clear_history")
            DEF_ICON(CLEAR_LINEEDIT,                    "clear_lineedit")
            DEF_ICON(COLUMN,                            "column")
            DEF_ICON(COLUMN_CONSTRAINT,                 "column_constraint")
            DEF_ICO2(COLUMN_CONSTRAINT_ADD,             COLUMN_CONSTRAINT, PLUS)
            DEF_ICO2(COLUMN_CONSTRAINT_DEL,             COLUMN_CONSTRAINT, MINUS)
            DEF_ICO2(COLUMN_CONSTRAINT_EDIT,            COLUMN_CONSTRAINT, EDIT)
            DEF_ICON(COLUMN_EDIT,                       "column_edit")
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
            DEF_ICON(CONFIGURE_CONSTRAINT,              "configure_constraint")
            DEF_ICON(CONSTRAINT_CHECK,                  "check")
            DEF_ICO2(CONSTRAINT_CHECK_ADD,              CONSTRAINT_CHECK, PLUS)
            DEF_ICON(CONSTRAINT_COLLATION,              "collation")
            DEF_ICO2(CONSTRAINT_COLLATION_ADD,          CONSTRAINT_COLLATION, PLUS)
            DEF_ICON(CONSTRAINT_DEFAULT,                "default")
            DEF_ICO2(CONSTRAINT_DEFAULT_ADD,            CONSTRAINT_DEFAULT, PLUS)
            DEF_ICON(CONSTRAINT_FOREIGN_KEY,            "fk")
            DEF_ICO2(CONSTRAINT_FOREIGN_KEY_ADD,        CONSTRAINT_FOREIGN_KEY, PLUS)
            DEF_ICON(CONSTRAINT_NOT_NULL,               "not_null")
            DEF_ICO2(CONSTRAINT_NOT_NULL_ADD,           CONSTRAINT_NOT_NULL, PLUS)
            DEF_ICON(CONSTRAINT_PRIMARY_KEY,            "pk")
            DEF_ICO2(CONSTRAINT_PRIMARY_KEY_ADD,        CONSTRAINT_PRIMARY_KEY, PLUS)
            DEF_ICON(CONSTRAINT_UNIQUE,                 "unique")
            DEF_ICO2(CONSTRAINT_UNIQUE_ADD,             CONSTRAINT_UNIQUE, PLUS)
            DEF_ICON(DATABASE,                          "database")
            DEF_ICO2(DATABASE_ADD,                      DATABASE, PLUS)
            DEF_ICON(DATABASE_CONNECT,                  "database_connect")
            DEF_ICON(DATABASE_CONNECTED,                "database_connected")
            DEF_ICO2(DATABASE_DEL,                      DATABASE, MINUS)
            DEF_ICON(DATABASE_DISCONNECT,               "database_disconnect")
            DEF_ICO2(DATABASE_EDIT,                     DATABASE, EDIT)
            DEF_ICON(DATABASE_FILE,                     "database_file")
            DEF_ICO2(DATABASE_INVALID,                  DATABASE, WARNING)
            DEF_ICON(DATABASE_NETWORK,                  "database_network")
            DEF_ICON(DATABASE_RELOAD,                   "database_reload")
            DEF_ICON(DDL_HISTORY,                       "ddl_history")
            DEF_ICON(DELETE_ROW,                        "delete_row")
            DEF_ICO3(DELETE_COLLATION,                  DELETE_ROW)
            DEF_ICO3(DELETE_FN_ARG,                     DELETE_ROW)
            DEF_ICO3(DELETE_FUNCTION,                   DELETE_ROW)
            DEF_ICON(DELETE_SELECTED,                   "delete_selected")
            DEF_ICON(DIRECTORY,                         "directory")
            DEF_ICO2(DIRECTORY_ADD,                     DIRECTORY, PLUS)
            DEF_ICO2(DIRECTORY_DEL,                     DIRECTORY, MINUS)
            DEF_ICO2(DIRECTORY_EDIT,                    DIRECTORY, EDIT)
            DEF_ICON(DIRECTORY_OPEN,                    "directory_open")
            DEF_ICON(DIRECTORY_OPEN_WITH_DB,            "directory_open_with_db")
            DEF_ICON(DIRECTORY_WITH_DB,                 "directory_with_db")
            DEF_ICON(ERASE,                             "erase")
            DEF_ICON(EXEC_QUERY,                        "exec_query")
            DEF_ICON(EXPLAIN_QUERY,                     "explain_query")
            DEF_ICON(EXPORT,                            "export")
            DEF_ICON(EXPORT_FILE_BROWSE,                "export_file_browse")
            DEF_ICON(FONT_BROWSE,                       "font_browse")
            DEF_ICON(FORMAT_SQL,                        "format_sql")
            DEF_ICON(FUNCTION,                          "function")
            DEF_ICON(HELP,                              "help")
            DEF_ICON(INDEX,                             "index")
            DEF_ICO2(INDEX_ADD,                         INDEX, PLUS)
            DEF_ICO2(INDEX_DEL,                         INDEX, MINUS)
            DEF_ICO2(INDEX_EDIT,                        INDEX, EDIT)
            DEF_ICON(INDEXES,                           "indexes")
            DEF_ICON(INDICATOR_ERROR,                   "indicator_error")
            DEF_ICON(INDICATOR_HINT,                    "indicator_hint")
            DEF_ICON(INDICATOR_INFO,                    "indicator_info")
            DEF_ICON(INDICATOR_WARN,                    "indicator_warn")
            DEF_ICON(INFO_BALLOON,                      "info_balloon")
            DEF_ICON(INSERT_ROW,                        "insert_row")
            DEF_ICON(INSERT_ROWS,                       "insert_rows")
            DEF_ICO3(INSERT_FN_ARG,                     INSERT_ROW)
            DEF_ICON(KEYWORD,                           "keyword")
            DEF_ICON(LOADING,                           "loading")
            DEF_ICON(MOVE_DOWN,                         "move_down")
            DEF_ICON(MOVE_UP,                           "move_up")
            DEF_ICO3(NEW_COLLATION,                     INSERT_ROW)
            DEF_ICO3(NEW_FUNCTION,                      INSERT_ROW)
            DEF_ICON(OPEN_SQL_EDITOR,                   "open_sql_editor")
            DEF_ICON(OPEN_SQL_FILE,                     "open_sql_file")
            DEF_ICON(OPEN_VALUE_EDITOR,                 "open_value_editor")
            DEF_ICON(PAGE_FIRST,                        "page_first")
            DEF_ICON(PAGE_LAST,                         "page_last")
            DEF_ICON(PAGE_NEXT,                         "page_next")
            DEF_ICON(PAGE_PREV,                         "page_prev")
            DEF_ICON(RELOAD,                            "reload")
            DEF_ICON(RENAME_FN_ARG,                     "rename_fn_arg")
            DEF_ICON(RESULTS_BELOW,                     "results_below")
            DEF_ICON(RESULTS_IN_TAB,                    "results_in_tab")
            DEF_ICON(ROLLBACK,                          "rollback")
            DEF_ICON(SAVE_SQL_FILE,                     "save_sql_file")
            DEF_ICON(SET_NULL,                          "set_null")
            DEF_ICON(SQLITESTUDIO_APP,                  "sqlitestudio")
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
        )

        static IconManager* getInstance();

        QString getFilePathForName(const QString& name);
        bool isMovie(const QString& name);
        QMovie* getMovie(const QString& name);
        QIcon* getIcon(const QString& name);
        void init();

    private:
        IconManager();
        void loadRecurently(QString dirPath, const QString& prefix, bool movie);

        static IconManager* instance;
        QHash<QString,QIcon*> icons;
        QHash<QString,QMovie*> movies;
        QHash<QString,QString> paths;
        QStringList iconDirs;
        QStringList iconFileExtensions;
        QStringList movieFileExtensions;
};

#define ICONS IconManager::getInstance()->iconEnums

#endif // ICONMANAGER_H
