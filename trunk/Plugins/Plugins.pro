TEMPLATE = subdirs

pluginsProject = 1

SUBDIRS += \
    CsvExport \
    CsvImport \
    DbSqlite2 \
    HtmlExport \
    PdfExport \
    SqlExport \
    SqlFormatterSimple \
    XmlExport \
    JsonExport \
    RegExpImport \
    Printing \
    SqlEnterpriseFormatter \
    ConfigMigration \
    ScriptingTcl \
    DbAndroid \
    DbSqliteCipher \
    DbSqliteWx

win32: {
SUBDIRS += \
    DbSqliteSystemData
}
