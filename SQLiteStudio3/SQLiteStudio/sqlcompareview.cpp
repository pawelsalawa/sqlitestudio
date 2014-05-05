#include "sqlcompareview.h"
#include "sqlview.h"
#include "common/utils.h"
#include "diff/diff_match_patch.h"
#include "sqlitesyntaxhighlighter.h"
#include <QHeaderView>
#include <QDebug>

SqlCompareView::SqlCompareView(QWidget *parent) :
    QTableWidget(parent)
{
    setColumnCount(2);
    setVerticalScrollMode(ScrollPerPixel);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    horizontalHeader()->setVisible(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    diff = new diff_match_patch;
}

void SqlCompareView::setSides(const QList<QPair<QString, QString>>& data)
{
    setRowCount(data.size());

    SqliteSyntaxHighlighter* highlighterLeft;
    SqliteSyntaxHighlighter* highlighterRight;

    int row = 0;
    SqlView* view;
    for (const QPair<QString, QString>& rowData : data)
    {
        view = new SqlView();
        view->setFrameStyle(QFrame::NoFrame);
        view->setPlainText(rowData.first);
        highlighterLeft = view->getHighlighter();
        setCellWidget(row, 0, view);

        view = new SqlView();
        view->setFrameStyle(QFrame::NoFrame);
        view->setPlainText(rowData.second);
        highlighterRight = view->getHighlighter();
        setCellWidget(row, 1, view);

        setupHighlighting(rowData.first, rowData.second, highlighterLeft, highlighterRight);

        row++;
    }
    updateLabels();
    updateSizes();
}

void SqlCompareView::setLeftLabel(const QString& label)
{
    leftLabel = label;
}

void SqlCompareView::setRightLabel(const QString& label)
{
    rightLabel = label;
}

void SqlCompareView::updateSizes()
{
    if (rowCount() == 0 || !isVisible())
        return;

    SqlView* view = dynamic_cast<SqlView*>(cellWidget(0, 0));
    if (!view)
    {
        qCritical() << "Not a SqlView in SqlCompareView::updateSizes():" << cellWidget(0, 0);
        return;
    }

    QFont font = view->font();
    QFontMetrics fm(font);

    int leftWidth = horizontalHeader()->sectionSize(0);
    int rightWidth = horizontalHeader()->sectionSize(1);

    QSize size;
    for (int row = 0, total = rowCount(); row < total; ++row)
    {
        view = dynamic_cast<SqlView*>(cellWidget(row, 0));
        view->document()->setTextWidth(leftWidth - 20);
        size = QSize(leftWidth, view->document()->size().toSize().height());
        view->setFixedSize(size);

        view = dynamic_cast<SqlView*>(cellWidget(row, 1));
        view->document()->setTextWidth(rightWidth - 20);
        size = QSize(rightWidth, view->document()->size().toSize().height());
        view->setFixedSize(size);
    }
    verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void SqlCompareView::updateLabels()
{
    setHorizontalHeaderLabels({leftLabel, rightLabel});
}

void SqlCompareView::setupHighlighting(const QString& left, const QString& right, SqliteSyntaxHighlighter* highlighterLeft, SqliteSyntaxHighlighter* highlighterRight)
{
    QList<Diff> diffs = diff->diff_main(left, right);
    int leftPos = 0;
    int rightPos = 0;
    int lgt = 0;
    for (const Diff& d : diffs)
    {
        lgt = d.text.length();
        switch (d.operation)
        {
            case DELETE:
                highlighterLeft->addCustomBgColor(leftPos, leftPos + lgt - 1, Qt::red);
                leftPos += lgt;
                break;
            case EQUAL:
                leftPos += lgt;
                rightPos += lgt;
                break;
            case INSERT:
                highlighterRight->addCustomBgColor(leftPos, leftPos + lgt - 1, Qt::green);
                rightPos += lgt;
                break;
        }
    }
    highlighterLeft->rehighlight();
    highlighterRight->rehighlight();
}

void SqlCompareView::resizeEvent(QResizeEvent* e)
{
    QTableWidget::resizeEvent(e);
    updateSizes();
}

void SqlCompareView::showEvent(QShowEvent* e)
{
    QTableWidget::showEvent(e);
    updateSizes();
}
