#include "pdfexport.h"
#include "common/unused.h"
#include <QTextDocument>
#include <QPainter>
#include <QPdfWriter>
#include <QFont>
#include <QDebug>

QString PdfExport::bulletChar = "\u2022";

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

QPagedPaintDevice* PdfExport::createPaintDevice(const QString& documentTitle)
{
    QPdfWriter* pdfWriter = new QPdfWriter(output);
    pdfWriter->setTitle(documentTitle);
    pdfWriter->setCreator(tr("SQLiteStudio v%1").arg(SQLITESTUDIO->getVersionString()));
    return pdfWriter;
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

    clearDataHeaders();
    exportDataColumnsHeader(columnNames);

    QList<int> columnDataLengths = getColumnDataLengths(columnNames.size(), providedData);
    calculateDataColumnWidths(columnNames, columnDataLengths);
    return true;
}

bool PdfExport::exportQueryResultsRow(SqlResultsRowPtr row)
{
    exportDataRow(row->valueList());
    return true;
}

bool PdfExport::afterExportQueryResults()
{
    flushDataPages(true);
    endDoc();
    return true;
}

bool PdfExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(columnNames);
    UNUSED(database);
    UNUSED(ddl);

    if (isTableExport())
        beginDoc(tr("Exported table: %1").arg(table));

    exportObjectHeader(tr("Table: %1").arg(table));

    QStringList tableDdlColumns = {tr("Column"), tr("Data type"), tr("Constraints")};
    exportTableColumnsHeader(tableDdlColumns);

    QString colDef;
    QString colType;
    QStringList columnsAndTypes;
    int colNamesLength = 0;
    int dataTypeLength = 0;
    for (SqliteCreateTable::Column* col : createTable->columns)
    {
        colDef = col->name;
        colNamesLength = qMax(colNamesLength, colDef.size());
        colType = "";
        if (col->type)
        {
            colType = col->type->toDataType().toFullTypeString();
            colDef += "\n" + colType;
            dataTypeLength = qMax(dataTypeLength, colType.size());
        }

        columnsAndTypes << colDef;
    }

    QList<int> columnDataLengths = {colNamesLength, dataTypeLength, 0};
    calculateDataColumnWidths(tableDdlColumns, columnDataLengths, 2);

    for (SqliteCreateTable::Column* col : createTable->columns)
        exportTableColumnRow(col);

    if (createTable->constraints.size() > 0)
    {
        QStringList tableDdlColumns = {tr("Global table constraints")};
        exportTableColumnsHeader(tableDdlColumns);
        exportTableConstraintsRow(createTable->constraints);
    }

    flushObjectPages();

    prepareTableDataExport(table, columnsAndTypes, providedData);
    return true;
}

bool PdfExport::exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    if (isTableExport())
        beginDoc(tr("Exported table: %1").arg(table));

    prepareTableDataExport(table, columnNames, providedData);

    return true;
}

void PdfExport::prepareTableDataExport(const QString& table, const QStringList& columnNames, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    resetDataTable();
    totalRows = providedData[ExportManager::ROW_COUNT].toInt();

    // Prepare for exporting data row
    clearDataHeaders();
    if (!isTableExport()) // for database export we need to mark what is this object name
        exportDataHeader(tr("Table: %1").arg(table));

    exportDataColumnsHeader(columnNames);

    QList<int> columnDataLengths = getColumnDataLengths(columnNames.size(), providedData);
    calculateDataColumnWidths(columnNames, columnDataLengths);
}

QList<int> PdfExport::getColumnDataLengths(int columnCount, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    QList<int> columnDataLengths = providedData[ExportManager::DATA_LENGTHS].value<QList<int>>();
    if (columnDataLengths.size() < columnCount)
    {
        qWarning() << "PdfExport: column widths provided by ExportWorker (" << columnDataLengths.size()
                   << ") is less than number of columns to export (" << columnCount << ").";
    }

    // Fill up column data widths if there are any missing from the provided data (should not happen)
    while (columnDataLengths.size() < columnCount)
        columnDataLengths << maxColWidth;

    return columnDataLengths;
}

bool PdfExport::exportTableRow(SqlResultsRowPtr data)
{
    exportDataRow(data->valueList());
    return true;
}

