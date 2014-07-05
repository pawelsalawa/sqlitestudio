#ifndef PDFEXPORT_H
#define PDFEXPORT_H

#include "pdfexport_global.h"
#include "plugins/genericexportplugin.h"

class QPagedPaintDevice;
class QPainter;
class QTextOption;
class QFont;

class PDFEXPORTSHARED_EXPORT PdfExport : public GenericExportPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("pdfexport.json")

    public:
        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        ExportManager::ExportProviderFlags getProviderFlags() const;
        void validateOptions();
        QString defaultFileExtension() const;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns,
                                      const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportQueryResultsRow(SqlResultsRowPtr row);
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable,
                         const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable,
                                const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool afterExportTable();
        bool exportTableRow(SqlResultsRowPtr data);
        bool beforeExportDatabase(const QString& database);
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex);
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger);
        bool exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr view);
        bool afterExportQueryResults();
        bool afterExportDatabase();
        bool isBinaryData() const;
        bool init();
        void deinit();

    protected:
        virtual QPagedPaintDevice* createPaintDevice(const QString& documentTitle);

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
        void beginDoc(const QString& title);
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
        void exportTableColumnsHeader(const QStringList& columns);
        void exportTableColumnRow(SqliteCreateTable::Column* column);
        void exportTableConstraintsRow(const QList<SqliteCreateTable::Constraint*>& constrList);
        void checkForDataRender();
        void flushObjectPages();
        void drawObjectTopLine();
        void drawObjectCellHeaderBackground(int x1, int y1, int x2, int y2);
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
        qreal mmToPoints(qreal sizeMM);

        QPagedPaintDevice* pagedWriter = nullptr;
        QPainter* painter = nullptr;
        QTextOption* textOption = nullptr;
        QFont* stdFont = nullptr;
        QFont* boldFont = nullptr;
        QFont* italicFont = nullptr;
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
        int maxRowHeight = 0;
        int maxColWidth = 0;
        int rowsToPrebuffer = 0;
        int currentPage = -1;
        int rowNum = 0;
        qreal pointsPerMm = 1.0;
        int lineWidth = 15;
        static QString bulletChar;

        // Configurable fields
        int padding = 0;
        bool printRowNum = true;
        int topMargin = 0;
        int rightMargin = 0;
        int leftMargin = 0;
        int bottomMargin = 0;
};

#endif // PDFEXPORT_H
