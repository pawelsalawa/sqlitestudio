#include "scriptingtcl.h"
#include "common/global.h"
#include "common/unused.h"
#include "db/db.h"
#include "db/sqlquery.h"
#include <QDebug>
#include <QMutexLocker>

Db* ScriptingTcl::currentDb = nullptr;
bool ScriptingTcl::useDbLocking = false;

ScriptingTcl::ScriptingTcl()
{
    dbMutex = new QMutex();
    mainInterpMutex = new QMutex();
}

ScriptingTcl::~ScriptingTcl()
{
    safe_delete(mainInterpMutex);
    safe_delete(dbMutex);
}

bool ScriptingTcl::init()
{
    Q_INIT_RESOURCE(scriptingtcl);
    QMutexLocker locker(mainInterpMutex);
    mainContext = new ContextTcl();
    return true;
}

void ScriptingTcl::deinit()
{
    QMutexLocker locker(mainInterpMutex);
    safe_delete(mainContext);
    Tcl_Finalize();
    Q_CLEANUP_RESOURCE(scriptingtcl);
}

QString ScriptingTcl::getLanguage() const
{
    return "Tcl";
}

ScriptingPlugin::Context* ScriptingTcl::createContext()
{
    ContextTcl* ctx = new ContextTcl();
    contexts << ctx;
    return ctx;
}

void ScriptingTcl::releaseContext(ScriptingPlugin::Context* context)
{
    ContextTcl* ctx = getContext(context);
    if (!ctx)
        return;

    contexts.removeOne(ctx);
    delete ctx;
}

void ScriptingTcl::resetContext(ScriptingPlugin::Context* context)
{
    ContextTcl* ctx = getContext(context);
    if (!ctx)
        return;

    ctx->reset();
}

void ScriptingTcl::setVariable(ScriptingPlugin::Context* context, const QString& name, const QVariant& value)
{
    ContextTcl* ctx = getContext(context);
    if (!ctx)
        return;

    Tcl_Obj* varName = Tcl_NewStringObj(name.toUtf8().constData(), -1);
    Tcl_IncrRefCount(varName);
    Tcl_Obj* tclObjValue = variantToTclObj(value);
    Tcl_IncrRefCount(tclObjValue);
    Tcl_ObjSetVar2(ctx->interp, varName, nullptr, tclObjValue, TCL_GLOBAL_ONLY);
    Tcl_DecrRefCount(tclObjValue);
    Tcl_DecrRefCount(varName);
}

QVariant ScriptingTcl::getVariable(ScriptingPlugin::Context* context, const QString& name)
{
    ContextTcl* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    Tcl_Obj* varName = Tcl_NewStringObj(name.toUtf8().constData(), -1);
    Tcl_IncrRefCount(varName);
    Tcl_Obj* obj = Tcl_ObjGetVar2(ctx->interp, varName, nullptr, TCL_GLOBAL_ONLY);
    Tcl_IncrRefCount(obj);
    QVariant val = tclObjToVariant(obj);
    Tcl_DecrRefCount(varName);
    Tcl_DecrRefCount(obj);
    return val;
}

bool ScriptingTcl::hasError(ScriptingPlugin::Context* context) const
{
    ContextTcl* ctx = getContext(context);
    if (!ctx)
        return false;

    return !ctx->error.isEmpty();
}

QString ScriptingTcl::getErrorMessage(ScriptingPlugin::Context* context) const
{
    ContextTcl* ctx = getContext(context);
    if (!ctx)
        return QString();

    return ctx->error;
}

QString ScriptingTcl::getIconPath() const
{
    return ":/scriptingtcl/scriptingtcl.png";
}

QVariant ScriptingTcl::evaluate(ScriptingPlugin::Context* context, const QString& code, const QList<QVariant>& args, Db* db, bool locking)
{
    ContextTcl* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    setArgs(ctx, args);
    return compileAndEval(ctx, code, db, locking);
}

QVariant ScriptingTcl::evaluate(const QString& code, const QList<QVariant>& args, Db* db, bool locking, QString* errorMessage)
{
    QMutexLocker locker(mainInterpMutex);
    setArgs(mainContext, args);
    QVariant results = compileAndEval(mainContext, code, db, locking);

    if (errorMessage && !mainContext->error.isEmpty())
        *errorMessage = mainContext->error;

    return results;
}

ScriptingTcl::ContextTcl* ScriptingTcl::getContext(ScriptingPlugin::Context* context) const
{
    ContextTcl* ctx = dynamic_cast<ContextTcl*>(context);
    if (!ctx)
        qDebug() << "Invalid context passed to ScriptingTcl:" << context;

    return ctx;
}