bool PdfExport::afterExportTable()
{
    flushDataPages(true);
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
    exportObjectHeader(tr("Index: %1").arg(name));

    QStringList indexColumns = {tr("Indexed table"), tr("Unique index")};
    exportIndexColumnsHeader(indexColumns);
    exportIndexTableAndUniqueness(createIndex->table, createIndex->uniqueKw);

    indexColumns = {tr("Column"), tr("Collation"), tr("Sort order")};
    exportIndexColumnsHeader(indexColumns);

    for (SqliteIndexedColumn* idxCol : createIndex->indexedColumns)
        exportIndexColumnRow(idxCol);

    if (createIndex->where)
    {
        indexColumns = {tr("Partial index condition")};
        exportIndexColumnsHeader(indexColumns);
        exportIndexPartialCondition(createIndex->where);
    }

    flushObjectPages();
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
    safe_delete(pagedWriter);
    safe_delete(stdFont);
    safe_delete(boldFont);
    pagedWriter = createPaintDevice(title);
    painter = new QPainter(pagedWriter);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(Qt::black, lineWidth));

    setupConfig();
}

void PdfExport::endDoc()
{
    safe_delete(painter);
    safe_delete(pagedWriter);
}

void PdfExport::setupConfig()
{
    pagedWriter->setPageSize(QPdfWriter::A4);
    pageWidth = pagedWriter->width();
    pageHeight = pagedWriter->height();
    pointsPerMm = pageWidth / pagedWriter->pageSizeMM().width();

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

void PdfExport::clearDataHeaders()
{
    headerRow.reset();
    columnsHeaderRow.reset();
}

void PdfExport::resetDataTable()
{
    clearDataHeaders();
    bufferedDataRows.clear();
    rowNum = 0;
}

void PdfExport::exportDataRow(const QList<QVariant>& data)
{
    DataCell cell;
    DataRow row;

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

    bufferedDataRows << row;
    checkForDataRender();
}

void PdfExport::exportObjectHeader(const QString& contents)
{
    ObjectRow row;
    ObjectCell cell;
    cell.headerBackground = true;
    cell.contents << contents;
    cell.bold = true;
    cell.alignment = Qt::AlignCenter;
    row.cells << cell;

    row.type = ObjectRow::Type::SINGLE;
    bufferedObjectRows << row;
}

void PdfExport::exportTableColumnsHeader(const QStringList& columns)
{
    ObjectRow row;
    ObjectCell cell;

    for (const QString& col : columns)
    {
        cell.headerBackground = true;
        cell.contents.clear();
        cell.contents << col;
        cell.alignment = Qt::AlignCenter;
        row.cells << cell;
    }

    row.recalculateColumnWidths = true;
    row.type = ObjectRow::Type::MULTI;
    bufferedObjectRows << row;
}

void PdfExport::exportTableColumnRow(SqliteCreateTable::Column* column)
{
    ObjectRow row;
    row.type = ObjectRow::Type::MULTI;

    ObjectCell cell;
    cell.contents << column->name;
    row.cells << cell;
    cell.contents.clear();

    if (column->type)
        cell.contents << column->type->toDataType().toFullTypeString();
    else
        cell.contents << "";

    row.cells << cell;
    cell.contents.clear();

    if (column->constraints.size() > 0)
    {
        for (SqliteCreateTable::Column::Constraint* constr : column->constraints)
            cell.contents << constr->detokenize();

        cell.type = ObjectCell::Type::LIST;
    }
    else
    {
        cell.contents << "";
    }
    row.cells << cell;
    cell.contents.clear();

    bufferedObjectRows << row;
}

void PdfExport::exportTableConstraintsRow(const QList<SqliteCreateTable::Constraint*>& constrList)
{
    ObjectRow row;
    row.type = ObjectRow::Type::SINGLE;

    ObjectCell cell;
    cell.type = ObjectCell::Type::LIST;

    if (constrList.size() > 0)
    {
        for (SqliteCreateTable::Constraint* constr : constrList)
            cell.contents << constr->detokenize();
    }
    else
    {
        cell.contents << "";
    }
    row.cells << cell;

    bufferedObjectRows << row;
}

void PdfExport::exportIndexTableAndUniqueness(const QString& table, bool unique)
{
    ObjectRow row;
    row.type = ObjectRow::Type::MULTI;

    ObjectCell cell;
    cell.contents << table;
    row.cells << cell;
    cell.contents.clear();

    cell.contents << (unique ? tr("Yes") : tr("No"));
    row.cells << cell;
    cell.contents.clear();

    bufferedObjectRows << row;
}

void PdfExport::exportIndexColumnsHeader(const QStringList& columns)
{
    exportTableColumnsHeader(columns); // currently those methods do same things
}

void PdfExport::exportIndexColumnRow(SqliteIndexedColumn* idxCol)
{
    ObjectRow row;
    row.type = ObjectRow::Type::MULTI;

    ObjectCell cell;
    cell.contents << idxCol->name;
    row.cells << cell;
    cell.contents.clear();

    cell.contents << idxCol->collate;
    row.cells << cell;
    cell.contents.clear();

    if (idxCol->sortOrder != SqliteSortOrder::null)
        cell.contents << sqliteSortOrder(idxCol->sortOrder);
    else
        cell.contents << "";

    row.cells << cell;
    cell.contents.clear();

    bufferedObjectRows << row;
}

void PdfExport::exportIndexPartialCondition(SqliteExpr* where)
{
    ObjectRow row;
    row.type = ObjectRow::Type::SINGLE;

    ObjectCell cell;
    cell.contents << where->detokenize();
    row.cells << cell;

    bufferedObjectRows << row;
}

int PdfExport::calculateRowHeight(int maxTextWidth, const QStringList& listContents)
{
    int textWidth = maxTextWidth - calculateBulletPrefixWidth();
    int totalHeight = 0;
    for (const QString& contents : listContents)
        totalHeight += calculateRowHeight(textWidth, contents);

    return totalHeight;
}

int PdfExport::calculateBulletPrefixWidth()
{
    static QString prefix = bulletChar + " ";

    QTextOption opt = *textOption;
    opt.setWrapMode(QTextOption::NoWrap);

    return painter->boundingRect(QRect(0, 0, 1, 1), prefix, *textOption).width();
}

void PdfExport::checkForDataRender()
{
    if (bufferedDataRows.size() >= rowsToPrebuffer)
        flushDataPages();
}

void PdfExport::flushObjectPages()
{
    if (bufferedObjectRows.isEmpty())
        return;

    newPage();
    drawObjectTopLine();

    int totalHeight = 0;
    int y = topMargin;
    while (!bufferedObjectRows.isEmpty())
    {
        ObjectRow& row = bufferedObjectRows.first();

        totalHeight += row.height;
        if (row.height > pageHeight)
        {
            newPage();
            drawObjectTopLine();
            totalHeight = row.height;
            y = topMargin;
        }
        flushObjectRow(row, y);
        y += row.height;

        bufferedObjectRows.removeFirst();
    }
}

void PdfExport::drawObjectTopLine()
{
    painter->drawLine(leftMargin, topMargin, leftMargin + pageWidth, topMargin);
}

void PdfExport::drawObjectCellHeaderBackground(int x1, int y1, int x2, int y2)
{
    int halfLine = lineWidth / 2;
    painter->save();
    painter->setBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
    painter->setPen(Qt::NoPen);
    painter->drawRect(x1 + halfLine, y1 + halfLine, x2 - x1 - lineWidth, y2 - y1 - lineWidth);
    painter->restore();
}

void PdfExport::flushObjectRow(const PdfExport::ObjectRow& row, int y)
{
    if (row.recalculateColumnWidths || row.cells.size() != calculatedObjectColumnWidths.size())
        calculateObjectColumnWidths();

    painter->save();
    int x = leftMargin;
    int bottom = y + row.height;
    int right = leftMargin + pageWidth;
    switch (row.type)
    {
        case ObjectRow::Type::SINGLE:
        {
            const ObjectCell& cell = row.cells.first();
            if (cell.headerBackground)
                drawObjectCellHeaderBackground(leftMargin, y, right, bottom);

            painter->drawLine(leftMargin, y, leftMargin, bottom);
            painter->drawLine(right, y, right, bottom);
            painter->drawLine(leftMargin, bottom, right, bottom);

            flushObjectCell(cell, leftMargin, y, pageWidth, row.height);
            break;
        }
        case ObjectRow::Type::MULTI:
        {
            int width = 0;
            for (int col = 0, total = calculatedObjectColumnWidths.size(); col < total; ++col)
            {
                width = calculatedObjectColumnWidths[col];
                if (row.cells[col].headerBackground)
                    drawObjectCellHeaderBackground(x, y, x + width, bottom);

                x += width;
            }

            x = leftMargin;
            painter->drawLine(x, y, x, bottom);
            for (int w : calculatedObjectColumnWidths)
            {
                x += w;
                painter->drawLine(x, y, x, bottom);
            }
            painter->drawLine(leftMargin, bottom, right, bottom);

            x = leftMargin;
            for (int col = 0, total = calculatedObjectColumnWidths.size(); col < total; ++col)
            {
                const ObjectCell& cell = row.cells[col];
                width = calculatedObjectColumnWidths[col];
                flushObjectCell(cell, x, y, width, row.height);
                x += width;
            }
            break;
        }
    }
    painter->restore();
}

void PdfExport::flushObjectCell(const PdfExport::ObjectCell& cell, int x, int y, int w, int h)
{
    QTextOption opt = *textOption;
    opt.setAlignment(cell.alignment);

    if (cell.bold)
        painter->setFont(*boldFont);
    else if (cell.italic)
        painter->setFont(*italicFont);

    switch (cell.type)
    {
        case ObjectCell::Type::NORMAL:
        {
            painter->drawText(QRect(x + padding, y + padding, w - 2 * padding, h - 2 * padding), cell.contents.first(), opt);
            break;
        }
        case ObjectCell::Type::LIST:
        {
            static QString prefix = bulletChar + " ";
            int prefixWidth = calculateBulletPrefixWidth();
            x += padding;
            y += padding;
            w -= 2 * padding;
            h -= 2 * padding;
            int txtX = x + prefixWidth;
            int txtW = w - prefixWidth;

            QTextOption prefixOpt = opt;
            prefixOpt.setAlignment(opt.alignment() | Qt::AlignTop);

            int txtH = 0;
            for (const QString& contents : cell.contents)
            {
                txtH = calculateRowHeight(txtW, contents);
                painter->drawText(QRect(x, y, prefixWidth, txtH), prefix, prefixOpt);
                painter->drawText(QRect(txtX, y, txtW, txtH), contents, opt);
                y += txtH;
            }
            break;
        }
    }
}

void PdfExport::calculateObjectColumnWidths(int columnToExpand)
{
    calculatedObjectColumnWidths.clear();
    if (bufferedObjectRows.size() == 0)
        return;

    QTextOption opt = *textOption;
    opt.setWrapMode(QTextOption::NoWrap);

    int colCount = bufferedObjectRows.first().cells.size();
    for (int i = 0; i < colCount; i++)
        calculatedObjectColumnWidths << 0;

    int width = 0;
    for (const ObjectRow& row : bufferedObjectRows)
    {
        if (row.cells.size() != colCount)
            break;

        for (int col = 0; col < colCount; col++)
        {
            width = painter->boundingRect(QRectF(0, 0, 1, 1), row.cells[col].contents.join("\n"), opt).width();
            width += 2 * padding;
            calculatedObjectColumnWidths[col] = qMax(calculatedObjectColumnWidths[col], width);
        }
    }

    int totalWidth = correctMaxObjectColumnWidths(colCount, columnToExpand);
    if (totalWidth < pageWidth)
    {
        int col = (columnToExpand > -1) ? columnToExpand : (colCount - 1);
        calculatedObjectColumnWidths[col] += (pageWidth - totalWidth);
    }

    calculateObjectRowHeights();
}

int PdfExport::correctMaxObjectColumnWidths(int colCount, int columnToExpand)
{
    int totalWidth = 0;
    for (int w : calculatedObjectColumnWidths)
        totalWidth += w;

    int maxWidth = pageWidth / colCount;
    if (totalWidth <= pageWidth)
        return totalWidth;

    int tmpWidth = 0;
    for (int col = 0; col < colCount && totalWidth > pageWidth; col++)
    {
        if (calculatedObjectColumnWidths[col] <= maxWidth)
            continue;

        if (col == columnToExpand)
            continue; // will handle that column as last one (if needed)

        tmpWidth = calculatedObjectColumnWidths[col];
        if ((totalWidth - calculatedObjectColumnWidths[col] + maxWidth) <= pageWidth)
        {
            calculatedObjectColumnWidths[col] -= (pageWidth - totalWidth + calculatedObjectColumnWidths[col] - maxWidth);
            return pageWidth; // the 'if' condition guarantees that shrinking this column that much will give us pageWidth
        }
        else
            calculatedObjectColumnWidths[col] = maxWidth;

        totalWidth -= tmpWidth - calculatedObjectColumnWidths[col];
    }

    if (columnToExpand > -1 && totalWidth > pageWidth)
    {
        tmpWidth = calculatedObjectColumnWidths[columnToExpand];
        if ((totalWidth - calculatedObjectColumnWidths[columnToExpand] + maxWidth) <= pageWidth)
            calculatedObjectColumnWidths[columnToExpand] -= (pageWidth - totalWidth + calculatedObjectColumnWidths[columnToExpand] - maxWidth);
        else
            calculatedObjectColumnWidths[columnToExpand] = maxWidth;
    }

    return pageWidth;
}

void PdfExport::calculateObjectRowHeights()
{
    int maxHeight = 0;
    int colWidth = 0;
    int height = 0;
    int colCount = calculatedObjectColumnWidths.size();
    for (ObjectRow& row : bufferedObjectRows)
    {
        if (row.cells.size() != colCount)
            return; // stop at this row, further calculation will be done when columns are recalculated

        maxHeight = 0;
        for (int col = 0; col < colCount; ++col)
        {
            colWidth = calculatedObjectColumnWidths[col];
            const ObjectCell& cell = row.cells[col];

            switch (cell.type)
            {
                case ObjectCell::Type::NORMAL:
                    height = calculateRowHeight(colWidth, cell.contents.first());
                    break;
                case ObjectCell::Type::LIST:
                    height = calculateRowHeight(colWidth, cell.contents);
                    break;
            }
            maxHeight = qMax(maxHeight, height);
        }

        row.height = qMin(maxRowHeight, maxHeight);
    }
}

void PdfExport::flushDataPages(bool forceRender)
{
    calculateDataRowHeights();

    int rowsToRender = 0;
    int totalRowHeight = 0;
    int colStartAt = 0;
    while ((bufferedDataRows.size() >= rowsToPrebuffer) || (forceRender && bufferedDataRows.size() > 0))
    {
        // Calculate how many rows we can render on single page
        rowsToRender = 0;
        totalRowHeight = totalHeaderRowsHeight;
        for (const DataRow& row : bufferedDataRows)
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
            flushDataRowsPage(colStartAt, colStartAt + cols, rowsToRender);
            colStartAt += cols;
        }

        for (int i = 0; i < rowsToRender; i++)
            bufferedDataRows.removeFirst();

        rowNum += rowsToRender;
    }
}

