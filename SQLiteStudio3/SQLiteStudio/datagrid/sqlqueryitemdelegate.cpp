#include "sqlqueryitemdelegate.h"
#include "sqlquerymodel.h"
#include "sqlqueryitem.h"
#include "unused.h"
#include "notifymanager.h"
#include "uiconfig.h"
#include <QPainter>
#include <QEvent>
#include <QLineEdit>

SqlQueryItemDelegate::SqlQueryItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void SqlQueryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    SqlQueryItem* item = getItem(index);

    if (item->isUncommited())
    {
        painter->setPen(item->isCommitingError() ? CFG_UI.Colors.DataUncommitedError.get() : CFG_UI.Colors.DataUncommited.get());
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
    }
}

QWidget* SqlQueryItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    UNUSED(option);
    if (!index.isValid())
        return nullptr;

    const SqlQueryModel* model = dynamic_cast<const SqlQueryModel*>(index.model());
    SqlQueryItem* item = model->itemFromIndex(index);

    if (item->isDeletedRow())
    {
        notifyWarn(tr("Cannot edit this cell. Details: %2").arg(tr("The row is marked for deletion.")));
        return nullptr;
    }

    if (!item->getColumn()->canEdit())
    {
        notifyWarn(tr("Cannot edit this cell. Details: %2").arg(item->getColumn()->getEditionForbiddenReason()));
        return nullptr;
    }

    if (item->isLimitedValue())
        item->loadFullData();

    return getEditor(item->getValue().userType(), parent);
}

SqlQueryItem* SqlQueryItemDelegate::getItem(const QModelIndex &index) const
{
    const SqlQueryModel* queryModel = dynamic_cast<const SqlQueryModel*>(index.model());
    return queryModel->itemFromIndex(index);
}

QWidget* SqlQueryItemDelegate::getEditor(int type, QWidget* parent) const
{
    UNUSED(type);
    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(editor->style()->styleHint(QStyle::SH_ItemView_DrawDelegateFrame, 0, editor));
    return editor;

}
