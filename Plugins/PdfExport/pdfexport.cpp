#include "pdfexport.h"
#include <QTextDocument>
#include <QPainter>
#include <QPdfWriter>
#include <QFont>
#include <QDebug>

bool PdfExport::init()
{
    textOption = new QTextOption();
    textOption->setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    return GenericExportPlugin::init();
}

void PdfExport::deinit()
{
    safe_delete(textOption);
}

QString PdfExport::getFormatName() const
{
    return "PDF";
}

ExportManager::StandardConfigFlags PdfExport::standardOptionsToEnable() const
{
    return 0;
}

ExportManager::ExportProviderFlags PdfExport::getProviderFlags() const
{
    return ExportManager::DATA_LENGTHS;
}

void PdfExport::validateOptions()
{
}

QString PdfExport::defaultFileExtension() const
{
    return "pdf";
}

bool PdfExport::beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    beginDoc(tr("SQL query results"));

    QStringList columnNames;
    for (const QueryExecutor::ResultColumnPtr& col : columns)
        columnNames << col->displayName;

    currentHeaderRows.clear();
    exportColumnsHeader(columnNames);
    calculateColumnWidths(columnNames, providedData);
    return true;
}

bool PdfExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    exportDataRow(row->valueList());
    return true;
}

bool PdfExport::afterExportQueryResults()
{
    endDoc();
    return true;
}

bool PdfExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    if (isTableExport())
        beginDoc(tr("Exported table: %1").arg(table));

    currentHeaderRows.clear();
    exportObjectHeader(tr("Table: %1").arg(table));
    exportColumnsHeader(columnNames);
    calculateColumnWidths(columnNames, providedData);

    return true;
}

bool PdfExport::exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    if (isTableExport())
        beginDoc(tr("Exported table: %1").arg(table));

    return true;
}

bool PdfExport::exportTableRow(SqlResultsRowPtr data)
{
    exportDataRow(data->valueList());
    return true;
}

bool PdfExport::afterExportTable()
{
    if (isTableExport())
        endDoc();

    return true;
}

bool PdfExport::beforeExportDatabase(const QString& database)
{
    beginDoc(tr("Exported database: %1").arg(database));
    return true;
}

bool PdfExport::exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex)
{
    return true;
}

bool PdfExport::exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger)
{
    return true;
}

bool PdfExport::exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr view)
{
    return true;
}

bool PdfExport::afterExportDatabase()
{
    endDoc();
    return true;
}

bool PdfExport::isBinaryData() const
{
    return true;
}

void PdfExport::beginDoc(const QString& title)
{
    safe_delete(painter);
    safe_delete(pdfWriter);
    safe_delete(stdFont);
    safe_delete(boldFont);
    pdfWriter = new QPdfWriter(output);
    painter = new QPainter(pdfWriter);
    painter->setBrush(Qt::NoBrush);
    stdPen = new QPen(Qt::black, 15);
    painter->setPen(*stdPen);
    pdfWriter->setTitle(title);
    pdfWriter->setCreator(tr("SQLiteStudio v%1").arg(SQLITESTUDIO->getVersionString()));

    setupConfig();
}

void PdfExport::endDoc()
{
    renderDataPages(true);
    safe_delete(painter);
    safe_delete(pdfWriter);
}

void PdfExport::setupConfig()
{
    pdfWriter->setPageSize(QPdfWriter::A4);
    pageWidth = pdfWriter->width();
    pageHeight = pdfWriter->height();

    maxColWidth = pageWidth / 5;
    padding = 50;

    stdFont = new QFont(painter->font());
    stdFont->setPointSize(10);
    boldFont = new QFont(*stdFont);
    boldFont->setBold(true);
    italicFont = new QFont(*stdFont);
    italicFont->setItalic(true);

    QRectF rect = painter->boundingRect(QRectF(padding, padding, pageWidth - 2 * padding, 1), "X", *textOption);
    minRowHeight = rect.height() + padding * 2;
    maxRowHeight = qMax((int)(pageHeight * 0.2), minRowHeight);
    rowsToPrebuffer = (int)ceil((double)pageHeight / minRowHeight);

    currentPage = -1;
}

