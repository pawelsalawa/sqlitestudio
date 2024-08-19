#ifndef SCRIPTINGPYTHON_H
#define SCRIPTINGPYTHON_H

#ifdef PYTHON_DYNAMIC_BINDING
#include "dynamicpythonapi.h"
#else
#include "staticpythonapi.h"
#endif

#include "scriptingpython_global.h"
#include "plugins/genericplugin.h"
#include "plugins/scriptingplugin.h"
#include "db/sqlquery.h"
#include <QCache>

class QMutex;

#ifdef PYTHON_DYNAMIC_BINDING

#include "plugins/uiconfiguredplugin.h"
#include "config_builder.h"
#include <QLibrary>
#include <QVersionNumber>

CFG_CATEGORIES(ScriptingPythonConfig,
     CFG_CATEGORY(ScriptingPython,
         CFG_ENTRY(QString,     LibraryPath,         QString())
         CFG_ENTRY(QStringList, DiscoveredLibraries, QStringList(), false)
     )
)

#endif

class SCRIPTINGPYTHONSHARED_EXPORT ScriptingPython : public GenericPlugin,
#ifdef PYTHON_DYNAMIC_BINDING
                                                     public DynamicPythonApi,
                                                     public UiConfiguredPlugin,
#endif
                                                     public DbAwareScriptingPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("scriptingpython.json")

    public:
        static PyObject* dbEval(PyObject *self, PyObject* const* args, Py_ssize_t nargs);
        static PyObject* dbEvalCompat(PyObject *self, PyObject* args);

        ScriptingPython();
        ~ScriptingPython();

        bool init();
        void deinit();
        QString getLanguage() const;
        Context* createContext();
        void releaseContext(Context* context);
        void resetContext(Context* context);
        void setVariable(Context* context, const QString& name, const QVariant& value);
        QVariant getVariable(Context* context, const QString& name);
        bool hasError(Context* context) const;
        QString getErrorMessage(Context* context) const;
        QString getIconPath() const;
        QVariant evaluate(Context* context, const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking = false);
        QVariant evaluate(const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking = false, QString* errorMessage = nullptr);
#ifdef PYTHON_DYNAMIC_BINDING
        QString getConfigUiForm() const;
        CfgMain* getMainUiConfig();
        void configDialogOpen();
        void configDialogClosed();
#endif

    private:
#ifdef PYTHON_DYNAMIC_BINDING
        CFG_LOCAL_PERSISTABLE(ScriptingPythonConfig, cfg)
#endif

        class ContextPython;
        class ScriptObject
        {
            public:
                ScriptObject(const QString& code, const FunctionInfo& funcInfo, ContextPython* context);
                ~ScriptObject();

                PyObject* getCompiled() const;

            private:
                PyObject* compiled = nullptr;
        };

        class ContextPython : public ScriptingPlugin::Context
        {
            public:
                ContextPython();
                ~ContextPython();

                void reset();

                PyThreadState* interp = nullptr;
                PyObject* mainModule = nullptr;
                PyObject* envDict = nullptr;
                QCache<QString, ScriptObject> scriptCache;
                QString error;
                Db* db = nullptr;
                bool useDbLocking = false;

            private:
                void init();
                void clear();
        };

        ContextPython* getContext(ScriptingPlugin::Context* context) const;
        QVariant compileAndEval(ContextPython* ctx, const QString& code, const FunctionInfo& funcInfo,
                                const QList<QVariant>& args, Db* db, bool locking);
        void clearError(ContextPython* ctx);
        ScriptObject* getScriptObject(const QString code, const ScriptingPlugin::FunctionInfo& funcInfo, ContextPython* ctx);
        void initPython();
        void deinitPython();
#ifdef PYTHON_DYNAMIC_BINDING
        bool bindLibrary();
        QStringList discoverLibraries() const;
        void setLibraryPath(QString path);
#endif

        static QString extractError();
        static PyObject* argsToPyArgs(const QVariantList& args, const QStringList& namedParameters);
        static QVariant pythonObjToVariant(PyObject* obj);
        static QString pythonObjToString(PyObject* obj);
        static PyObject* variantToPythonObj(const QVariant& value);
        static PyObject* stringToPythonObj(const QString& value);
        static SqlQueryPtr dbCommonEval(PyObject* sqlArg, const char* fnName);
        static QVariant getVariable(const QString& name);

        static const constexpr int cacheSize = 5;
        static QHash<PyThreadState*, ContextPython*> contexts;

        ContextPython* mainContext = nullptr;
        QMutex* mainInterpMutex = nullptr;
#ifdef PYTHON_DYNAMIC_BINDING
        QString libraryPath;
        QLibrary library;
        QVersionNumber libraryVersion;

    private slots:
        void configModified(CfgEntry* entry);
#endif
};

#endif // SCRIPTINGPYTHON_H
