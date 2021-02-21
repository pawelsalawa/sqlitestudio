#ifndef SQLQUERYITEMDELEGATE_H
#define SQLQUERYITEMDELEGATE_H

#include "guiSQLiteStudio_global.h"
#include "db/sqlquery.h"
#include <QStyledItemDelegate>
#include <QSet>

class SqlQueryItem;
class QComboBox;
class QStandardItemModel;
class SqlQueryModel;
class SqlQueryView;

class GUI_API_EXPORT SqlQueryItemDelegate : public QStyledItemDelegate
{
        Q_OBJECT
    public:
        explicit SqlQueryItemDelegate(QObject *parent = 0);

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
        bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index);
        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QString	displayText(const QVariant & value, const QLocale & locale) const;
        void setEditorData(QWidget * editor, const QModelIndex & index) const;
        void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;
        void mouseLeftIndex(const QModelIndex& index);

    private:
        class FkComboFilter : public QObject
        {
            public:
                explicit FkComboFilter(const SqlQueryItemDelegate* delegate, SqlQueryView* comboView, QObject* parent = 0);
                bool eventFilter(QObject *obj, QEvent *event);

            private:
                const SqlQueryItemDelegate* delegate = nullptr;
                SqlQueryView* comboView = nullptr;
        };

        class FkComboShowFilter : public QObject
        {
            public:
                explicit FkComboShowFilter(const SqlQueryItemDelegate* delegate, SqlQueryView* comboView, QObject* parent = 0);
                bool eventFilter(QObject *obj, QEvent *event);

            private:
                const SqlQueryItemDelegate* delegate = nullptr;
                SqlQueryView* comboView = nullptr;
        };

        static QRect getLoadFullValueButtonRegion(const QRect& cell);
        static bool isOverFullValueButton(const QRect& cell, QMouseEvent* event);
        static bool isOverFullValueButton(const QRect& cell, int x, int y);
        static bool shouldLoadFullData(const QRect& rect, QMouseEvent* event, const QModelIndex& index);
        static bool shouldLoadFullData(const QRect& rect, int x, int y, const QModelIndex& index);
        static bool isLimited(const QModelIndex &index);

        SqlQueryItem* getItem(const QModelIndex &index) const;
        QWidget* getEditor(int type, QWidget* parent) const;
        QWidget* getFkEditor(SqlQueryItem* item, QWidget* parent, const SqlQueryModel *model) const;
        void setEditorDataForLineEdit(QLineEdit* le, const QModelIndex& index) const;
        void setEditorDataForFk(QComboBox* cb, const QModelIndex& index) const;
        void setModelDataForFk(QComboBox* editor, QAbstractItemModel* model, const QModelIndex& index) const;
        void setModelDataForLineEdit(QLineEdit* editor, QAbstractItemModel* model, const QModelIndex& index) const;
        QString getSqlForFkEditor(SqlQueryItem* item) const;
        qlonglong getRowCountForFkEditor(Db* db, const QString& query, bool *isError) const;
        int getFkViewHeaderWidth(SqlQueryView* fkView, bool includeScrollBar) const;
        void updateComboViewGeometry(SqlQueryView* comboView, bool initial) const;

        QStyleOptionButton fullValueButtonOption;
        QSet<QWidget*> editorsWithAsyncExecution;
        QModelIndex mouseOverFullDataButton;
        bool showingFullButtonTooltip = false;
        bool lmbPressedOnButton = false;
        mutable int fkViewParentItemSize = 0;
        mutable QHash<SqlQueryModel*, QComboBox*> modelToFkCombo;
        mutable QHash<SqlQueryModel*, QVariant> modelToFkInitialValue;

        static bool warnedAboutHugeContents;
        static const int LOAD_FULL_VALUE_BUTTON_SIZE = 18;
        static const int LOAD_FULL_VALUE_BUTTON_SIDE_MARGIN = 2;
        static const int LOAD_FULL_VALUE_ICON_SIZE = 12;
        static const qlonglong MAX_ROWS_FOR_FK = 10000L;
        static const int FK_CELL_LENGTH_LIMIT = 30;
        static const int HUGE_CONTENTS_WARNING_LIMIT = 32767; // pow(2, 16) / 2 - 1

    private slots:
        void fkDataReady();
        void fkDataFailed(const QString& errorText);
};

#endif // SQLQUERYITEMDELEGATE_H
