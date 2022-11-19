#ifndef PDFEXPORT_H
#define PDFEXPORT_H

#include "pdfexport_global.h"
#include "plugins/genericexportplugin.h"
#include "config_builder.h"
#include <QPdfWriter>
#include <QFont>
#include <QColor>

class QPagedPaintDevice;
class QPainter;
class QTextOption;
class QFont;

namespace Cfg
{
    PDFEXPORTSHARED_EXPORT QFont getPdfExportDefaultFont();
    PDFEXPORTSHARED_EXPORT QStringList getPdfPageSizes();
}

CFG_CATEGORIES(PdfExportConfig,
    CFG_CATEGORY(PdfExport,
        CFG_ENTRY(QString,     PageSize,         "A4")
        CFG_ENTRY(QStringList, PageSizes,        Cfg::getPdfPageSizes())
        CFG_ENTRY(int,         Padding,          1)
        CFG_ENTRY(bool,        PrintRowNum,      true)
        CFG_ENTRY(bool,        PrintPageNumbers, true)
        CFG_ENTRY(int,         TopMargin,        20)
        CFG_ENTRY(int,         RightMargin,      20)
        CFG_ENTRY(int,         BottomMargin,     20)
        CFG_ENTRY(int,         LeftMargin,       20)
        CFG_ENTRY(int,         MaxCellBytes,     100)
        CFG_ENTRY(QFont,       Font,             Cfg::getPdfExportDefaultFont())
        CFG_ENTRY(int,         FontSize,         10)
        CFG_ENTRY(QColor,      HeaderBgColor,    QColor(Qt::lightGray))
        CFG_ENTRY(QColor,      NullValueColor,   QColor(Qt::gray))
    )
)

