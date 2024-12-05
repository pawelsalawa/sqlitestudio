#include "pdfexport.h"
#include "common/unused.h"
#include "uiutils.h"
#include <QtMath>
#include <QPainter>
#include <QFont>
#include <QDebug>

QString PdfExport::bulletChar = "\u2022";

CFG_DEFINE(PdfExportConfig)
#define PDFEXPORT_CFG CFG_INSTANCE(PdfExportConfig)

bool PdfExport::init()
{
    SQLS_INIT_RESOURCE(pdfexport);
    textOption = new QTextOption();
    textOption->setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    return GenericExportPlugin::init();
}

void PdfExport::deinit()
{
    safe_delete(textOption);
    CFG_DELETE_INSTANCE(PdfExportConfig);
    SQLS_CLEANUP_RESOURCE(pdfexport);
}

QPagedPaintDevice* PdfExport::createPaintDevice(const QString& documentTitle, bool &takeOwnership)
{
    QPdfWriter* pdfWriter = new QPdfWriter(output);
    pdfWriter->setTitle(documentTitle);
    pdfWriter->setCreator(tr("SQLiteStudio v%1").arg(SQLITESTUDIO->getVersionString()));
    takeOwnership = true;
    return pdfWriter;
}

QString PdfExport::getFormatName() const
{
    return "PDF";
}

ExportManager::StandardConfigFlags PdfExport::standardOptionsToEnable() const
{
    return ExportManager::StandardConfigFlags();
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
    UNUSED(query);

    if (!beginDoc(tr("SQL query results")))
        return false;

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

bool PdfExport::exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(columnNames);
    UNUSED(database);
    UNUSED(ddl);

    if (isTableExport() && !beginDoc(tr("Exported table: %1").arg(table)))
        return false;

    exportObjectHeader(tr("Table: %1").arg(table));

    QStringList tableDdlColumns = {tr("Column"), tr("Data type"), tr("Constraints")};
    exportObjectColumnsHeader(tableDdlColumns);

    QString colDef;
    QString colType;
    QStringList columnsAndTypes;
    int colNamesLength = 0;
    int dataTypeLength = 0;
    for (SqliteCreateTable::Column*& col : createTable->columns)
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

    for (SqliteCreateTable::Column*& col : createTable->columns)
        exportTableColumnRow(col);

    if (createTable->constraints.size() > 0)
    {
        QStringList tableDdlColumns = {tr("Global table constraints")};
        exportObjectColumnsHeader(tableDdlColumns);
        exportTableConstraintsRow(createTable->constraints);
    }

    flushObjectPages();

    prepareTableDataExport(table, columnsAndTypes, providedData);
    return true;
}

bool PdfExport::exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable, const QHash<ExportManager::ExportProviderFlag, QVariant> providedData)
{
    UNUSED(columnNames);
    UNUSED(database);
    UNUSED(ddl);
    UNUSED(createTable);

    if (isTableExport() && !beginDoc(tr("Exported table: %1").arg(table)))
        return false;

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

    for (int& val : columnDataLengths)
    {
        if (val > cellDataLimit)
            val = cellDataLimit;
    }

    return columnDataLengths;
}

bool PdfExport::exportTableRow(SqlResultsRowPtr data)
{
    exportDataRow(data->valueList());
    return true;
}

bool PdfExport::afterExport()
{
    endDoc();
    return true;
}

bool PdfExport::afterExportTable()
{
    flushDataPages(true);
    return true;
}

bool PdfExport::afterExportQueryResults()
{
    flushDataPages(true);
    return true;
}

bool PdfExport::beforeExportDatabase(const QString& database)
{
    return beginDoc(tr("Exported database: %1").arg(database));
}

bool PdfExport::exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex)
{
    UNUSED(database);
    UNUSED(ddl);

    exportObjectHeader(tr("Index: %1").arg(name));

    QStringList indexColumns = {tr("Property", "index header"), tr("Value", "index header")};
    exportObjectColumnsHeader(indexColumns);

    exportObjectRow({tr("Indexed table"), name});
    exportObjectRow({tr("Unique index"), (createIndex->uniqueKw ? tr("Yes") : tr("No"))});

    indexColumns = QStringList({tr("Column"), tr("Collation"), tr("Sort order")});
    exportObjectColumnsHeader(indexColumns);

    QString sort;
    for (SqliteOrderBy*& idxCol : createIndex->indexedColumns)
    {
        if (idxCol->order != SqliteSortOrder::null)
            sort = sqliteSortOrder(idxCol->order);
        else
            sort = "";

        exportObjectRow({idxCol->getColumnString(), idxCol->getCollation(), sort});
    }

    if (createIndex->where)
    {
        indexColumns = QStringList({tr("Partial index condition")});
        exportObjectColumnsHeader(indexColumns);
        exportObjectRow(createIndex->where->detokenize());
    }

    flushObjectPages();
    return true;
}

