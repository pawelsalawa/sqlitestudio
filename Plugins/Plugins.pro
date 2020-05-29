TEMPLATE = subdirs

pluginsProject = 1

SUBDIRS += \
    CsvExport \
    CsvImport \
    FusionDarkStyle \
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
    DbSqliteCipher \
    DbSqliteWx \
    MultiEditorImage

win32: {
SUBDIRS += \
    DbSqliteSystemData
}
