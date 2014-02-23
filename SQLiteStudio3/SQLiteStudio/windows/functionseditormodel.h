#ifndef FUNCTIONSEDITORMODEL_H
#define FUNCTIONSEDITORMODEL_H

#include "config.h"
#include <QIcon>
#include <QAbstractListModel>

class FunctionsEditorModel : public QAbstractListModel
{
        Q_OBJECT

    public:
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
        bool isModified(const QString& name) const;
        void setModified(const QString& name, bool modified);
        bool isValid(const QString& name) const;
        void setValid(const QString& name, bool valid);
        void setCode(const QString& name, const QString& code);
        QString getCode(const QString& name) const;
        void setName(const QString& name, const QString& newName);
        QString getName(const QString& name) const;
        void setLang(const QString& name, const QString& lang);
        QString getLang(const QString& name) const;
        QStringList getDatabases(const QString& name) const;
        void setDatabases(const QString& name, const QStringList& value);
        QStringList getArguments(const QString& name) const;
        void setArguments(const QString& name, const QStringList& value);
        QString getType(const QString& name) const;
        void setType(const QString& name, const QString& value);
        bool getUndefinedArgs(const QString& name) const;
        void setUndefinedArgs(const QString& name, bool value);
        bool getAllDatabases(const QString& name) const;
        void setAllDatabases(const QString& name, bool value);
        void setData(const QList<Config::Function>& functions);
        void addFunction(const Config::Function& function);
        void deleteFunction(const QString& name);
        void updateFunction(const QString& name, const Config::Function& function);
        QList<Config::Function> getConfigFunctions() const;
        QModelIndex indexOf(const QString& name);
        QStringList getFunctionNames() const;
        void validateNames();
        bool isAllowedName(const QString& name, const QString& nameToValidate);

        int rowCount(const QModelIndex& parent) const;
        QVariant data(const QModelIndex& index, int role) const;

    private:
        struct Function
        {
            enum Type
            {
                SCALAR,
                AGGREGATE
            };

            Function();
            Function(const Config::Function& other);

            Config::Function toConfigFunction() const;

            QString name;
            QString lang;
            QString code;
            QStringList databases;
            QStringList arguments;
            Type type = SCALAR;
            bool undefinedArgs = true;
            bool allDatabases = true;
            bool modified = false;
            bool valid = true;
        };

        void init();
        bool isValidIndex(int row) const;
        void updateSortedList();
        void emitDataChanged(const QString& name);

        QList<Function*> functionList;
        QHash<QString,Function*> functionMap;
        QHash<QString,QIcon> langToIcon;
        bool listModified = false;
};

#endif // FUNCTIONSEDITORMODEL_H
