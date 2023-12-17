#include "scriptingpython.h"
#include "common/global.h"
#include "common/unused.h"
#include "db/db.h"
#include "db/sqlite3.h"
#include "parser/lexer.h"
#include "parser/token.h"
#include "common/utils_sql.h"
#include <QDebug>
#include <QMutexLocker>
#include <frameobject.h>

static PyMethodDef pyDbMethods[] = {
    {"eval", reinterpret_cast<PyCFunction>(ScriptingPython::dbEval), METH_FASTCALL, ""},
    {nullptr, nullptr, 0, nullptr}
};

static PyModuleDef pyDbModule = {
    PyModuleDef_HEAD_INIT, "db", nullptr, -1, pyDbMethods,
    nullptr, nullptr, nullptr, nullptr
};

static PyObject* pyDbModuleInit(void)
{
    return PyModule_Create(&pyDbModule);
}

QHash<PyThreadState*, ScriptingPython::ContextPython*> ScriptingPython::contexts;

ScriptingPython::ScriptingPython()
{
    mainInterpMutex = new QMutex();
}

ScriptingPython::~ScriptingPython()
{
    safe_delete(mainInterpMutex);
}

bool ScriptingPython::init()
{
    SQLS_INIT_RESOURCE(scriptingpython);
    QMutexLocker locker(mainInterpMutex);

    PyImport_AppendInittab("db", &pyDbModuleInit);
    Py_Initialize();
    PyRun_SimpleString("import db");

    mainContext = new ContextPython();
    contexts[mainContext->interp] = mainContext;
    return true;
}

void ScriptingPython::deinit()
{
    QMutexLocker locker(mainInterpMutex);
    contexts.clear();
    Py_Finalize();
    SQLS_CLEANUP_RESOURCE(scriptingpython);
}

QString ScriptingPython::getLanguage() const
{
    return "Python";
}

ScriptingPlugin::Context* ScriptingPython::createContext()
{
    ContextPython* ctx = new ContextPython();
    contexts[ctx->interp] = ctx;
    return ctx;
}

void ScriptingPython::releaseContext(ScriptingPlugin::Context* context)
{
    ContextPython* ctx = getContext(context);
    if (!ctx)
        return;

    contexts.remove(ctx->interp);
    delete ctx;
    PyThreadState_Swap(mainContext->interp);
}

void ScriptingPython::resetContext(ScriptingPlugin::Context* context)
{
    ContextPython* ctx = getContext(context);
    if (!ctx)
        return;

    ctx->reset();
}

void ScriptingPython::setVariable(ScriptingPlugin::Context* context, const QString& name, const QVariant& value)
{
    ContextPython* ctx = getContext(context);
    if (!ctx)
        return;

    PyObject* obj = variantToPythonObj(value);
    PyDict_SetItemString(ctx->envDict, name.toUtf8().constData(), obj);
    Py_DECREF(obj);
}


QVariant ScriptingPython::getVariable(ScriptingPlugin::Context* context, const QString& name)
{
    ContextPython* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    PyThreadState_Swap(ctx->interp);
    return getVariable(name);
}

bool ScriptingPython::hasError(ScriptingPlugin::Context* context) const
{
    ContextPython* ctx = getContext(context);
    if (!ctx)
        return false;

    return !ctx->error.isEmpty();
}

QString ScriptingPython::getErrorMessage(ScriptingPlugin::Context* context) const
{
    ContextPython* ctx = getContext(context);
    if (!ctx)
        return QString();

    return ctx->error;
}

QString ScriptingPython::getIconPath() const
{
    return ":/scriptingpython/scriptingpython.png";
}

QVariant ScriptingPython::evaluate(ScriptingPlugin::Context* context, const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking)
{
    ContextPython* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    return compileAndEval(ctx, code, funcInfo, args, db, locking);
}