void PdfExport::flushDataRowsPage(int columnStart, int columnEndBefore, int rowsToRender)
{
    QList<DataRow> allRows;
    if (headerRow)
        allRows += *headerRow;

    if (columnsHeaderRow)
        allRows += *columnsHeaderRow;

    allRows += bufferedDataRows.mid(0, rowsToRender);

    // Calculating width of all columns on this page
    int totalColumnsWidth = sum(calculatedDataColumnWidths.mid(columnStart, columnEndBefore - columnStart));

    // Calculating height of all rows
    int totalRowsHeight = 0;
    for (const DataRow& row : allRows)
        totalRowsHeight += row.height;

    // Draw header background
    int x = getDataColumnsStartX();
    painter->save();
    painter->setBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
    painter->setPen(Qt::NoPen);
    painter->drawRect(QRect(x, topMargin, totalColumnsWidth, totalHeaderRowsHeight));
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
    int horizontalLineEnd = x + totalColumnsWidth;
    painter->drawLine(leftMargin, y, horizontalLineEnd, y);
    for (const DataRow& row : allRows)
    {
        y += row.height;
        painter->drawLine(leftMargin, y, horizontalLineEnd, y);
    }

    // Draw dashed horizontal lines if there are more columns on the next page and there is space on the right side
    if (columnEndBefore < calculatedDataColumnWidths.size() && horizontalLineEnd < (leftMargin + pageWidth))
    {
        y = topMargin;
        painter->save();
        QPen pen(Qt::lightGray, 15, Qt::DashLine);
        pen.setDashPattern(QVector<qreal>({5.0, 3.0}));
        painter->setPen(pen);
        painter->drawLine(horizontalLineEnd, y, leftMargin + pageWidth, y);
        for (const DataRow& row : allRows)
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
    if (printRowNum)
        painter->drawLine(x, verticalLinesStart, x, topMargin + totalRowsHeight);

    for (int col = columnStart; col < columnEndBefore; col++)
    {
        x += calculatedDataColumnWidths[col];
        painter->drawLine(x, (col+1 == columnEndBefore) ? topMargin : verticalLinesStart, x, topMargin + totalRowsHeight);
    }

    // Draw header rows
    y = topMargin;
    if (headerRow)
        flushDataHeaderRow(*headerRow, y, totalColumnsWidth, columnStart, columnEndBefore);

    if (columnsHeaderRow)
        flushDataHeaderRow(*columnsHeaderRow, y, totalColumnsWidth, columnStart, columnEndBefore);

    // Draw data
    int localRowNum = rowNum;
    for (int rowCounter = 0; rowCounter < rowsToRender && !bufferedDataRows.isEmpty(); rowCounter++)
        flushDataRow(bufferedDataRows[rowCounter], y, columnStart, columnEndBefore, localRowNum++);
}

void PdfExport::flushDataRow(const DataRow& row, int& y, int columnStart, int columnEndBefore, int localRowNum)
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
        flushDataCell(QRect(x, y, textWidth, textHeight), QString::number(localRowNum), &opt);
        x += rowNumColumnWidth - padding;
    }

    for (int col = columnStart; col < columnEndBefore; col++)
    {
        const DataCell& cell = row.cells[col];
        colWidth = calculatedDataColumnWidths[col];

        x += padding;
        textWidth = colWidth - padding * 2;
        textHeight = row.height - padding * 2;
        flushDataCell(QRect(x, y, textWidth, textHeight), cell);
        x += colWidth - padding;
    }
    y += row.height - padding;
}

