#ifndef SQLCOMPAREVIEW_H
#define SQLCOMPAREVIEW_H

#include <QTableWidget>

class SqlView;
class diff_match_patch;
class SqliteSyntaxHighlighter;

class SqlCompareView : public QTableWidget
{
    public:
        explicit SqlCompareView(QWidget* parent = 0);

        void setSides(const QList<QPair<QString,QString>>& data);
        void setLeftLabel(const QString& label);
        void setRightLabel(const QString& label);
        void updateSizes();

    protected:
        void resizeEvent(QResizeEvent* e);
        void showEvent(QShowEvent* e);

    private:
        void updateLabels();
        void setupHighlighting(const QString& left, const QString& right, SqlView* leftView, SqlView* rightView);

        QString leftLabel;
        QString rightLabel;
        diff_match_patch* diff = nullptr;
};

#endif // SQLCOMPAREVIEW_H
