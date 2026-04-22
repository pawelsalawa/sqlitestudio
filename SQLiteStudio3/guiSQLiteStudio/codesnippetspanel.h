#ifndef CODESNIPPETSPANEL_H
#define CODESNIPPETSPANEL_H

#include <QDockWidget>


class CodeSnippetEditorModel;
class QSortFilterProxyModel;
class SqliteSyntaxHighlighter;
namespace Ui {
    class CodeSnippetsPanel;
}

class CodeSnippetsPanel : public QDockWidget
{
        Q_OBJECT

    public:
        explicit CodeSnippetsPanel(QWidget *parent = nullptr);
        ~CodeSnippetsPanel();
        QVariant saveSession();
        void restoreSession(const QVariant& value);

    private:
        void init();

        Ui::CodeSnippetsPanel *ui;
        CodeSnippetEditorModel* model = nullptr;
        QSortFilterProxyModel* snippetFilterModel = nullptr;

    private slots:
        void cfgCodeSnippetListChanged();
        void snippetSelected(const QModelIndex &current, const QModelIndex &previous);
};

#endif // CODESNIPPETSPANEL_H
