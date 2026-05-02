#ifndef CODESNIPPETEDITORMODEL_H
#define CODESNIPPETEDITORMODEL_H

#include "guiSQLiteStudio_global.h"
#include "services/codesnippetmanager.h"
#include <QAbstractListModel>

class GUI_API_EXPORT CodeSnippetEditorModel : public QAbstractListModel
{
    Q_OBJECT

    public:
        using QAbstractItemModel::setData;

        enum Role
        {
            CODE = 1000,
            MODIFIED = 1001,
            VALID = 1002,
            HOTKEY = 1003
        };

        explicit CodeSnippetEditorModel(QObject *parent = 0);

        void clearModified();
        bool isModified() const;
        bool isValid() const;
        void setSnippets(const QList<CodeSnippetManager::CodeSnippet*>& snippets);
        void addSnippet(CodeSnippetManager::CodeSnippet* snippet);
        void deleteSnippet(const QModelIndex& idx);
        QList<CodeSnippetManager::CodeSnippet*> generateSnippets() const;
        QStringList getSnippetNames() const;
        void validateNames();
        bool isAllowedName(const QModelIndex& idx, const QString& nameToValidate);
        bool isAllowedHotkey(const QModelIndex& idx, const QKeySequence& hotkeyToValidate);
        void setDragEnabled(bool value);

        QModelIndex moveUp(const QModelIndex& idx);
        QModelIndex moveDown(const QModelIndex& idx);
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role) const override;
        bool setData(const QModelIndex& index, const QVariant& value, int role) override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QStringList mimeTypes() const override;
        QMimeData* mimeData(const QModelIndexList &indexes) const override;

        static const constexpr char* MIMETYPE = "application/x-sqlitestudio-codesnippet";

    private:
        struct Snippet
        {
            Snippet();
            Snippet(CodeSnippetManager::CodeSnippet* other);

            CodeSnippetManager::CodeSnippet data;
            bool modified = false;
            bool valid = true;
            QString originalName;
        };

        QList<Snippet*> snippetList;

        /**
         * @brief List of snippets pointers before modifications.
         *
         * This list is kept to check for modifications in the overall list of snippets.
         * Pointers on this list may be already deleted, so don't use them!
         * It's only used to compare list of pointers to snippetList, so it can tell you
         * if the list was modified in regards of adding or deleting functions.
         */
        QList<Snippet*> originalSnippetList;
        bool listModified = false;

        /**
         * @brief Whether dragging snippets outside of the list is enabled.
         */
        bool dragEnabled = false;
};

#endif // CODESNIPPETEDITORMODEL_H
