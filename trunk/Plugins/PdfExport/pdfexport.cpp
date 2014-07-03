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
    return ExportManager::DATA_LENGTHS|ExportManager::ROW_COUNT;
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

    totalRows = providedData[ExportManager::ROW_COUNT].toInt();

    QStringList columnNames;
    for (const QueryExecutor::ResultColumnPtr& col : columns)
        columnNames << col->displayName;

    clearHeaderRows();
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

    totalRows = providedData[ExportManager::ROW_COUNT].toInt();

    // Prepare for exporting data row
    clearHeaderRows();
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
    painter->setPen(QPen(Qt::black, lineWidth));
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
    pointsPerMm = pageWidth / pdfWriter->pageSizeMM().width();

    topMargin = mmToPoints(20);
    rightMargin = mmToPoints(20);
    leftMargin = mmToPoints(20);
    bottomMargin = mmToPoints(20);
    updateMargins();

    maxColWidth = pageWidth / 5;
    padding = 50;

    stdFont = new QFont(painter->font());
    stdFont->setPointSize(10);
    boldFont = new QFont(*stdFont);
    boldFont->setBold(true);
    italicFont = new QFont(*stdFont);
    italicFont->setItalic(true);
    painter->setFont(*stdFont);

    QRectF rect = painter->boundingRect(QRectF(padding, padding, pageWidth - 2 * padding, 1), "X", *textOption);
    minRowHeight = rect.height() + padding * 2;
    maxRowHeight = qMax((int)(pageHeight * 0.225), minRowHeight);
    rowsToPrebuffer = (int)ceil((double)pageHeight / minRowHeight);

    currentPage = -1;
    rowNum = 1;
}

void PdfExport::updateMargins()
{
    pageWidth -= (leftMargin + rightMargin);
    pageHeight -= (topMargin + bottomMargin);
    painter->setClipRect(QRect(leftMargin, topMargin, pageWidth, pageHeight));

    // In order to render full width of the line, we need to add more margin, a half of the line width
    leftMargin += lineWidth / 2;
    rightMargin += lineWidth / 2;
    topMargin += lineWidth / 2;
    bottomMargin += lineWidth / 2;
    pageWidth -= lineWidth;
    pageHeight -= lineWidth;
}

void PdfExport::clearHeaderRows()
{
    headerRow.reset();
    columnsHeaderRow.reset();
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

        rowNum += rowsToRender;
    }
}

void PdfExport::renderDataRowsPage(int columnStart, int columnEndBefore, int rowsToRender)
{
    QList<Row> allRows;
    if (headerRow)
        allRows += *headerRow;

    if (columnsHeaderRow)
        allRows += *columnsHeaderRow;

    allRows += bufferedRows.mid(0, rowsToRender);

    // Calculating width of all columns on this page
    int totalColsWidth = sum(calculatedColumnWidths.mid(columnStart, columnEndBefore - columnStart));

    // Calculating height of all rows
    int totalRowsHeight = 0;
    for (const Row& row : allRows)
        totalRowsHeight += row.height;

    // Draw header background
    int x = getDataColumnsStartX();
    painter->save();
    painter->setBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
    painter->setPen(Qt::NoPen);
    painter->drawRect(QRect(x, topMargin, totalColsWidth, totalHeaderRowsHeight));
    painter->restore();

    // Draw rowNum background
    if (printRowNum)
    {
        painter->save();
        painter->setBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
        painter->setPen(Qt::NoPen);
        painter->drawRect(QRect(leftMargin, topMargin, rowNumColumnWidth, totalRowsHeight));
        painter->restore();
    }

    // Draw horizontal lines
    int y = topMargin;
    int horizontalLineEnd = x + totalColsWidth;
    painter->drawLine(leftMargin, y, horizontalLineEnd, y);
    for (const Row& row : allRows)
    {
        y += row.height;
        painter->drawLine(leftMargin, y, horizontalLineEnd, y);
    }

    // Draw dashed horizontal lines if there are more columns on the next page and there is space on the right side
    if (columnEndBefore < calculatedColumnWidths.size() && horizontalLineEnd < (leftMargin + pageWidth))
    {
        y = topMargin;
        painter->save();
        QPen pen(Qt::lightGray, 15, Qt::DashLine);
        pen.setDashPattern(QVector<qreal>({5.0, 3.0}));
        painter->setPen(pen);
        painter->drawLine(horizontalLineEnd, y, leftMargin + pageWidth, y);
        for (const Row& row : allRows)
        {
            y += row.height;
            painter->drawLine(horizontalLineEnd, y, leftMargin + pageWidth, y);
        }
        painter->restore();
    }

    // Finding first row to start vertical lines from. It's either a COLUMNS_HEADER, or first data row, after headers.
    int verticalLinesStart = topMargin;
    if (headerRow)
        verticalLinesStart += headerRow->height;

    // Draw vertical lines
    x = getDataColumnsStartX();
    painter->drawLine(leftMargin, topMargin, leftMargin, topMargin + totalRowsHeight);
    if (rowNum)
        painter->drawLine(x, topMargin, x, topMargin + totalRowsHeight);

    for (int col = columnStart; col < columnEndBefore; col++)
    {
        x += calculatedColumnWidths[col];
        painter->drawLine(x, (col+1 == columnEndBefore) ? topMargin : verticalLinesStart, x, topMargin + totalRowsHeight);
    }

    // Draw header rows
    y = topMargin;
    if (headerRow)
        renderHeaderRow(*headerRow, y, totalColsWidth, columnStart, columnEndBefore);

    if (columnsHeaderRow)
        renderHeaderRow(*columnsHeaderRow, y, totalColsWidth, columnStart, columnEndBefore);

    // Draw data
    int localRowNum = rowNum;
    for (int rowCounter = 0; rowCounter < rowsToRender && !bufferedRows.isEmpty(); rowCounter++)
        renderDataRow(bufferedRows[rowCounter], y, columnStart, columnEndBefore, localRowNum++);
}

