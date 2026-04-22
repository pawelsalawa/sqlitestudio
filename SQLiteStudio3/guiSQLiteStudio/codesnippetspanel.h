#ifndef CODESNIPPETSPANEL_H
#define CODESNIPPETSPANEL_H

#include <QDockWidget>


class CodeSnippetEditorModel;
class QSortFilterProxyModel;
class SqliteSyntaxHighlighter;
class QLineEdit;
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

    protected:
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;
        void dropEvent(QDropEvent *event) override;

    private:
        void init();

        Ui::CodeSnippetsPanel *ui;
        QLineEdit* nameFilter = nullptr;
        CodeSnippetEditorModel* dataModel = nullptr;
        QSortFilterProxyModel* viewModel = nullptr;

    private slots:
        void cfgCodeSnippetListChanged();
        void snippetSelected(const QModelIndex &current, const QModelIndex &previous);
        void applyFilter(const QString& value);
        void editSnippet(const QModelIndex &idx);
};

#endif // CODESNIPPETSPANEL_H
