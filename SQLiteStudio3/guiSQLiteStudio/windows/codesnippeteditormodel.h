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
            VALID = 1002
        };

        explicit CodeSnippetEditorModel(QObject *parent = 0);

        void clearModified();
        bool isModified() const;
        bool isModified(int row) const;
        void setModified(int row, bool modified);
        bool isValid() const;
        bool isValid(int row) const;
        void setValid(int row, bool valid);
        void setCode(int row, const QString& code);
        QString getCode(int row) const;
        void setName(int row, const QString& newName);
        QString getName(int row) const;
        void setHotkey(int row, const QKeySequence& value);
        QKeySequence getHotkey(int row) const;
        void setData(const QList<CodeSnippetManager::CodeSnippet*>& snippets);
        void addSnippet(CodeSnippetManager::CodeSnippet* snippet);
        void deleteSnippet(int row);
        QList<CodeSnippetManager::CodeSnippet*> generateSnippets() const;
        QStringList getSnippetNames() const;
        void validateNames();
        bool isAllowedName(int rowToSkip, const QString& nameToValidate);
        bool isAllowedHotkey(int rowToSkip, const QKeySequence& hotkeyToValidate);
        bool isValidRowIndex(int row) const;

        int moveUp(int row);
        int moveDown(int row);
        int rowCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant data(const QModelIndex& index, int role) const;

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

        void emitDataChanged(int row);

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
};

#endif // CODESNIPPETEDITORMODEL_H
