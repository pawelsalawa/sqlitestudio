#ifndef COMPLETERMODEL_H
#define COMPLETERMODEL_H

#include "expectedtoken.h"
#include <QAbstractItemModel>
#include <QModelIndex>

class CompleterView;
class Icon;

class CompleterModel : public QAbstractItemModel
{
    public:
        enum UserRole
        {
            VALUE = 1000,
            CONTEXT = 1001,
            PREFIX = 1002,
            LABEL = 1003,
            TYPE = 1004
        };

        explicit CompleterModel(QObject *parent = 0);

        QModelIndex index(int row, int column, const QModelIndex& parent) const;
        QModelIndex parent(const QModelIndex& child) const;
        int rowCount(const QModelIndex& parent) const;
        int columnCount(const QModelIndex& parent) const;
        QVariant data(const QModelIndex& index, int role) const;

        void setCompleterView(CompleterView* view);
        void setData(const QList<ExpectedTokenPtr>& data);
        void setFilter(const QString& filter);
        QString getFilter() const;
        void clear();
        ExpectedTokenPtr getToken(int index) const;

    private:
        QIcon getIcon(ExpectedToken::Type type) const;
        void applyFilter();

        QList<ExpectedTokenPtr> tokens;
        QString filter;
        CompleterView* completerView = nullptr;
};

#endif // COMPLETERMODEL_H
