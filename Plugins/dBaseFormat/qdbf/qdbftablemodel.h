#ifndef QDBFTABLEMODEL_H
#define QDBFTABLEMODEL_H

#include "qdbf_global.h"
#include "qdbftable.h"

#include <QtCore/QAbstractTableModel>

namespace QDbf {
namespace Internal {

class QDBF_EXPORT QDbfTableModelPrivate;

} // namespace Internal

class QDbfTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit QDbfTableModel(QObject *parent = 0);
    QDbfTableModel(const QString &filePath, QObject *parent = 0);
    ~QDbfTableModel();

    bool open(const QString &filePath, bool readOnly = false);
    bool open(bool readOnly = false);

    void close();

    bool readOnly() const;

    QDbfTable::DbfTableError error() const;

    int rowCount(const QModelIndex &index = QModelIndex()) const;
    int columnCount(const QModelIndex &index = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant data(const QModelIndex &index, int role) const;

    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::DisplayRole);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool canFetchMore(const QModelIndex &index = QModelIndex()) const;
    void fetchMore(const QModelIndex &index = QModelIndex());

private:
    Internal::QDbfTableModelPrivate *const d;

    friend class Internal::QDbfTableModelPrivate;
};

} // namespace QDbf

#endif // QDBFTABLEMODEL_H