void PdfExport::exportDataRow(const QList<QVariant>& data)
{
    Cell cell;
    Row row;
    for (const QVariant& value : data)
    {
        switch (value.type())
        {
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::LongLong:
            case QVariant::ULongLong:
            case QVariant::Double:
                cell.alignment = Qt::AlignRight;
                break;
            default:
                cell.alignment = Qt::AlignLeft;
                break;
        }

        if (value.isNull())
        {
            cell.alignment = Qt::AlignCenter;
            cell.isNull = true;
            cell.contents = QStringLiteral("NULL");
        }
        else
        {
            cell.isNull = false;
            cell.contents = value.toString();
        }
        row.cells << cell;
    }

    bufferedRows << row;
    checkForDataRender();
}

void PdfExport::checkForDataRender()
{
    if (bufferedRows.size() >= rowsToPrebuffer)
        renderDataPages();
}

void PdfExport::renderDataPages(bool finalRender)
{
    calculateRowHeights();

    int rowsToRender = 0;
    int totalRowHeight = 0;
    int colStartAt = 0;
    while ((bufferedRows.size() >= rowsToPrebuffer) || (finalRender && bufferedRows.size() > 0))
    {
        // Calculate how many rows we can render on single page
        rowsToRender = 0;
        totalRowHeight = totalHeaderRowsHeight;
        for (const Row& row : bufferedRows)
        {
            rowsToRender++;
            totalRowHeight += row.height;
            if (totalRowHeight >= pageHeight)
            {
                rowsToRender--;
                break;
            }
        }

        // Render limited number of columns and rows per single page
        colStartAt = 0;
        for (int cols : columnsPerPage)
        {
            newPage();
            renderDataRowsPage(colStartAt, colStartAt + cols, rowsToRender);
            colStartAt += cols;
        }

        for (int i = 0; i < rowsToRender; i++)
            bufferedRows.removeFirst();
    }
}

void PdfExport::renderDataRowsPage(int columnStart, int columnEndBefore, int rowsToRender)
{
    // Calculating width of all columns on this page
    int totalColsWidth = sum(calculatedColumnWidths.mid(columnStart, columnEndBefore - columnStart));

    // Draw header background
    int x = 0;
    int y = 0;
    painter->setBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
    painter->setPen(Qt::NoPen);
    painter->drawRect(QRect(x, y, totalColsWidth, totalHeaderRowsHeight));
    painter->setBrush(Qt::NoBrush);
    painter->setPen(*stdPen);

    // Draw horizontal lines
    y = 0;
    painter->drawLine(0, y, totalColsWidth, y);
    for (const Row& row : currentHeaderRows + bufferedRows.mid(0, rowsToRender))
    {
        y += row.height;
        painter->drawLine(0, y, totalColsWidth, y);
    }

    // Finding first row to start vertical lines from. It's either a COLUMNS_HEADER, or first data row, after headers.
    int verticalLinesStart = 0;
    for (const Row& row : currentHeaderRows)
    {
        if (row.type == Row::COLUMNS_HEADER)
            break;

        verticalLinesStart += row.height;
    }

    // Draw vertical lines
    int totalRowsHeight = y;
    x = 0;
    painter->drawLine(x, 0, x, totalRowsHeight);
    for (int col = columnStart; col < columnEndBefore; col++)
    {
        x += calculatedColumnWidths[col];
        painter->drawLine(x, (col+1 == columnEndBefore) ? 0 : verticalLinesStart, x, totalRowsHeight);
    }

    // Draw header rows
    y = 0;
    for (const Row& row : currentHeaderRows)
        renderHeaderRow(row, y, totalColsWidth, columnStart, columnEndBefore);

    // Draw data
    for (int rowCounter = 0; rowCounter < rowsToRender && !bufferedRows.isEmpty(); rowCounter++)
        renderDataRow(bufferedRows.first(), y, columnStart, columnEndBefore);
}

