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
//    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    diff = new diff_match_patch;
}

void SqlCompareView::setSides(const QList<QPair<QString, QString>>& data)
{
    setRowCount(data.size());

    int row = 0;
    SqlView* leftView = nullptr;
    SqlView* rightView = nullptr;
    for (const QPair<QString, QString>& rowData : data)
    {
        leftView = new SqlView();
        leftView->setFrameStyle(QFrame::NoFrame);
        leftView->setPlainText(rowData.first);
        setCellWidget(row, 0, leftView);

        rightView = new SqlView();
        rightView->setFrameStyle(QFrame::NoFrame);
        rightView->setPlainText(rowData.second);
        setCellWidget(row, 1, rightView);

        setupHighlighting(rowData.first, rowData.second, leftView, rightView);

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

    SqlView* leftView = nullptr;
    SqlView* rightView = nullptr;
    QSize leftSize;
    QSize rightSize;
    for (int row = 0, total = rowCount(); row < total; ++row)
    {
        leftView = dynamic_cast<SqlView*>(cellWidget(row, 0));
        leftView->document()->setTextWidth(leftWidth);

        rightView = dynamic_cast<SqlView*>(cellWidget(row, 1));
        rightView->document()->setTextWidth(rightWidth);

        leftSize = QSize(leftWidth, leftView->document()->size().toSize().height());
        rightSize = QSize(rightWidth, rightView->document()->size().toSize().height());
        if (leftSize.height() > rightSize.height())
            rightSize.setHeight(leftSize.height());
        else
            leftSize.setHeight(rightSize.height());

        leftView->setFixedSize(leftSize);
        rightView->setFixedSize(rightSize);
    }
    verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void SqlCompareView::updateLabels()
{
    setHorizontalHeaderLabels({leftLabel, rightLabel});
}

void SqlCompareView::setupHighlighting(const QString& left, const QString& right, SqlView* leftView, SqlView* rightView)
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
                leftView->setTextBackgroundColor(leftPos, leftPos + lgt - 1, Qt::red);
                leftPos += lgt;
                break;
            case EQUAL:
                leftPos += lgt;
                rightPos += lgt;
                break;
            case INSERT:
                rightView->setTextBackgroundColor(leftPos, leftPos + lgt - 1, Qt::green);
                rightPos += lgt;
                break;
        }
    }
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
