#ifndef SCRIPTINGPYTHON_H
#define SCRIPTINGPYTHON_H

#include <Python.h>

#include "scriptingpython_global.h"
#include "plugins/genericplugin.h"
#include "plugins/scriptingplugin.h"
#include "db/sqlquery.h"
#include <QCache>

class QMutex;

class SCRIPTINGPYTHONSHARED_EXPORT ScriptingPython : public GenericPlugin, public DbAwareScriptingPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("scriptingpython.json")

    public:
        static PyObject* dbEval(PyObject *self, PyObject* const* args, Py_ssize_t nargs);

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
        QVariant evaluate(Context* context, const QString& code, const QList<QVariant>& args, Db* db, bool locking = false);
        QVariant evaluate(const QString& code, const QList<QVariant>& args, Db* db, bool locking = false, QString* errorMessage = nullptr);

    private:
        class ContextPython;
        class ScriptObject
        {
            public:
                ScriptObject(const QString& code, ContextPython* context);
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
        QVariant compileAndEval(ContextPython* ctx, const QString& code, const QList<QVariant>& args, Db* db, bool locking);
        void clearError(ContextPython* ctx);

        static QString extractError();
        static PyObject* argsToPyArgs(const QVariantList& args);
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
};

#endif // SCRIPTINGPYTHON_H