QVariant ScriptingPython::evaluate(const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking, QString* errorMessage)
{
    QMutexLocker locker(mainInterpMutex);
    QVariant results = compileAndEval(mainContext, code, funcInfo, args, db, locking);

    if (errorMessage && !mainContext->error.isEmpty())
        *errorMessage = mainContext->error;

    return results;
}

ScriptingPython::ContextPython* ScriptingPython::getContext(ScriptingPlugin::Context* context) const
{
    ContextPython* ctx = dynamic_cast<ContextPython*>(context);
    if (!ctx)
        qDebug() << "Invalid context passed to ScriptingPython:" << context;

    return ctx;
}

QVariant ScriptingPython::compileAndEval(ScriptingPython::ContextPython* ctx, const QString& code, const FunctionInfo& funcInfo,
                                         const QList<QVariant>& args, Db* db, bool locking)
{
    PyThreadState_Swap(ctx->interp);
    clearError(ctx);

    ScriptObject* scriptObj = getScriptObject(code, funcInfo, ctx);
    if (PyErr_Occurred() || !scriptObj->getCompiled())
    {
        ctx->error = extractError();
        return QVariant();
    }

    ctx->db = db;
    ctx->useDbLocking = locking;

    PyObject* pyArgs = argsToPyArgs(args, funcInfo.getArguments());
    PyObject* result = PyObject_CallObject(scriptObj->getCompiled(), pyArgs);
    Py_DECREF(pyArgs);

    ctx->db = nullptr;
    ctx->useDbLocking = false;

    if (PyErr_Occurred())
    {
        Py_XDECREF(result);
        ctx->error = extractError();
        return QVariant();
    }

    QVariant resultVariant = pythonObjToVariant(result);
    Py_XDECREF(result);
    return resultVariant;
}

QString ScriptingPython::extractError()
{
    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    if (!value)
        return QString();

    PyObject* strErr = PyObject_Repr(value);
    QString err = QString::fromUtf8(PyUnicode_AsUTF8(strErr));
    PyErr_Clear();

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
    Py_XDECREF(strErr);

    return err;
}

void ScriptingPython::clearError(ContextPython* ctx)
{
    PyErr_Clear();
    ctx->error.clear();
}

ScriptingPython::ScriptObject* ScriptingPython::getScriptObject(const QString code, const ScriptingPlugin::FunctionInfo& funcInfo, ContextPython* ctx)
{
    static const QString keyTpl = QStringLiteral("{%1} %2");

    QString key = keyTpl.arg(funcInfo.getArguments().join("#"), code);
    if (ctx->scriptCache.contains(key))
        return ctx->scriptCache[key];

    ScriptObject* scriptObj = new ScriptObject(code, funcInfo, ctx);
    ctx->scriptCache.insert(key, scriptObj);
    return scriptObj;
}

PyObject* ScriptingPython::argsToPyArgs(const QVariantList& args, const QStringList& namedParameters)
{
    PyObject* result = PyTuple_New(args.size());
    PyObject* namedParamTuple = namedParameters.isEmpty() ? nullptr : PyTuple_New(namedParameters.size() + 1);
    int i = 0;
    for (const QVariant& value : args)
    {
        PyObject* valueObj = variantToPythonObj(value);
        PyTuple_SetItem(result, i, valueObj);
        if (namedParamTuple && i < namedParameters.size())
        {
            Py_INCREF(valueObj);
            PyTuple_SetItem(namedParamTuple, i, valueObj);
        }
        i++;
    }

    // If function has named parameters, than named param tuple takes precedense
    // and positional tuple becomes last argument.
    if (namedParamTuple)
    {
        PyTuple_SetItem(namedParamTuple, namedParameters.size(), result);
        result = namedParamTuple;
    }

    return result;
}