void PdfExport::renderDataRow(const Row& row, int& y, int columnStart, int columnEndBefore)
{
    int textWidth = 0;
    int textHeight = 0;
    int colWidth = 0;
    int x = 0;

    y += padding;
    for (int col = columnStart; col < columnEndBefore; col++)
    {
        const Cell& cell = row.cells[col];
        colWidth = calculatedColumnWidths[col];

        x += padding;
        textWidth = colWidth - padding * 2;
        textHeight = row.height - padding * 2;
        renderDataCell(QRect(x, y, textWidth, textHeight), cell);
        x += colWidth - padding;
    }
    y += row.height - padding;
}

void PdfExport::renderDataCell(const QRect& rect, const PdfExport::Cell& cell)
{
    QTextOption opt = *textOption;
    opt.setAlignment(cell.alignment);

    if (cell.isNull)
    {
        painter->setPen(Qt::gray);
        painter->setFont(*italicFont);
    }

    painter->drawText(rect, cell.contents, opt);

    if (cell.isNull)
    {
        painter->setPen(Qt::black);
        painter->setFont(*stdFont);
    }
}

void PdfExport::renderHeaderRow(const PdfExport::Row& row, int& y, int totalColsWidth, int columnStart, int columnEndBefore)
{
    QTextOption opt = *textOption;
    opt.setAlignment(Qt::AlignHCenter);
    int x = 0;
    y += padding;
    switch (row.type)
    {
        case Row::HEADER:
        {
            x += padding;
            painter->setFont(*boldFont);
            painter->drawText(QRect(x, y, totalColsWidth - 2 * padding, row.height - 2 * padding), row.cells.first().contents, opt);
            painter->setFont(*stdFont);
            break;
        }
        case Row::COLUMNS_HEADER:
        {
            for (int col = columnStart; col < columnEndBefore; col++)
            {
                x += padding;
                painter->drawText(QRect(x, y, calculatedColumnWidths[col] - 2 * padding, row.height - 2 * padding), row.cells[col].contents, opt);
                x += calculatedColumnWidths[col] - padding;
            }
            break;
        }
        case Row::NORMAL:
            break; // no-op
    }
    y += row.height - padding;
}

void PdfExport::exportObjectHeader(const QString& contents)
{
    Row row;
    row.type = Row::HEADER;

    Cell cell;
    cell.contents = contents;
    cell.alignment = Qt::AlignHCenter;
    row.cells << cell;

    currentHeaderRows << row;
}

void PdfExport::exportColumnsHeader(const QStringList& columns)
{
    Row row;
    row.type = Row::COLUMNS_HEADER;

    Cell cell;
    for (const QString& col : columns)
    {
        cell.contents = col;
        cell.alignment = Qt::AlignHCenter;
        row.cells << cell;
    }

    currentHeaderRows << row;
}

void PdfExport::newPage()
{
    if (currentPage < 0)
    {
        currentPage = 0;
        return;
    }

    pdfWriter->newPage();
    currentPage++;
}

