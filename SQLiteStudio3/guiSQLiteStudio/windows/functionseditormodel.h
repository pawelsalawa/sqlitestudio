#ifndef FUNCTIONSEDITORMODEL_H
#define FUNCTIONSEDITORMODEL_H

#include "services/functionmanager.h"
#include "guiSQLiteStudio_global.h"
#include <QIcon>
#include <QAbstractListModel>

class GUI_API_EXPORT FunctionsEditorModel : public QAbstractListModel
{
        Q_OBJECT

    public:
        using QAbstractItemModel::setData;

        enum Role
        {
            CODE = 1000,
            MODIFIED = 1001,
            VALID = 1002,
            TYPE = 1003
        };

        explicit FunctionsEditorModel(QObject *parent = 0);

        void clearModified();
        bool isModified() const;
        bool isModified(int row) const;
        void setModified(int row, bool modified);
        bool isValid() const;
        bool isValid(int row) const;
        void setValid(int row, bool valid);
        void setCode(int row, const QString& code);
        QString getCode(int row) const;
        void setFinalCode(int row, const QString& code);
        QString getFinalCode(int row) const;
        void setInitCode(int row, const QString& code);
        QString getInitCode(int row) const;
        void setName(int row, const QString& newName);
        QString getName(int row) const;
        void setLang(int row, const QString& lang);
        QString getLang(int row) const;
        QStringList getDatabases(int row) const;
        void setDatabases(int row, const QStringList& value);
        QStringList getArguments(int row) const;
        void setArguments(int row, const QStringList& value);
        FunctionManager::ScriptFunction::Type getType(int row) const;
        void setType(int row, FunctionManager::ScriptFunction::Type type);
        bool isAggregate(int row) const;
        bool isScalar(int row) const;
        void setDeterministic(int row, bool value);
        bool isDeterministic(int row) const;
        bool getUndefinedArgs(int row) const;
        void setUndefinedArgs(int row, bool value);
        bool getAllDatabases(int row) const;
        void setAllDatabases(int row, bool value);
        void setData(const QList<FunctionManager::ScriptFunction*>& functions);
        void addFunction(FunctionManager::ScriptFunction* function);
        void deleteFunction(int row);
        QList<FunctionManager::ScriptFunction*> generateFunctions() const;
        QStringList getFunctionNames() const;
        void validateNames();
        bool isAllowedName(int rowToSkip, const QString& nameToValidate, const QStringList &argList, bool undefinedArgs);
        bool isValidRowIndex(int row) const;

        int rowCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant data(const QModelIndex& index, int role) const;

    private:
        struct UniqueFunctionName
        {
            QString name;
            QStringList arguments;
            bool undefArg;

            int argCount() const;
            bool operator==(const UniqueFunctionName& other) const;
        };

        struct Function
        {
            Function();
            Function(FunctionManager::ScriptFunction* other);
            UniqueFunctionName toUniqueName() const;

            FunctionManager::ScriptFunction data;
            bool modified = false;
            bool valid = true;
            QString originalName;
        };

        void init();
        void emitDataChanged(int row);
        QList<UniqueFunctionName> getUniqueFunctionNames() const;

        friend int qHash(FunctionsEditorModel::UniqueFunctionName fnName);

        QList<Function*> functionList;

        /**
         * @brief List of function pointers before modifications.
         *
         * This list is kept to check for modifications in the overall list of functions.
         * Pointers on this list may be already deleted, so don't use them!
         * It's only used to compare list of pointers to functionList, so it can tell you
         * if the list was modified in regards of adding or deleting functions.
         */
        QList<Function*> originalFunctionList;
        QHash<QString,QIcon> langToIcon;
        bool listModified = false;
};

int qHash(FunctionsEditorModel::UniqueFunctionName fnName);

#endif // FUNCTIONSEDITORMODEL_H