QVariant ScriptingPython::pythonObjToVariant(PyObject* obj)
{
    if (!obj)
        return QVariant();

    if (PyUnicode_Check(obj))
        return QString::fromUtf8(PyUnicode_AsUTF8(obj));

    if (PyLong_Check(obj))
        return PyLong_AsLongLong(obj);

    if (PyBool_Check(obj))
        return PyObject_IsTrue(obj) != 0;

    if (PyFloat_Check(obj))
        return PyFloat_AsDouble(obj);

    if (PyBytes_Check(obj))
    {
        char* buf;
        Py_ssize_t size;
        if (PyBytes_AsStringAndSize(obj, &buf, &size) != 0)
            return QVariant();

        return QByteArray::fromRawData(buf, size);
    }

    if (PyByteArray_Check(obj))
    {
        char* buf = PyByteArray_AsString(obj);
        Py_ssize_t size = PyByteArray_Size(obj);
        return QByteArray::fromRawData(buf, size);
    }

    if (PyTuple_Check(obj))
    {
        QList<QVariant> result;

        Py_ssize_t size = PyTuple_Size(obj);
        for (Py_ssize_t i = 0; i < size; i++)
            result << pythonObjToVariant(PyTuple_GetItem(obj, i));

        return result;
    }

    if (PyList_Check(obj))
    {
        QList<QVariant> result;

        Py_ssize_t size = PyList_Size(obj);
        for (Py_ssize_t i = 0; i < size; i++)
            result << pythonObjToVariant(PyList_GetItem(obj, i));

        return result;
    }

    if (PySet_Check(obj))
    {
        QSet<QVariant> result;

        PyObject *iterator = PyObject_GetIter(obj);
        PyObject *item;

        if (iterator == NULL)
            return QVariant();

        while ((item = PyIter_Next(iterator)))
        {
            result << pythonObjToVariant(item);
            Py_DECREF(item);
        }

        Py_DECREF(iterator);

        return QVariant::fromValue(result);
    }

    if (PyDict_Check(obj))
    {
        QHash<QString, QVariant> result;

        PyObject *key, *value;
        Py_ssize_t pos = 0;

        while (PyDict_Next(obj, &pos, &key, &value))
            result[pythonObjToString(key)] = pythonObjToVariant(value);

        return result;
    }

    PyObject* strObj = PyObject_Repr(obj);
    QString result = QString::fromUtf8(PyUnicode_AsUTF8(strObj));
    Py_DECREF(strObj);
    return result;
}

QString ScriptingPython::pythonObjToString(PyObject* obj)
{
    PyObject* strObj = PyObject_Repr(obj);
    if (!strObj)
        return QString();

    QString result = QString::fromUtf8(PyUnicode_AsUTF8(strObj));
    Py_DECREF(strObj);
    return result;
}