QVariant ScriptingTcl::compileAndEval(ScriptingTcl::ContextTcl* ctx, const QString& code, Db* db, bool locking)
{
    ScriptObject* scriptObj = nullptr;
    if (!ctx->scriptCache.contains(code))
    {
        scriptObj = new ScriptObject(code);
        ctx->scriptCache.insert(code, scriptObj);
    }
    else
    {
        scriptObj = ctx->scriptCache[code];
    }
    Tcl_ResetResult(ctx->interp);
    ctx->error.clear();

    dbMutex->lock();
    currentDb = db;
    useDbLocking = locking;

    int result = Tcl_EvalObjEx(ctx->interp, scriptObj->getTclObj(), TCL_EVAL_GLOBAL);

    currentDb = nullptr;
    useDbLocking = false;
    dbMutex->unlock();

    if (result != TCL_OK)
    {
        ctx->error = QString::fromUtf8(Tcl_GetStringResult(ctx->interp));
        return QVariant();
    }
    return extractResult(ctx);
}

QVariant ScriptingTcl::extractResult(ScriptingTcl::ContextTcl* ctx)
{
    Tcl_Obj* obj = Tcl_GetObjResult(ctx->interp);
    return tclObjToVariant(obj);
}

void ScriptingTcl::setArgs(ScriptingTcl::ContextTcl* ctx, const QList<QVariant>& args)
{
    setVariable(ctx, "argc", args.size());
    setVariable(ctx, "argv", args);
}

Tcl_Obj* ScriptingTcl::argsToList(const QList<QVariant>& args)
{
    Tcl_Obj** objArray = new Tcl_Obj*[args.size()];

    int i = 0;
    for (const QVariant& arg : args)
        objArray[i++] = variantToTclObj(arg);

    Tcl_Obj* obj = Tcl_NewListObj(args.size(), objArray);
    delete[] objArray;

    return obj;
}

QVariant ScriptingTcl::tclObjToVariant(Tcl_Obj* obj)
{
    static const QStringList typeLiterals = {"boolean", "booleanString", "double", "int", "wideInt", "bignum", "bytearray", "string", "list", "dict"};

    TclDataType type = TclDataType::UNKNOWN;
    if (obj->typePtr)
    {
        int typeIdx = typeLiterals.indexOf(obj->typePtr->name);
        if (typeIdx > -1)
            type = static_cast<TclDataType>(typeIdx);
    }

    QVariant result;
    bool ok = true;
    switch (type)
    {
        case TclDataType::Boolean:
        case TclDataType::BooleanString:
        {
            int b;
            if (Tcl_GetBooleanFromObj(nullptr, obj, &b) == TCL_OK)
                result = (bool)b;
            else
                ok = false;

            break;
        }
        case TclDataType::Double:
        {
            double d;
            if (Tcl_GetDoubleFromObj(nullptr, obj, &d) == TCL_OK)
                result = d;
            else
                ok = false;

            break;
        }
        case TclDataType::Int:
        {
            int i;
            if (Tcl_GetIntFromObj(nullptr, obj, &i) == TCL_OK)
                result = i;
            else
                ok = false;

            break;
        }
        case TclDataType::WideInt:
        {
            Tcl_WideInt wideInt;
            if (Tcl_GetWideIntFromObj(nullptr, obj, &wideInt) == TCL_OK)
                result = (qint64)wideInt;
            else
                ok = false;

            break;
        }
        case TclDataType::Bytearray:
        {
            int lgt;
            unsigned char* bytes = Tcl_GetByteArrayFromObj(obj, &lgt);
            result = QByteArray::fromRawData(reinterpret_cast<char*>(bytes), lgt);
            break;
        }
        case TclDataType::List:
        {
            QList<QVariant> list;
            int objc;
            Tcl_Obj** objv;
            Tcl_ListObjGetElements(nullptr, obj, &objc, &objv);
            for (int i = 0; i < objc; i++)
                list << tclObjToVariant(objv[i]);

            result = list;
            break;
        }
        case TclDataType::Dict:
        {
            Tcl_DictSearch search;
            Tcl_Obj* key;
            Tcl_Obj* value;
            QString keyStr;
            QVariant valueVariant;
            int done;
            QHash<QString,QVariant> hash;
            if (Tcl_DictObjFirst(nullptr, obj, &search, &key, &value, &done) == TCL_OK)
            {
                for (; !done ; Tcl_DictObjNext(&search, &key, &value, &done))
                {
                    keyStr = QString::fromUtf8(Tcl_GetStringFromObj(key, nullptr));
                    valueVariant = tclObjToVariant(value);
                    hash[keyStr] = valueVariant;
                }
                Tcl_DictObjDone(&search);
            }
            result = hash;
        }
        case TclDataType::Bignum:
        case TclDataType::String:
        case TclDataType::UNKNOWN:
        default:
            result = QString::fromUtf8(Tcl_GetStringFromObj(obj, nullptr));
            break;
    }

    if (!ok)
        result = QString::fromUtf8(Tcl_GetStringFromObj(obj, nullptr));

    return result;
}

