#include "scriptingtcl.h"
#include "common/global.h"
#include <tcl.h>
#include <QDebug>

ScriptingTcl::ScriptingTcl()
{
}

bool ScriptingTcl::init()
{
    Q_INIT_RESOURCE(scriptingtcl);
    mainContext = new ContextTcl();
    return true;
}

void ScriptingTcl::deinit()
{
    safe_delete(mainContext);
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

QVariant ScriptingTcl::evaluate(ScriptingPlugin::Context* context, const QString& code, const QList<QVariant>& args)
{
    ContextTcl* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    setArgs(ctx, args);
    return compileAndEval(ctx, code);
}

QVariant ScriptingTcl::evaluate(const QString& code, const QList<QVariant>& args, QString* errorMessage)
{
    setArgs(mainContext, args);
    QVariant results = compileAndEval(mainContext, code);

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

QVariant ScriptingTcl::compileAndEval(ScriptingTcl::ContextTcl* ctx, const QString& code)
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

    int result = Tcl_EvalObjEx(ctx->interp, scriptObj->getTclObj(), TCL_EVAL_GLOBAL);
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
    Tcl_Obj* listObj = argsToList(args);
    Tcl_IncrRefCount(listObj);
    Tcl_Obj* argvName = Tcl_NewStringObj("argv", -1);
    Tcl_IncrRefCount(argvName);
    Tcl_ObjSetVar2(ctx->interp, argvName, nullptr, listObj, TCL_GLOBAL_ONLY);

    Tcl_Obj* intObj = Tcl_NewIntObj(args.size());
    Tcl_IncrRefCount(intObj);
    Tcl_Obj* argcName = Tcl_NewStringObj("argc", -1);
    Tcl_IncrRefCount(argcName);
    Tcl_ObjSetVar2(ctx->interp, argcName, nullptr, intObj, TCL_GLOBAL_ONLY);

    Tcl_DecrRefCount(listObj);
    Tcl_DecrRefCount(intObj);
    Tcl_DecrRefCount(argvName);
    Tcl_DecrRefCount(argcName);
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
        case TclDataType::Bignum:
        case TclDataType::String:
        case TclDataType::List:
        case TclDataType::Dict:
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
        case QVariant::String:
        default:
            obj = Tcl_NewStringObj(value.toString().toUtf8().constData(), -1);
            break;
    }

    if (!obj)
        obj = Tcl_NewStringObj(value.toString().toUtf8().constData(), -1);

    return obj;
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

Tcl_Obj*ScriptingTcl::ScriptObject::getTclObj()
{
    return obj;
}

ScriptingTcl::ContextTcl::ContextTcl()
{
    scriptCache.setMaxCost(cacheSize);
    interp = Tcl_CreateInterp();
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
}