bool PdfExport::exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger)
{
    UNUSED(database);
    UNUSED(ddl);

    exportObjectHeader(tr("Trigger: %1").arg(name));

    QStringList trigColumns = {tr("Property", "trigger header"), tr("Value", "trigger header")};
    exportObjectColumnsHeader(trigColumns);
    exportObjectRow({tr("Activation time"), SqliteCreateTrigger::time(createTrigger->eventTime)});

    QString event = createTrigger->event ? SqliteCreateTrigger::Event::typeToString(createTrigger->event->type) : "";
    exportObjectRow({tr("For action"), event});

    QString onObj;
    if (createTrigger->eventTime == SqliteCreateTrigger::Time::INSTEAD_OF)
        onObj = tr("On view");
    else
        onObj = tr("On table");

    exportObjectRow({onObj, createTrigger->table});

    QString cond = createTrigger->precondition ? createTrigger->precondition->detokenize() : "";
    exportObjectRow({tr("Activation condition"), cond});

    QStringList queryStrings;
    for (SqliteQuery*& q : createTrigger->queries)
        queryStrings << q->detokenize();

    exportObjectColumnsHeader({tr("Code executed")});
    exportObjectRow(queryStrings.join("\n"));

    flushObjectPages();
    return true;
}

bool PdfExport::exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr view)
{
    UNUSED(database);
    UNUSED(ddl);

    exportObjectHeader(tr("View: %1").arg(name));
    exportObjectColumnsHeader({tr("Query:")});
    exportObjectRow(view->select->detokenize());

    flushObjectPages();
    return true;
}

bool PdfExport::isBinaryData() const
{
    return true;
}

bool PdfExport::beginDoc(const QString& title)
{
    safe_delete(painter);

    if (takeDeviceOwnership)
        safe_delete(pagedWriter);

    pagedWriter = createPaintDevice(title, takeDeviceOwnership);
    if (!pagedWriter)
        return false;

    painter = new QPainter(pagedWriter);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(Qt::black, lineWidth));

    setupConfig();
    return true;
}

void PdfExport::endDoc()
{
    drawFooter();
}

void PdfExport::cleanupAfterExport()
{
    safe_delete(painter);
    if (takeDeviceOwnership)
        safe_delete(pagedWriter);
}

void PdfExport::setupConfig()
{
    pagedWriter->setPageSize(convertPageSize(PDFEXPORT_CFG.PdfExport.PageSize.get()));
    pageWidth = pagedWriter->width();
    pageHeight = pagedWriter->height();
    pointsPerMm = pageWidth / pagedWriter->pageLayout().pageSize().size(QPageSize::Millimeter).width();

    stdFont = PDFEXPORT_CFG.PdfExport.Font.get();
    stdFont.setPointSize(PDFEXPORT_CFG.PdfExport.FontSize.get());
    boldFont = stdFont;
    boldFont.setBold(true);
    italicFont = stdFont;
    italicFont.setItalic(true);
    painter->setFont(stdFont);

    topMargin = mmToPoints(PDFEXPORT_CFG.PdfExport.TopMargin.get());
    rightMargin = mmToPoints(PDFEXPORT_CFG.PdfExport.RightMargin.get());
    leftMargin = mmToPoints(PDFEXPORT_CFG.PdfExport.LeftMargin.get());
    bottomMargin = mmToPoints(PDFEXPORT_CFG.PdfExport.BottomMargin.get());
    updateMargins();

    maxColWidth = pageWidth / 5;
    padding = mmToPoints(PDFEXPORT_CFG.PdfExport.Padding.get());

    QRectF rect = painter->boundingRect(QRectF(padding, padding, pageWidth - 2 * padding, 1), "X", *textOption);
    minRowHeight = rect.height() + padding * 2;
    maxRowHeight = qMax((int)(pageHeight * 0.225), minRowHeight);
    rowsToPrebuffer = (int)qCeil((double)pageHeight / minRowHeight);

    cellDataLimit = PDFEXPORT_CFG.PdfExport.MaxCellBytes.get();
    printRowNum = PDFEXPORT_CFG.PdfExport.PrintRowNum.get();
    printPageNumbers = PDFEXPORT_CFG.PdfExport.PrintPageNumbers.get();

    lastRowY = getContentsTop();
    currentPage = -1;
    rowNum = 1;
}