Tcl_Obj* ScriptingTcl::variantToTclObj(const QVariant& value)
{
    Tcl_Obj* obj = nullptr;
    switch (value.type())
    {
        case QVariant::Bool:
            obj = Tcl_NewBooleanObj(value.toBool());
            break;
        case QVariant::Int:
        case QVariant::UInt:
            obj = Tcl_NewIntObj(value.toInt());
            break;
        case QVariant::LongLong:
        case QVariant::ULongLong:
            obj = Tcl_NewWideIntObj((Tcl_WideInt)value.toLongLong());
            break;
        case QVariant::Double:
            obj = Tcl_NewDoubleObj(value.toDouble());
            break;
        case QVariant::ByteArray:
        {
            QByteArray bytes = value.toByteArray();
            unsigned char* ubytes = reinterpret_cast<unsigned char*>(bytes.data());
            obj = Tcl_NewByteArrayObj(ubytes, bytes.size());
            break;
        }
        case QVariant::List:
        {
            QList<QVariant> list = value.toList();
            int listSize = list.size();
            Tcl_Obj** objList = new Tcl_Obj*[listSize];
            for (int i = 0; i < listSize; ++i)
                objList[i] = variantToTclObj(list[i]);

            obj = Tcl_NewListObj(listSize, objList);
            delete[] objList;
            break;
        }
        case QVariant::Hash:
        {
            QHash<QString, QVariant> hash = value.toHash();
            obj = Tcl_NewDictObj();
            QHashIterator<QString, QVariant> it(hash);
            while (it.hasNext())
            {
                it.next();
                Tcl_DictObjPut(nullptr, obj, variantToTclObj(it.key()), variantToTclObj(it.value()));
            }
            break;
        }
        case QVariant::String:
        default:
            obj = Tcl_NewStringObj(value.toString().toUtf8().constData(), -1);
            break;
    }

    if (!obj)
        obj = Tcl_NewStringObj(value.toString().toUtf8().constData(), -1);

    return obj;
}

int ScriptingTcl::dbCommand(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    UNUSED(clientData);

    Tcl_Obj* result = nullptr;
    if (!currentDb)
    {
        result = Tcl_NewStringObj(tr("No database available in current context, while called Tcl's 'db' command.").toUtf8().constData(), -1);
        Tcl_SetObjResult(interp, result);
        return TCL_ERROR;
    }

    if (objc == 3 && strcmp(Tcl_GetStringFromObj(objv[1], nullptr), "eval") == 0)
        return dbEval(interp, objv);

    result = Tcl_NewStringObj(tr("Invalid 'db' command sytax. Should be: db eval sql").toUtf8().constData(), -1);
    Tcl_SetObjResult(interp, result);
    return TCL_ERROR;
}

int ScriptingTcl::dbEval(Tcl_Interp* interp, Tcl_Obj* const objv[])
{
    Db::Flags flags;
    if (!useDbLocking)
        flags |= Db::Flag::NO_LOCK;

    Tcl_Obj* result = nullptr;
    QString sql = QString::fromUtf8(Tcl_GetStringFromObj(objv[2], nullptr));
    SqlQueryPtr execResults = currentDb->exec(sql, QList<QVariant>(), flags);
    if (execResults->isError())
    {
        result = Tcl_NewStringObj(tr("Error from Tcl's' 'db' command: %1").arg(execResults->getErrorText()).toUtf8().constData(), -1);
        Tcl_SetObjResult(interp, result);
        return TCL_ERROR;
    }

    QList<QVariant> rows;
    SqlResultsRowPtr row;
    while (execResults->hasNext())
    {
        row = execResults->next();
        rows << QVariant(row->valueList());
    }
    result = variantToTclObj(rows);

    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

ScriptingTcl::ScriptObject::ScriptObject(const QString& code)
{
    QByteArray utf8Bytes = code.toUtf8();
    obj = Tcl_NewStringObj(utf8Bytes.constData(), utf8Bytes.size());
    Tcl_IncrRefCount(obj);
}

ScriptingTcl::ScriptObject::~ScriptObject()
{
    Tcl_DecrRefCount(obj);
}

Tcl_Obj* ScriptingTcl::ScriptObject::getTclObj()
{
    return obj;
}

ScriptingTcl::ContextTcl::ContextTcl()
{
    scriptCache.setMaxCost(cacheSize);
    interp = Tcl_CreateInterp();
    init();
}

ScriptingTcl::ContextTcl::~ContextTcl()
{
    Tcl_DeleteInterp(interp);
}

void ScriptingTcl::ContextTcl::reset()
{
    Tcl_DeleteInterp(interp);
    interp = Tcl_CreateInterp();
    error = QString();
    init();
}

void ScriptingTcl::ContextTcl::init()
{
    Tcl_CreateObjCommand(interp, "db", ScriptingTcl::dbCommand, nullptr, nullptr);
}