class PDFEXPORTSHARED_EXPORT PdfExport : public GenericExportPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("pdfexport.json")

    public:
        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        ExportManager::ExportProviderFlags getProviderFlags() const;
        void validateOptions();
        CfgMain* getConfig();
        QString getExportConfigFormName() const;
        QString defaultFileExtension() const;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns,
                                      const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportQueryResultsRow(SqlResultsRowPtr row);
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable,
                         const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable,
                                const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool afterExport();
        bool afterExportTable();
        bool afterExportQueryResults();
        bool exportTableRow(SqlResultsRowPtr data);
        bool beforeExportDatabase(const QString& database);
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex);
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger);
        bool exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr view);
        void cleanupAfterExport();
        bool isBinaryData() const;
        bool init();
        void deinit();

    protected:
        virtual QPagedPaintDevice* createPaintDevice(const QString& documentTitle, bool& takeOwnership);

        int lineWidth = 15;

    private:
        struct DataCell
        {
            QString contents;
            Qt::Alignment alignment = Qt::AlignLeft;
            bool isNull = false;
            bool isRowNum = false;
        };

        struct DataRow
        {
            enum class Type
            {
                NORMAL,
                TOP_HEADER,
                COLUMNS_HEADER
            };

            QList<DataCell> cells;
            int height = 0;
            Type type = Type::NORMAL;
        };

        struct ObjectCell
        {
            enum class Type
            {
                NORMAL,
                LIST
            };

            QStringList contents;
            Qt::Alignment alignment = Qt::AlignLeft;
            bool headerBackground = false;
            bool bold = false;
            bool italic = false;
            Type type = Type::NORMAL;
        };

        struct ObjectRow
        {
            enum class Type
            {
                MULTI,
                SINGLE
            };

            QList<ObjectCell> cells;
            int height = 0;
            Type type = Type::SINGLE;
            bool recalculateColumnWidths = false;
        };

        void prepareTableDataExport(const QString& table, const QStringList& columnNames, const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        QList<int> getColumnDataLengths(int columnCount, const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool beginDoc(const QString& title);
        void endDoc();
        void setupConfig();
        void updateMargins();
        void drawDdl(const QString& objName, const QString& ddl);
        void clearDataHeaders();
        void resetDataTable();
        void exportDataHeader(const QString& contents);
        void exportDataColumnsHeader(const QStringList& columns);
        void exportDataRow(const QList<QVariant>& data);
        void exportObjectHeader(const QString& contents);
        void exportObjectColumnsHeader(const QStringList& columns);
        void exportTableColumnRow(SqliteCreateTable::Column* column);
        void exportTableConstraintsRow(const QList<SqliteCreateTable::Constraint*>& constrList);
        void exportObjectRow(const QStringList& values);
        void exportObjectRow(const QString& values);
        void checkForDataRender();
        void flushObjectPages();
        void drawObjectTopLine(int y);
        void drawObjectCellHeaderBackground(int x1, int y1, int x2, int y2);
        void drawFooter();
        void flushObjectRow(const ObjectRow& row, int y);
        void flushObjectCell(const ObjectCell& cell, int x, int y, int w, int h);
        void flushDataPages(bool forceRender = false);
        void flushDataRowsPage(int columnStart, int columnEndBefore, int rowsToRender);
        void flushDataRow(const DataRow& row, int& y, int columnStart, int columnEndBefore, int localRowNum);
        void flushDataCell(const QRect& rect, const DataCell& cell);
        void flushDataCell(const QRect& rect, const QString& contents, QTextOption* opt);
        void flushDataHeaderRow(const DataRow& row, int& y, int totalColsWidth, int columnStart, int columnEndBefore);
        void flushDataHeaderCell(int& x, int y, const DataRow& row, int col, QTextOption* opt);
        void renderPageNumber();
        int getPageNumberHeight();
        void newPage();
        void calculateDataColumnWidths(const QStringList& columnNames, const QList<int>& columnDataLengths, int columnToExpand = -1);
        void calculateDataRowHeights();
        int calculateRowHeight(int textWidth, const QString& contents);
        int calculateRowHeight(int maxTextWidth, const QStringList& listContents);
        int calculateBulletPrefixWidth();
        void calculateObjectColumnWidths(int columnToExpand = -1);
        int correctMaxObjectColumnWidths(int colCount, int columnToExpand);
        void calculateObjectRowHeights();
        int getDataColumnsWidth() const;
        int getDataColumnsStartX() const;
        int getContentsLeft() const;
        int getContentsTop() const;
        int getContentsRight() const;
        int getContentsBottom() const;
        qreal mmToPoints(qreal sizeMM);

        //CFG_LOCAL_PERSISTABLE(PdfExportConfig, cfg)
        QPagedPaintDevice* pagedWriter = nullptr;
        bool takeDeviceOwnership = true;
        QPainter* painter = nullptr;
        QTextOption* textOption = nullptr;
        QFont stdFont;
        QFont boldFont;
        QFont italicFont;
        int totalRows = 0;
        QList<ObjectRow> bufferedObjectRows; // object rows (for exporting ddl of all object)
        QList<DataRow> bufferedDataRows; // data rows
        int totalHeaderRowsHeight = 0; // total height of all current header rows
        int currentHeaderMinWidth = 0; // minimum width of the object header, required to extend data columns when hey are slimmer than this
        QList<int> calculatedObjectColumnWidths; // object column widths calculated basing on header column widths and columns from other object rows
        QList<int> calculatedDataColumnWidths; // data column widths calculated basing on header column widths and data column widths
        QList<int> columnsPerPage; // number of columns that will fit on each page
        QScopedPointer<DataRow> headerRow; // Top level header (object name)
        QScopedPointer<DataRow> columnsHeaderRow; // columns header for data tables
        int rowNumColumnWidth = 0;
        int pageWidth = 0;
        int pageHeight = 0;
        int minRowHeight = 0;
        int rowsToPrebuffer = 0;
        int currentPage = -1;
        int rowNum = 0;
        int lastRowY = 0;
        qreal pointsPerMm = 1.0;
        int maxColWidth = 0;
        int maxRowHeight = 0;
        int cellDataLimit = 100;

        static QString bulletChar;

        // Configurable fields
        int padding = 0;
        bool printRowNum = true;
        bool printPageNumbers = true;
        int topMargin = 0;
        int rightMargin = 0;
        int leftMargin = 0;
        int bottomMargin = 0;
};

#endif // PDFEXPORT_H