void PdfExport::flushDataCell(const QRect& rect, const PdfExport::DataCell& cell)
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

void PdfExport::flushDataCell(const QRect& rect, const QString& contents, QTextOption* opt)
{
    painter->drawText(rect, contents, *opt);
}

void PdfExport::flushDataHeaderRow(const PdfExport::DataRow& row, int& y, int totalColsWidth, int columnStart, int columnEndBefore)
{
    QTextOption opt = *textOption;
    opt.setAlignment(Qt::AlignHCenter);
    int x = leftMargin;
    y += padding;
    switch (row.type)
    {
        case DataRow::Type::TOP_HEADER:
        {
            x += padding;
            painter->save();
            painter->setFont(*boldFont);
            painter->drawText(QRect(x, y, totalColsWidth - 2 * padding, row.height - 2 * padding), row.cells.first().contents, opt);
            painter->restore();
            break;
        }
        case DataRow::Type::COLUMNS_HEADER:
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
                flushDataHeaderCell(x, y, row, col, &opt);

            break;
        }
        case DataRow::Type::NORMAL:
            break; // no-op
    }
    y += row.height - padding;
}

void PdfExport::flushDataHeaderCell(int& x, int y, const PdfExport::DataRow& row, int col, QTextOption* opt)
{
    x += padding;
    painter->drawText(QRect(x, y, calculatedDataColumnWidths[col] - 2 * padding, row.height - 2 * padding), row.cells[col].contents, *opt);
    x += calculatedDataColumnWidths[col] - padding;
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

void PdfExport::exportDataHeader(const QString& contents)
{
    DataRow* row = new DataRow;
    row->type = DataRow::Type::TOP_HEADER;

    DataCell cell;
    cell.contents = contents;
    cell.alignment = Qt::AlignHCenter;
    row->cells << cell;

    headerRow.reset(row);
}

void PdfExport::exportDataColumnsHeader(const QStringList& columns)
{
    DataRow* row = new DataRow;
    row->type = DataRow::Type::COLUMNS_HEADER;

    DataCell cell;
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

    pagedWriter->newPage();
    currentPage++;
    renderPageNumber();
}

void PdfExport::calculateDataColumnWidths(const QStringList& columnNames, const QList<int>& columnDataLengths, int columnToExpand)
{
    static const QString tplChar = QStringLiteral("W");

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
    rowNumColumnWidth = 0;
    if (printRowNum)
        rowNumColumnWidth = painter->boundingRect(QRectF(0, 0, 1, 1), QString::number(totalRows), opt).width() + 2 * padding;

    // Precalculate column widths for the header row
    QList<int> headerWidths;
    for (const QString& colName : columnNames)
        headerWidths << painter->boundingRect(QRectF(0, 0, 1, 1), colName, opt).width();

    // Calculate width for each column and compare it with its header width, then pick the wider, but never wider than the maximum width.
    calculatedDataColumnWidths.clear();
    int dataWidth = 0;
    int headerWidth = 0;
    int totalWidth = 0;
    for (int i = 0, total = columnDataLengths.size(); i < total; ++i)
    {
        dataWidth = painter->boundingRect(QRectF(0, 0, 1, 1), tplChar.repeated(columnDataLengths[i]), opt).width();
        headerWidth = headerWidths[i];

        // Pick the wider one, but never wider than maxColWidth
        totalWidth = qMax(dataWidth, headerWidth) + padding * 2; // wider one + padding on sides
        calculatedDataColumnWidths << qMin(maxColWidth, totalWidth);
    }

    // Calculate how many columns will fit for every page, until the full row is rendered.
    columnsPerPage.clear();
    int colsForThePage = 0;
    int currentTotalWidth = 0;
    int expandColumnIndex = 0;
    int dataColumnsWidth = getDataColumnsWidth();
    for (int i = 0, total = calculatedDataColumnWidths.size(); i < total; ++i)
    {
        colsForThePage++;
        currentTotalWidth += calculatedDataColumnWidths[i];
        if (currentTotalWidth > dataColumnsWidth)
        {
            colsForThePage--;
            columnsPerPage << colsForThePage;

            // Make sure that columns on previous page are at least as wide as the header
            currentTotalWidth -= calculatedDataColumnWidths[i];
            if (currentTotalWidth < currentHeaderMinWidth && i > 0)
            {
                expandColumnIndex = 1;
                if (columnToExpand > -1)
                    expandColumnIndex = colsForThePage - columnToExpand;

                calculatedDataColumnWidths[i - expandColumnIndex] += (currentHeaderMinWidth - currentTotalWidth);
            }

            // Reset values fot next interation
            currentTotalWidth = calculatedDataColumnWidths[i];
            colsForThePage = 1;
        }
    }

    if (colsForThePage > 0)
    {
        columnsPerPage << colsForThePage;
        if (currentTotalWidth < currentHeaderMinWidth && !calculatedDataColumnWidths.isEmpty())
        {
            int i = calculatedDataColumnWidths.size();
            expandColumnIndex = 1;
            if (columnToExpand > -1)
                expandColumnIndex = colsForThePage - columnToExpand;

            calculatedDataColumnWidths[i - expandColumnIndex] += (currentHeaderMinWidth - currentTotalWidth - rowNumColumnWidth);
        }
    }
}

void PdfExport::calculateDataRowHeights()
{
    // Calculating heights for data rows
    int thisRowMaxHeight = 0;
    int actualColHeight = 0;
    for (DataRow& row : bufferedDataRows)
    {
        if (row.height > 0) // was calculated in the previous rendering iteration
            continue;

        thisRowMaxHeight = 0;
        for (int col = 0, total = row.cells.size(); col < total; ++col)
        {
            // We pass rect, that is as wide as calculated column width and we look how height extends
            actualColHeight = calculateRowHeight(calculatedDataColumnWidths[col], row.cells[col].contents);
            thisRowMaxHeight = qMax(thisRowMaxHeight, actualColHeight);
        }
        row.height = qMin(maxRowHeight, thisRowMaxHeight);
    }

    // Calculating heights for header rows
    totalHeaderRowsHeight = 0;
    if (headerRow)
    {
        painter->save();
        painter->setFont(*boldFont);
        // Main header can be as wide as page, so that's the rect we pass
        actualColHeight = calculateRowHeight(pageWidth, headerRow->cells.first().contents);

        headerRow->height = qMin(maxRowHeight, actualColHeight);
        totalHeaderRowsHeight += headerRow->height;
        painter->restore();
    }

    if (columnsHeaderRow)
    {
        thisRowMaxHeight = 0;
        for (int col = 0, total = columnsHeaderRow->cells.size(); col < total; ++col)
        {
            // This is the same as for data rows (see above)
            actualColHeight = calculateRowHeight(calculatedDataColumnWidths[col], columnsHeaderRow->cells[col].contents);
            thisRowMaxHeight = qMax(thisRowMaxHeight, actualColHeight);
        }

        columnsHeaderRow->height = qMin(maxRowHeight, thisRowMaxHeight);
        totalHeaderRowsHeight += columnsHeaderRow->height;
    }

}

int PdfExport::calculateRowHeight(int maxTextWidth, const QString& contents)
{
    // Measures height expanding due to constrained text width, line wrapping and top+bottom padding
    return painter->boundingRect(QRect(0, 0, (maxTextWidth - padding * 2), 1), contents, *textOption).height() + padding * 2;
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