void PdfExport::updateMargins()
{
    pageWidth -= (leftMargin + rightMargin);
    pageHeight -= (topMargin + bottomMargin);
    painter->setClipRect(QRect(leftMargin, topMargin, pageWidth, pageHeight));

    if (printPageNumbers)
    {
        int pageNumHeight = getPageNumberHeight();
        bottomMargin += pageNumHeight;
        pageHeight -= pageNumHeight;
    }

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
        switch (value.userType())
        {
            case QMetaType::Int:
            case QMetaType::UInt:
            case QMetaType::LongLong:
            case QMetaType::ULongLong:
            case QMetaType::Double:
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
    row.recalculateColumnWidths = true;
    bufferedObjectRows << row;
}

void PdfExport::exportObjectColumnsHeader(const QStringList& columns)
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
        for (SqliteCreateTable::Column::Constraint*& constr : column->constraints)
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

void PdfExport::exportObjectRow(const QStringList& values)
{
    ObjectRow row;
    row.type = ObjectRow::Type::MULTI;

    ObjectCell cell;
    for (const QString& value : values)
    {
        cell.contents << value;
        row.cells << cell;
        cell.contents.clear();
    }

    bufferedObjectRows << row;
}

void PdfExport::exportObjectRow(const QString& value)
{
    ObjectRow row;
    row.type = ObjectRow::Type::SINGLE;

    ObjectCell cell;
    cell.contents << value;
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

    int y = getContentsTop();
    int totalHeight = lastRowY - y;

    if (totalHeight > 0)
    {
        totalHeight += minRowHeight * 2; // a space between objects on one page
        y += totalHeight;
    }
    else
        newPage();

    while (!bufferedObjectRows.isEmpty())
    {
        ObjectRow& row = bufferedObjectRows.first();

        if (row.recalculateColumnWidths || row.cells.size() != calculatedObjectColumnWidths.size())
            calculateObjectColumnWidths();

        totalHeight += row.height;
        if (totalHeight > pageHeight)
        {
            newPage();
            y = getContentsTop();
            totalHeight = row.height;
        }
        flushObjectRow(row, y);

        y += row.height;

        bufferedObjectRows.removeFirst();
    }

    lastRowY = getContentsTop() + totalHeight;
}

void PdfExport::drawObjectTopLine(int y)
{
    painter->drawLine(getContentsLeft(), y, getContentsRight(), y);
}

void PdfExport::drawObjectCellHeaderBackground(int x1, int y1, int x2, int y2)
{
    painter->save();
    painter->setBrush(QBrush(PDFEXPORT_CFG.PdfExport.HeaderBgColor.get(), Qt::SolidPattern));
    painter->setPen(Qt::NoPen);
    painter->drawRect(x1, y1, x2 - x1, y2 - y1);
    painter->restore();
}

void PdfExport::drawFooter()
{
    QString footer = tr("Document generated with SQLiteStudio v%1").arg(SQLITESTUDIO->getVersionString());

    QTextOption opt = *textOption;
    opt.setAlignment(Qt::AlignRight);

    int y = lastRowY + minRowHeight;
    int height = pageHeight - y;
    int txtHeight = painter->boundingRect(QRect(0, 0, pageWidth, height), footer, opt).height();
    if ((y + txtHeight) > pageHeight)
    {
        newPage();
        y = getContentsTop();
    }

    painter->save();
    painter->setFont(italicFont);
    painter->drawText(QRect(getContentsLeft(), y, pageWidth, txtHeight), footer, opt);
    painter->restore();
}

void PdfExport::flushObjectRow(const PdfExport::ObjectRow& row, int y)
{
    painter->save();
    int x = getContentsLeft();
    int bottom = y + row.height;
    int top = y;
    int left = getContentsLeft();
    int right = getContentsRight();
    switch (row.type)
    {
        case ObjectRow::Type::SINGLE:
        {
            const ObjectCell& cell = row.cells.first();
            if (cell.headerBackground)
                drawObjectCellHeaderBackground(left, y, right, bottom);

            painter->drawLine(left, y, left, bottom);
            painter->drawLine(right, y, right, bottom);
            painter->drawLine(left, top, right, top);
            painter->drawLine(left, bottom, right, bottom);

            flushObjectCell(cell, left, y, pageWidth, row.height);
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

            x = left;
            painter->drawLine(x, y, x, bottom);
            for (int& w : calculatedObjectColumnWidths)
            {
                x += w;
                painter->drawLine(x, y, x, bottom);
            }
            painter->drawLine(left, top, right, top);
            painter->drawLine(left, bottom, right, bottom);

            x = left;
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
        painter->setFont(boldFont);
    else if (cell.italic)
        painter->setFont(italicFont);

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
    for (int& w : calculatedObjectColumnWidths)
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

    int left = getContentsLeft();
    int right = getContentsRight();
    int top = getContentsTop();

    // Calculating width of all columns on this page
    int totalColumnsWidth = sum(calculatedDataColumnWidths.mid(columnStart, columnEndBefore - columnStart));
    int totalColumnsWidthWithRowId = totalColumnsWidth + rowNumColumnWidth;

    // Calculating height of all rows
    int totalRowsHeight = 0;
    for (const DataRow& row : allRows)
        totalRowsHeight += row.height;

    // Draw header background
    int x = getDataColumnsStartX();
    painter->save();
    painter->setBrush(QBrush(PDFEXPORT_CFG.PdfExport.HeaderBgColor.get(), Qt::SolidPattern));
    painter->setPen(Qt::NoPen);
    painter->drawRect(QRect(x, top, totalColumnsWidth, totalHeaderRowsHeight));
    painter->restore();

    // Draw rowNum background
    if (printRowNum)
    {
        painter->save();
        painter->setBrush(QBrush(PDFEXPORT_CFG.PdfExport.HeaderBgColor.get(), Qt::SolidPattern));
        painter->setPen(Qt::NoPen);
        painter->drawRect(QRect(left, top, rowNumColumnWidth, totalRowsHeight));
        painter->restore();
    }

    // Draw horizontal lines
    int y = top;
    int horizontalLineEnd = x + totalColumnsWidth;
    painter->drawLine(left, y, horizontalLineEnd, y);
    for (const DataRow& row : allRows)
    {
        y += row.height;
        painter->drawLine(left, y, horizontalLineEnd, y);
    }

    // Draw dashed horizontal lines if there are more columns on the next page and there is space on the right side
    if (columnEndBefore < calculatedDataColumnWidths.size() && horizontalLineEnd < right)
    {
        y = top;
        painter->save();
        QPen pen(Qt::lightGray, lineWidth, Qt::DashLine);
        pen.setDashPattern(QVector<qreal>({5.0, 3.0}));
        painter->setPen(pen);
        painter->drawLine(horizontalLineEnd, y, right, y);
        for (const DataRow& row : allRows)
        {
            y += row.height;
            painter->drawLine(horizontalLineEnd, y, right, y);
        }
        painter->restore();
    }

    // Finding first row to start vertical lines from. It's either a COLUMNS_HEADER, or first data row, after headers.
    int verticalLinesStart = top;
    if (headerRow)
        verticalLinesStart += headerRow->height;

    // Draw vertical lines
    x = getDataColumnsStartX();
    painter->drawLine(left, top, left, top + totalRowsHeight);
    if (printRowNum)
        painter->drawLine(x, verticalLinesStart, x, top + totalRowsHeight);

    for (int col = columnStart; col < columnEndBefore; col++)
    {
        x += calculatedDataColumnWidths[col];
        painter->drawLine(x, (col+1 == columnEndBefore) ? top : verticalLinesStart, x, top + totalRowsHeight);
    }

    // Draw header rows
    y = top;
    if (headerRow)
        flushDataHeaderRow(*headerRow, y, totalColumnsWidthWithRowId, columnStart, columnEndBefore);

    if (columnsHeaderRow)
        flushDataHeaderRow(*columnsHeaderRow, y, totalColumnsWidthWithRowId, columnStart, columnEndBefore);

    // Draw data
    int localRowNum = rowNum;
    for (int rowCounter = 0; rowCounter < rowsToRender && !bufferedDataRows.isEmpty(); rowCounter++)
        flushDataRow(bufferedDataRows[rowCounter], y, columnStart, columnEndBefore, localRowNum++);

    lastRowY = y;
}

void PdfExport::flushDataRow(const DataRow& row, int& y, int columnStart, int columnEndBefore, int localRowNum)
{
    int textWidth = 0;
    int textHeight = 0;
    int colWidth = 0;
    int x = getContentsLeft();

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
        painter->setPen(PDFEXPORT_CFG.PdfExport.NullValueColor.get());
        painter->setFont(italicFont);
    }

    painter->drawText(rect, cell.contents.left(cellDataLimit), opt);
    painter->restore();
}

void PdfExport::flushDataCell(const QRect& rect, const QString& contents, QTextOption* opt)
{
    painter->drawText(rect, contents.left(cellDataLimit), *opt);
}

void PdfExport::flushDataHeaderRow(const PdfExport::DataRow& row, int& y, int totalColsWidth, int columnStart, int columnEndBefore)
{
    QTextOption opt = *textOption;
    opt.setAlignment(Qt::AlignHCenter);
    int x = getContentsLeft();
    y += padding;
    switch (row.type)
    {
        case DataRow::Type::TOP_HEADER:
        {
            x += padding;
            painter->save();
            painter->setFont(boldFont);
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
    if (!printPageNumbers)
        return;

    QString page = QString::number(currentPage + 1);

    QTextOption opt = *textOption;
    opt.setWrapMode(QTextOption::NoWrap);

    painter->save();
    painter->setFont(italicFont);
    QRect rect = painter->boundingRect(QRect(0, 0, 1, 1), page, opt).toRect();
    int x = getContentsRight() - rect.width();
    int y = getContentsBottom(); // the bottom margin was already increased to hold page numbers
    QRect newRect(x, y, rect.width(), rect.height());
    painter->drawText(newRect, page, *textOption);
    painter->restore();
}

int PdfExport::getPageNumberHeight()
{
    QTextOption opt = *textOption;
    opt.setWrapMode(QTextOption::NoWrap);

    painter->save();
    painter->setFont(italicFont);
    int height = painter->boundingRect(QRect(0, 0, 1, 1), "0123456789", opt).height();
    painter->restore();
    return height;
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
    lastRowY = getContentsTop();
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
            painter->setFont(boldFont);
            currentHeaderMinWidth = painter->boundingRect(QRectF(0, 0, 1, 1), headerRow->cells.first().contents, opt).width();
            currentHeaderMinWidth += padding * 2;
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
            if ((currentTotalWidth + rowNumColumnWidth) < currentHeaderMinWidth && i > 0)
            {
                expandColumnIndex = 1;
                if (columnToExpand > -1)
                    expandColumnIndex = colsForThePage - columnToExpand;

                calculatedDataColumnWidths[i - expandColumnIndex] += (currentHeaderMinWidth - (currentTotalWidth + rowNumColumnWidth));
            }

            // Reset values fot next interation
            currentTotalWidth = calculatedDataColumnWidths[i];
            colsForThePage = 1;
        }
    }

    if (colsForThePage > 0)
    {
        columnsPerPage << colsForThePage;
        if ((currentTotalWidth + rowNumColumnWidth) < currentHeaderMinWidth && !calculatedDataColumnWidths.isEmpty())
        {
            int i = calculatedDataColumnWidths.size();
            expandColumnIndex = 1;
            if (columnToExpand > -1)
                expandColumnIndex = colsForThePage - columnToExpand;

            calculatedDataColumnWidths[i - expandColumnIndex] += (currentHeaderMinWidth - (currentTotalWidth + rowNumColumnWidth));
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
        painter->setFont(boldFont);
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
        return getContentsLeft() + rowNumColumnWidth;

    return getContentsLeft();
}

int PdfExport::getContentsLeft() const
{
    return leftMargin;
}

int PdfExport::getContentsTop() const
{
    return topMargin;
}

int PdfExport::getContentsRight() const
{
    return getContentsLeft() + pageWidth;
}

int PdfExport::getContentsBottom() const
{
    return topMargin + pageHeight;
}

qreal PdfExport::mmToPoints(qreal sizeMM)
{
    return pointsPerMm * sizeMM;
}

CfgMain* PdfExport::getConfig()
{
    return &PDFEXPORT_CFG;
}

QString PdfExport::getExportConfigFormName() const
{
    return "PdfExportConfig";
}

QFont Cfg::getPdfExportDefaultFont()
{
    QPainter p;
    return p.font();
}

QStringList Cfg::getPdfPageSizes()
{
    return getAllPageSizes();
}
