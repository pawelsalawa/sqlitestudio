#include "guidbobjectorganizer.h"
#include "db/db.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

GuiDbObjectOrganizer::GuiDbObjectOrganizer()
{
    this->confirmFunction = [=](const QStringList& tables) -> bool {return confirmReferencedTables(tables);};
    this->nameConflictResolveFunction = [=](QString& nameInConflict) -> bool {return resolveNameConflict(nameInConflict);};
}

bool GuiDbObjectOrganizer::confirmReferencedTables(const QStringList& tables)
{
    QString msg;
    switch (mode)
    {
        case Mode::COPY_OBJECTS:
            msg = tr("Selected tables refer to other tables:\n%1\n\nWould you like to copy them too?");
            break;
        case Mode::MOVE_OBJECTS:
            msg = tr("Selected tables refer to other tables:\n%1\n\nWould you like to move them too?");
            break;
        case Mode::COPY_COLUMNS:
        case Mode::MOVE_COLUMNS:
            return false; // confirm should not be called for columns
        case Mode::unknown:
            qWarning() << "Unhandled unknown mode in DbObjectOrganizer.";
            return false;
    }

    int btn = QMessageBox::question(nullptr, tr("Referenced tables"), msg.arg(tables.join(", ")),
                                                            QMessageBox::Yes, QMessageBox::No);
    return btn == QMessageBox::Yes;
}

bool GuiDbObjectOrganizer::resolveNameConflict(QString& nameInConflict)
{
    QString msg;
    switch (mode)
    {
        case Mode::COPY_OBJECTS:
        case Mode::MOVE_OBJECTS:
            msg = tr("The target name '%1' already exists in database '%2'. Provide new, unique name:").arg(nameInConflict).arg(dstDb->getName());
            break;
        case Mode::COPY_COLUMNS:
        case Mode::MOVE_COLUMNS:
            msg = tr("The column '%1' already exists in table '%2'. Provide new, unique name:").arg(nameInConflict).arg(dstTable);
            break;
        case Mode::unknown:
            qWarning() << "Unhandled unknown mode in DbObjectOrganizer.";
            return false;
    }

    bool ok = false;
    QString newValue = QInputDialog::getText(nullptr, tr("Name conflict"), msg, QLineEdit::Normal, nameInConflict, &ok);
    if (ok)
        nameInConflict = newValue;

    return ok;
}
