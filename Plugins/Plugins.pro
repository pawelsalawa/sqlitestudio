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
    Printing

!win32: {
    SUBDIRS += \
        ScriptingTcl
}