void PdfExport::renderDataRow(const Row& row, int& y, int columnStart, int columnEndBefore, int localRowNum)
{
    int textWidth = 0;
    int textHeight = 0;
    int colWidth = 0;
    int x = leftMargin;

    y += padding;
    if (printRowNum)
    {
        QTextOption opt = *textOption;
        opt.setAlignment(Qt::AlignRight);

        x += padding;
        textWidth = rowNumColumnWidth - padding * 2;
        textHeight = row.height - padding * 2;
        renderDataCell(QRect(x, y, textWidth, textHeight), QString::number(localRowNum), &opt);
        x += rowNumColumnWidth - padding;
    }

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

    painter->save();
    if (cell.isNull)
    {
        painter->setPen(Qt::gray);
        painter->setFont(*italicFont);
    }

    painter->drawText(rect, cell.contents, opt);
    painter->restore();
}

void PdfExport::renderDataCell(const QRect& rect, const QString& contents, QTextOption* opt)
{
    painter->drawText(rect, contents, *opt);
}

void PdfExport::renderHeaderRow(const PdfExport::Row& row, int& y, int totalColsWidth, int columnStart, int columnEndBefore)
{
    QTextOption opt = *textOption;
    opt.setAlignment(Qt::AlignHCenter);
    int x = leftMargin;
    y += padding;
    switch (row.type)
    {
        case Row::HEADER:
        {
            x += padding;
            painter->save();
            painter->setFont(*boldFont);
            painter->drawText(QRect(x, y, totalColsWidth - 2 * padding, row.height - 2 * padding), row.cells.first().contents, opt);
            painter->restore();
            break;
        }
        case Row::COLUMNS_HEADER:
        {
            if (printRowNum)
            {
                x += padding;
                int textWidth = rowNumColumnWidth - padding * 2;
                int textHeight = row.height - padding * 2;
                painter->drawText(QRect(x, y, textWidth, textHeight), "#", opt);
                x += rowNumColumnWidth - padding;
            }

            for (int col = columnStart; col < columnEndBefore; col++)
                renderHeaderCell(x, y, row, col, &opt);

            break;
        }
        case Row::NORMAL:
            break; // no-op
    }
    y += row.height - padding;
}

void PdfExport::renderHeaderCell(int& x, int y, const PdfExport::Row& row, int col, QTextOption* opt)
{
    x += padding;
    painter->drawText(QRect(x, y, calculatedColumnWidths[col] - 2 * padding, row.height - 2 * padding), row.cells[col].contents, *opt);
    x += calculatedColumnWidths[col] - padding;
}

void PdfExport::renderPageNumber()
{
    QString page = QString::number(currentPage + 1);

    QTextOption opt = *textOption;
    opt.setWrapMode(QTextOption::NoWrap);

    painter->save();
    painter->setFont(*italicFont);
    QRect rect = painter->boundingRect(QRectF(0, 0, 1, 1), page, opt).toRect();
    int x = leftMargin + pageWidth - rect.width();
    int y = topMargin + pageHeight- rect.height();
    QRect newRect(x, y, rect.width(), rect.height());
    painter->drawText(newRect, page, *textOption);
    painter->restore();
}

void PdfExport::exportObjectHeader(const QString& contents)
{
    Row* row = new Row;
    row->type = Row::HEADER;

    Cell cell;
    cell.contents = contents;
    cell.alignment = Qt::AlignHCenter;
    row->cells << cell;

    headerRow.reset(row);
}

void PdfExport::exportColumnsHeader(const QStringList& columns)
{
    Row* row = new Row;
    row->type = Row::COLUMNS_HEADER;

    Cell cell;
    cell.alignment = Qt::AlignHCenter;
    for (const QString& col : columns)
    {
        cell.contents = col;
        row->cells << cell;
    }

    columnsHeaderRow.reset(row);
}