PyObject* ScriptingPython::variantToPythonObj(const QVariant& value)
{
    PyObject* obj = nullptr;
    switch (value.type())
    {
        case QVariant::Bool:
            obj = PyBool_FromLong(value.toBool() ? 1L : 0L);
            break;
        case QVariant::Int:
        case QVariant::LongLong:
            obj = PyLong_FromLongLong(value.toLongLong());
            break;
        case QVariant::UInt:
        case QVariant::ULongLong:
            obj = PyLong_FromUnsignedLongLong(value.toULongLong());
            break;
        case QVariant::Double:
            obj = PyFloat_FromDouble(value.toDouble());
            break;
        case QVariant::ByteArray:
        {
            QByteArray bytes = value.toByteArray();
            char* data = bytes.data();
            obj = PyBytes_FromStringAndSize(data, bytes.size());
            break;
        }
        case QVariant::List:
        {
            QList<QVariant> list = value.toList();
            int listSize = list.size();
            obj = PyList_New(listSize);
            int pos = 0;
            for (const QVariant& item : list)
            {
                PyObject* subObj = variantToPythonObj(item);
                PyList_SetItem(obj, pos++, subObj);
                Py_DECREF(subObj);
            }

            break;
        }
        case QVariant::StringList:
        {
            QStringList list = value.toStringList();
            int listSize = list.size();
            obj = PyList_New(listSize);
            int pos = 0;
            for (const QString& item : list)
            {
                PyObject* subObj = stringToPythonObj(item);
                PyList_SetItem(obj, pos++, subObj);
                Py_DECREF(subObj);
            }

            break;
        }
        case QVariant::Hash:
        {
            QHash<QString, QVariant> hash = value.toHash();
            obj = PyDict_New();
            QHashIterator<QString, QVariant> it(hash);
            while (it.hasNext())
            {
                it.next();
                PyObject* subObj = variantToPythonObj(it.value());
                PyDict_SetItemString(obj, it.key().toUtf8().constData(), subObj);
                Py_DECREF(subObj);
            }
            break;
        }
        case QVariant::Map:
        {
            QMap<QString, QVariant> map = value.toMap();
            obj = PyDict_New();
            QMapIterator<QString, QVariant> it(map);
            while (it.hasNext())
            {
                it.next();
                PyObject* subObj = variantToPythonObj(it.value());
                PyDict_SetItemString(obj, it.key().toUtf8().constData(), subObj);
                Py_DECREF(subObj);
            }
            break;
        }
        case QVariant::String:
        default:
            obj = stringToPythonObj(value.toString());
            break;
    }

    if (!obj)
        obj = stringToPythonObj(value.toString());

    return obj;
}

PyObject* ScriptingPython::stringToPythonObj(const QString& value)
{
    QByteArray bytes = value.toUtf8();
    return PyUnicode_FromStringAndSize(bytes.constData(), bytes.size());
}

PyObject* ScriptingPython::dbEval(PyObject* self, PyObject *const *args, Py_ssize_t nargs)
{
    UNUSED(self);

    if (nargs != 1)
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Invalid use of %1 function. Expected %2 arguments, but got %3.")
                        .arg("db.eval()", QString::number(1), QString::number(nargs))
                        .toUtf8().constData());
        return nullptr;
    }

    SqlQueryPtr execResults = dbCommonEval(args[0], "db.eval()");
    if (!execResults)
    {
        PyErr_SetString(PyExc_RuntimeError, QObject::tr("Unknown error from function %1.").arg("db.eval()").toUtf8().constData());
        return nullptr;
    }

    if (execResults->isError())
    {
        PyErr_SetString(PyExc_RuntimeError, execResults->getErrorText().toUtf8().constData());
        return nullptr;
    }

    QList<PyObject*> tuples;
    while (execResults->hasNext())
    {
        SqlResultsRowPtr row = execResults->next();
        PyObject* tuple = PyTuple_New(row->valueList().size());
        int pos = 0;
        for (const QVariant& cell : row->valueList())
            PyTuple_SetItem(tuple, pos++, variantToPythonObj(cell));

        tuples << tuple;
    }

    PyObject* resultObject = PyList_New(0);
    for (PyObject* tuple : tuples)
    {
        PyList_Append(resultObject, tuple);
        Py_DECREF(tuple);
    }

    return resultObject;
}

