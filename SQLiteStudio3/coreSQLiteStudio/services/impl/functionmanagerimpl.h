#ifndef FUNCTIONMANAGERIMPL_H
#define FUNCTIONMANAGERIMPL_H

#include "services/functionmanager.h"
#include <QCryptographicHash>

class SqlFunctionPlugin;
class Plugin;
class PluginType;

class API_EXPORT FunctionManagerImpl : public FunctionManager
{
    Q_OBJECT

    public:
        FunctionManagerImpl();

        void setScriptFunctions(const QList<ScriptFunction*>& newFunctions);
        QList<ScriptFunction*> getAllScriptFunctions() const;
        QList<ScriptFunction*> getScriptFunctionsForDatabase(const QString& dbName) const;
        QList<NativeFunction*> getAllNativeFunctions() const;
        QVariant evaluateScalar(const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok);
        void evaluateAggregateInitial(const QString& name, int argCount, Db* db, QHash<QString, QVariant>& aggregateStorage);
        void evaluateAggregateStep(const QString& name, int argCount, const QList<QVariant>& args, Db* db, QHash<QString, QVariant>& aggregateStorage);
        QVariant evaluateAggregateFinal(const QString& name, int argCount, Db* db, bool& ok, QHash<QString, QVariant>& aggregateStorage);
        QVariant evaluateScriptScalar(ScriptFunction* func, const QString& name, int argCount, const QList<QVariant>& args, Db* db, bool& ok);
        void evaluateScriptAggregateInitial(ScriptFunction* func, Db* db,
                                            QHash<QString, QVariant>& aggregateStorage);
        void evaluateScriptAggregateStep(ScriptFunction* func, const QList<QVariant>& args, Db* db,
                                         QHash<QString, QVariant>& aggregateStorage);
        QVariant evaluateScriptAggregateFinal(ScriptFunction* func, const QString& name, int argCount, Db* db, bool& ok,
                                              QHash<QString, QVariant>& aggregateStorage);
        QVariant evaluateNativeScalar(NativeFunction* func, const QList<QVariant>& args, Db* db, bool& ok);

    private:
        struct Key
        {
            Key();
            Key(FunctionBase* function);

            QString name;
            int argCount;
            FunctionBase::Type type;
        };

        friend TYPE_OF_QHASH qHash(const FunctionManagerImpl::Key& key);
        friend bool operator==(const FunctionManagerImpl::Key& key1, const FunctionManagerImpl::Key& key2);

        void init();
        void initNativeFunctions();
        void refreshFunctionsByKey();
        void refreshNativeFunctionsByKey();
        void storeInConfig();
        void loadFromConfig();
        void clearFunctions();
        QString cannotFindFunctionError(const QString& name, int argCount);
        QString langUnsupportedError(const QString& name, int argCount, const QString& lang);
        void registerNativeFunction(const QString& name, const QStringList& args, NativeFunction::ImplementationFunction funcPtr);
        QString updateScriptingQtLang(const QString& lang) const;

        static QStringList getArgMarkers(int argCount);
        static QVariant nativeRegExp(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeSqlFile(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeReadFile(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeWriteFile(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeScript(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeLangs(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeHtmlEscape(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeUrlEncode(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeUrlDecode(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeBase64Encode(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeBase64Decode(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeCryptographicFunction(const QList<QVariant>& args, Db* db, bool& ok, QCryptographicHash::Algorithm algo);
        static QVariant nativeMd4(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeMd4Hex(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeMd5(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeMd5Hex(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeSha1(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeSha224(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeSha256(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeSha384(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeSha512(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeSha3_224(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeSha3_256(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeSha3_384(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeSha3_512(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeImport(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeImportFormats(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeImportOptions(const QList<QVariant>& args, Db* db, bool& ok);
        static QVariant nativeCharsets(const QList<QVariant>& args, Db* db, bool& ok);

        QList<ScriptFunction*> functions;
        QHash<Key,ScriptFunction*> functionsByKey;
        QList<NativeFunction*> nativeFunctions;
        QHash<Key,NativeFunction*> nativeFunctionsByKey;
};

TYPE_OF_QHASH qHash(const FunctionManagerImpl::Key& key);
bool operator==(const FunctionManagerImpl::Key& key1, const FunctionManagerImpl::Key& key2);

#endif // FUNCTIONMANAGERIMPL_H