void PdfExport::calculateColumnWidths(const QStringList& columnNames, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    static const QString tplChar = "W";

    // Text options for calculating widths will not allow word wrapping
    QTextOption opt = *textOption;
    opt.setWrapMode(QTextOption::NoWrap);

    // Calculate header width first
    currentHeaderMinWidth = 0;
    if (!currentHeaderRows.isEmpty() && currentHeaderRows.first().type == Row::HEADER)
    {
        painter->setFont(*boldFont);
        currentHeaderMinWidth = painter->boundingRect(QRectF(0, 0, 1, 1), currentHeaderRows.first().cells.first().contents, opt).width();
        painter->setFont(*stdFont);
    }

    // Now all column widths
    QList<int> columnDataLengths = providedData[ExportManager::DATA_LENGTHS].value<QList<int>>();
    int columnCount = columnNames.size();
    if (columnDataLengths.size() < columnCount)
    {
        qWarning() << "PdfExport: column widths provided by ExportWorker (" << columnDataLengths.size()
                   << ") is less than number of columns to export (" << columnCount << ").";
    }

    // Fill up column data widths if there are any missing from the provided data (should not happen)
    while (columnDataLengths.size() < columnCount)
        columnDataLengths << maxColWidth;

    // Precalculate column widths for the header row
    QList<int> headerWidths;
    for (const QString& colName : columnNames)
        headerWidths << painter->boundingRect(QRectF(0, 0, 1, 1), colName, opt).width();

    // Calculate width for each column and compare it with its header width, then pick the wider, but never wider than the maximum width.
    calculatedColumnWidths.clear();
    int dataWidth = 0;
    int headerWidth = 0;
    int totalWidth = 0;
    for (int i = 0, total = columnDataLengths.size(); i < total; ++i)
    {
        dataWidth = painter->boundingRect(QRectF(0, 0, 1, 1), tplChar.repeated(columnDataLengths[i]), opt).width();
        headerWidth = headerWidths[i];

        // Pick the wider one, but never wider than maxColWidth
        totalWidth = qMax(dataWidth, headerWidth) + padding * 2; // wider one + padding on sides
        calculatedColumnWidths << qMin(maxColWidth, totalWidth);
    }

    // Calculate how many columns will fit for every page, until the full row is rendered.
    columnsPerPage.clear();
    int colsForThePage = 0;
    int currentTotalWidth = 0;
    for (int i = 0, total = calculatedColumnWidths.size(); i < total; ++i)
    {
        colsForThePage++;
        currentTotalWidth += calculatedColumnWidths[i];
        if (currentTotalWidth >= pageWidth)
        {
            colsForThePage--;
            columnsPerPage << colsForThePage;

            // Make sure that columns on previous page are at least as wide as the header
            currentTotalWidth -= calculatedColumnWidths[i];
            if (currentTotalWidth < currentHeaderMinWidth && i > 0)
                calculatedColumnWidths[i-1] += (currentHeaderMinWidth - currentTotalWidth);

            // Reset values fot next interation
            currentTotalWidth = calculatedColumnWidths[i];
            colsForThePage = 1;
        }
    }

    if (colsForThePage > 0)
    {
        columnsPerPage << colsForThePage;
        if (currentTotalWidth < currentHeaderMinWidth && !calculatedColumnWidths.isEmpty())
            calculatedColumnWidths.last() += (currentHeaderMinWidth - currentTotalWidth);
    }
}

void PdfExport::calculateRowHeights()
{
    // Calculating heights for data rows
    int thisRowMaxHeight = 0;
    int colHeight = 0;
    for (Row& row : bufferedRows)
    {
        if (row.height > 0) // was calculated in the previous rendering iteration
            continue;

        thisRowMaxHeight = 0;
        for (int col = 0, total = row.cells.size(); col < total; ++col)
        {
            // We pass rect, that is as wide as calculated column width and we look how height extends
            colHeight = calculateRowHeight(calculatedColumnWidths[col], row.cells[col].contents);
            thisRowMaxHeight = qMax(thisRowMaxHeight, colHeight);
        }
        row.height = qMin(maxRowHeight, thisRowMaxHeight);
    }

    // Calculating heights for header rows
    totalHeaderRowsHeight = 0;
    for (Row& row : currentHeaderRows)
    {
        thisRowMaxHeight = 0;

        switch (row.type)
        {
            case Row::HEADER:
                painter->setFont(*boldFont);
                // Main header can be as wide as page, so that's the rect we pass
                colHeight = calculateRowHeight(pageWidth, row.cells.first().contents);
                thisRowMaxHeight = qMax(thisRowMaxHeight, colHeight);
                break;
            case Row::COLUMNS_HEADER:
                painter->setFont(*stdFont);
                for (int col = 0, total = row.cells.size(); col < total; ++col)
                {
                    // This is the same as for data rows (see above)
                    colHeight = calculateRowHeight(calculatedColumnWidths[col], row.cells[col].contents);
                    thisRowMaxHeight = qMax(thisRowMaxHeight, colHeight);
                }
                break;
            case Row::NORMAL: // this should not take place, normal rows are not stored in header rows
                break; // no-op
        }

        row.height = qMin(maxRowHeight, thisRowMaxHeight);
        totalHeaderRowsHeight += row.height;
    }
    painter->setFont(*stdFont);
}

int PdfExport::calculateRowHeight(int textWidth, const QString& contents)
{
    // Measures height expanding due to constrained text width, line wrapping and top+bottom padding
    return painter->boundingRect(QRectF(0, 0, (textWidth - padding * 2), 1), contents, *textOption).height() + padding * 2;
}