SqlQueryPtr ScriptingPython::dbCommonEval(PyObject* sqlArg, const char* fnName)
{
    QString sql;
    if (!PyUnicode_Check(sqlArg))
    {
        PyObject* strObj = PyObject_Repr(sqlArg);
        if (!strObj)
        {
            return SqlQuery::error(
                        QObject::tr("Could not calculate string representation of the Python object passed as argument to the function %1.")
                            .arg(fnName),
                        SQLITE_ERROR);
        }

        sql = QString::fromUtf8(PyUnicode_AsUTF8(strObj));
        Py_DECREF(strObj);
    }
    else
        sql = QString::fromUtf8(PyUnicode_AsUTF8(sqlArg));

    PyThreadState* state = PyThreadState_Get();
    ContextPython* ctx = contexts[state];
    if (!ctx)
    {
        return SqlQuery::error(
                    QObject::tr("Could not find execution context for function %1. This is a bug of Python plugin. Please report it.")
                        .arg(fnName),
                    SQLITE_ERROR);
    }

    Db::Flags flags;
    if (!ctx->useDbLocking)
        flags |= Db::Flag::NO_LOCK;

    TokenList bindTokens = Lexer::tokenize(sql).filter(Token::BIND_PARAM);
    QString bindVarName;
    QHash<QString, QVariant> queryArgs;
    for (const TokenPtr& token : bindTokens)
    {
        bindVarName = getBindTokenName(token);
        if (bindVarName == "?")
            continue;

        queryArgs[token->value] = getVariable(bindVarName);
    }

    SqlQueryPtr execResults = ctx->db->exec(sql, queryArgs, flags);
    if (execResults->isError())
    {
        return SqlQuery::error(
                    QObject::tr("Error from Python function %1: %2")
                        .arg(fnName, execResults->getErrorText()),
                    SQLITE_ERROR);
    }
    return execResults;
}

QVariant ScriptingPython::getVariable(const QString& name)
{
    PyThreadState* state = PyThreadState_Get();
#if PY_VERSION_HEX < 0x03090000
    PyFrameObject* frame = state->frame;
#else
    PyFrameObject* frame = PyThreadState_GetFrame(state);
#endif
    if (!frame)
        return QVariant();

    const char* varName = name.toUtf8().constData();
    PyObject* obj = nullptr;

    PyFrame_FastToLocals(frame);
#if PY_VERSION_HEX < 0x030b0000
    PyObject* locals = frame->f_locals;
    PyObject* globals = frame->f_globals;
#else
    PyObject* locals = PyFrame_GetLocals(frame);
    PyObject* globals = PyFrame_GetGlobals(frame);
#endif
    if (PyMapping_Check(locals))
        obj = PyMapping_GetItemString(locals, varName);
    else if (PyDict_Check(globals))
        obj = PyDict_GetItemString(globals, varName);

    if (!obj)
        return QVariant();

    return pythonObjToVariant(obj);
}

ScriptingPython::ScriptObject::ScriptObject(const QString& code, const ScriptingPlugin::FunctionInfo& funcInfo, ContextPython* context)
{
    static_qstring(fnTpl, "def fn(%1%2*args):\n%3");

    QString indentedCode = indentMultiline(code);
    QByteArray utf8Bytes = funcInfo.getUndefinedArgs() ?
                fnTpl.arg("", "", indentedCode).toUtf8() :
                fnTpl.arg(funcInfo.getArguments().join(", "), ", ", indentedCode).toUtf8();

    PyObject* result = PyRun_String(utf8Bytes.constData(), Py_file_input, context->envDict, context->envDict);
    if (!result)
        return;

    Py_DECREF(result);
    compiled = PyObject_GetAttrString(context->mainModule, "fn");
}

ScriptingPython::ScriptObject::~ScriptObject()
{
    Py_CLEAR(compiled);
}

PyObject* ScriptingPython::ScriptObject::getCompiled() const
{
    return compiled;
}

ScriptingPython::ContextPython::ContextPython()
{
    scriptCache.setMaxCost(cacheSize);
    init();
}

ScriptingPython::ContextPython::~ContextPython()
{
    clear();
}

void ScriptingPython::ContextPython::reset()
{
    clear();
    init();
}

void ScriptingPython::ContextPython::init()
{
    interp = Py_NewInterpreter();
    PyThreadState_Swap(interp);
    mainModule = PyImport_AddModule("__main__");
    envDict = PyModule_GetDict(mainModule);
    PyRun_SimpleString("import db");
}

void ScriptingPython::ContextPython::clear()
{
    PyThreadState_Swap(interp);
    PyDict_Clear(envDict);
    scriptCache.clear();
    PyErr_Clear();
    Py_EndInterpreter(interp);
    error = QString();
}