void PdfExport::newPage()
{
    if (currentPage < 0)
    {
        currentPage = 0;
        renderPageNumber();
        return;
    }

    pdfWriter->newPage();
    currentPage++;
    renderPageNumber();
}

void PdfExport::calculateColumnWidths(const QStringList& columnNames, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData, int columnToExpand)
{
    static const QString tplChar = "W";

    // Text options for calculating widths will not allow word wrapping
    QTextOption opt = *textOption;
    opt.setWrapMode(QTextOption::NoWrap);

    // Calculate header width first
    if (columnToExpand > -1)
    {
        // If any column was picked for expanding table to page width, the header will also be full page width.
        // This will also result later with expanding selected column to the header width = page width.
        currentHeaderMinWidth = pageWidth;
    }
    else
    {
        currentHeaderMinWidth = 0;
        if (headerRow)
        {
            painter->save();
            painter->setFont(*boldFont);
            currentHeaderMinWidth = painter->boundingRect(QRectF(0, 0, 1, 1), headerRow->cells.first().contents, opt).width();
            painter->restore();
        }
    }

    // Calculate width of rowNum column (if enabled)
    if (printRowNum)
        rowNumColumnWidth = painter->boundingRect(QRectF(0, 0, 1, 1), QString::number(totalRows), opt).width() + 2 * padding;

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
    int expandColumnIndex = 0;
    int dataColumnsWidth = getDataColumnsWidth();
    for (int i = 0, total = calculatedColumnWidths.size(); i < total; ++i)
    {
        colsForThePage++;
        currentTotalWidth += calculatedColumnWidths[i];
        if (currentTotalWidth > dataColumnsWidth)
        {
            colsForThePage--;
            columnsPerPage << colsForThePage;

            // Make sure that columns on previous page are at least as wide as the header
            currentTotalWidth -= calculatedColumnWidths[i];
            if (currentTotalWidth < currentHeaderMinWidth && i > 0)
            {
                expandColumnIndex = 1;
                if (columnToExpand > -1)
                    expandColumnIndex = colsForThePage - columnToExpand;

                calculatedColumnWidths[i - expandColumnIndex] += (currentHeaderMinWidth - currentTotalWidth);
            }

            // Reset values fot next interation
            currentTotalWidth = calculatedColumnWidths[i];
            colsForThePage = 1;
        }
    }

    if (colsForThePage > 0)
    {
        columnsPerPage << colsForThePage;
        if (currentTotalWidth < currentHeaderMinWidth && !calculatedColumnWidths.isEmpty())
        {
            int i = calculatedColumnWidths.size();
            expandColumnIndex = 1;
            if (columnToExpand > -1)
                expandColumnIndex = colsForThePage - columnToExpand;

            calculatedColumnWidths[i - expandColumnIndex] += (currentHeaderMinWidth - currentTotalWidth);
        }
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
    painter->save();
    totalHeaderRowsHeight = 0;
    thisRowMaxHeight = 0;
    if (headerRow)
    {
        painter->setFont(*boldFont);
        // Main header can be as wide as page, so that's the rect we pass
        colHeight = calculateRowHeight(pageWidth, headerRow->cells.first().contents);
        thisRowMaxHeight = qMax(thisRowMaxHeight, colHeight);

        columnsHeaderRow->height = qMin(maxRowHeight, thisRowMaxHeight);
        totalHeaderRowsHeight += columnsHeaderRow->height;
    }

    if (columnsHeaderRow)
    {
        for (int col = 0, total = columnsHeaderRow->cells.size(); col < total; ++col)
        {
            // This is the same as for data rows (see above)
            colHeight = calculateRowHeight(calculatedColumnWidths[col], columnsHeaderRow->cells[col].contents);
            thisRowMaxHeight = qMax(thisRowMaxHeight, colHeight);
        }

        columnsHeaderRow->height = qMin(maxRowHeight, thisRowMaxHeight);
        totalHeaderRowsHeight += columnsHeaderRow->height;
    }

    painter->restore();
}

int PdfExport::calculateRowHeight(int textWidth, const QString& contents)
{
    // Measures height expanding due to constrained text width, line wrapping and top+bottom padding
    return painter->boundingRect(QRectF(0, 0, (textWidth - padding * 2), 1), contents, *textOption).height() + padding * 2;
}

int PdfExport::getDataColumnsWidth() const
{
    if (printRowNum)
        return pageWidth - rowNumColumnWidth;

    return pageWidth;
}

int PdfExport::getDataColumnsStartX() const
{
    if (printRowNum)
        return leftMargin + rowNumColumnWidth;

    return leftMargin;
}

qreal PdfExport::mmToPoints(qreal sizeMM)
{
    return pointsPerMm * sizeMM;
}
