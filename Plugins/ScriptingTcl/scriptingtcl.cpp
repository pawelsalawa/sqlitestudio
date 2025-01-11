#include "scriptingtcl.h"
#include "common/global.h"
#include "common/unused.h"
#include "db/db.h"
#include "parser/lexer.h"
#include "parser/token.h"
#include "common/utils_sql.h"
#include <QDebug>
#include <QMutexLocker>

ScriptingTcl::ScriptingTcl()
{
    mainInterpMutex = new QMutex();
}

ScriptingTcl::~ScriptingTcl()
{
    safe_delete(mainInterpMutex);
}

bool ScriptingTcl::init()
{
    SQLS_INIT_RESOURCE(scriptingtcl);
    QMutexLocker locker(mainInterpMutex);
    mainContext = new ContextTcl();
    return true;
}

void ScriptingTcl::deinit()
{
    QMutexLocker locker(mainInterpMutex);
    safe_delete(mainContext);
    Tcl_Finalize();
    SQLS_CLEANUP_RESOURCE(scriptingtcl);
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

    setVariable(ctx->interp, name, value);
}

QVariant ScriptingTcl::getVariable(ScriptingPlugin::Context* context, const QString& name)
{
    ContextTcl* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    return getVariable(ctx->interp, name);
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
    return ":/scriptingtcl/scriptingtcl.svg";
}

QVariant ScriptingTcl::evaluate(ScriptingPlugin::Context* context, const QString& code, const FunctionInfo& funcInfo,
                                const QList<QVariant>& args, Db* db, bool locking)
{
    ContextTcl* ctx = getContext(context);
    if (!ctx)
        return QVariant();

    return compileAndEval(ctx, code, funcInfo, args, db, locking);
}

QVariant ScriptingTcl::evaluate(const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args,
                                Db* db, bool locking, QString* errorMessage)
{
    QMutexLocker locker(mainInterpMutex);
    QVariant results = compileAndEval(mainContext, code, funcInfo, args, db, locking);

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

QVariant ScriptingTcl::compileAndEval(ScriptingTcl::ContextTcl* ctx, const QString& code, const FunctionInfo& funcInfo,
                                      const QList<QVariant>& args, Db* db, bool locking)
{
    ScriptObject* scriptObj = getScript(code, funcInfo, ctx);

    Tcl_ResetResult(ctx->interp);
    ctx->error.clear();

    setArgs(ctx, args);

    int i = 0;
    for (const QString& key : funcInfo.getArguments())
    {
        if (i >= args.size())
            break;

        setVariable(ctx, key, args[i++]);
    }

    ctx->db = db;
    ctx->useDbLocking = locking;

    int result = Tcl_EvalObjEx(ctx->interp, scriptObj->getTclObj(), TCL_EVAL_GLOBAL);

    ctx->db = nullptr;
    ctx->useDbLocking = false;

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

ScriptingTcl::ScriptObject* ScriptingTcl::getScript(const QString code, const ScriptingPlugin::FunctionInfo& funcInfo, ContextTcl* ctx)
{
    static const QString keyTpl = QStringLiteral("{%1} %2");

    QString key = keyTpl.arg(funcInfo.getArguments().join(" "), code);
    if (ctx->scriptCache.contains(key))
        return ctx->scriptCache[key];

    ScriptObject* scriptObj = new ScriptObject(code);
    ctx->scriptCache.insert(key, scriptObj);
    return scriptObj;
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
            Tcl_Obj** objv = nullptr;
            Tcl_ListObjGetElements(nullptr, obj, &objc, &objv);
            for (int i = 0; i < objc; i++)
                list << tclObjToVariant(objv[i]);

            result = list;
            break;
        }
        case TclDataType::Dict:
        {
            Tcl_DictSearch search;
            Tcl_Obj* key = nullptr;
            Tcl_Obj* value = nullptr;
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
            break;
        }
        case TclDataType::Bignum:
        case TclDataType::String:
        case TclDataType::UNKNOWN:
        default:
            result = tclObjToString(obj);
            break;
    }

    if (!ok)
        result = tclObjToString(obj);

    return result;
}

QString ScriptingTcl::tclObjToString(Tcl_Obj* obj)
{
    return QString::fromUtf8(Tcl_GetStringFromObj(obj, nullptr));
}

Tcl_Obj* ScriptingTcl::variantToTclObj(const QVariant& value)
{
    Tcl_Obj* obj = nullptr;
    switch (value.userType())
    {
        case QMetaType::Bool:
            obj = Tcl_NewBooleanObj(value.toBool());
            break;
        case QMetaType::Int:
        case QMetaType::UInt:
            obj = Tcl_NewIntObj(value.toInt());
            break;
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
            obj = Tcl_NewWideIntObj((Tcl_WideInt)value.toLongLong());
            break;
        case QMetaType::Double:
            obj = Tcl_NewDoubleObj(value.toDouble());
            break;
        case QMetaType::QByteArray:
        {
            QByteArray bytes = value.toByteArray();
            unsigned char* ubytes = reinterpret_cast<unsigned char*>(bytes.data());
            obj = Tcl_NewByteArrayObj(ubytes, bytes.size());
            break;
        }
        case QMetaType::QVariantList:
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
        case QMetaType::QStringList:
        {
            QStringList list = value.toStringList();
            int listSize = list.size();
            Tcl_Obj** objList = new Tcl_Obj*[listSize];
            for (int i = 0; i < listSize; ++i)
                objList[i] = stringToTclObj(list[i]);

            obj = Tcl_NewListObj(listSize, objList);
            delete[] objList;
            break;
        }
        case QMetaType::QVariantHash:
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
        case QMetaType::QVariantMap:
        {
            QMap<QString, QVariant> map = value.toMap();
            obj = Tcl_NewDictObj();
            QMapIterator<QString, QVariant> it(map);
            while (it.hasNext())
            {
                it.next();
                Tcl_DictObjPut(nullptr, obj, variantToTclObj(it.key()), variantToTclObj(it.value()));
            }
            break;
        }
        case QMetaType::QString:
        default:
            obj = stringToTclObj(value.toString());
            break;
    }

    if (!obj)
        obj = stringToTclObj(value.toString());

    return obj;
}

Tcl_Obj* ScriptingTcl::stringToTclObj(const QString& value)
{
    return Tcl_NewStringObj(value.toUtf8().constData(), -1);
}

int ScriptingTcl::dbCommand(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    ContextTcl* ctx = reinterpret_cast<ContextTcl*>(clientData);

    Tcl_Obj* result = nullptr;
    if (!ctx->db)
    {
        result = Tcl_NewStringObj(tr("No database available in current context, while called Tcl's '%1' command.").arg("db").toUtf8().constData(), -1);
        Tcl_SetObjResult(interp, result);
        return TCL_ERROR;
    }

    if (strcmp(Tcl_GetStringFromObj(objv[1], nullptr), "eval") == 0)
    {
        if (objc == 3)
            return dbEval(ctx, interp, objv);
        else if (objc == 5) {
            return dbEvalRowByRow(ctx, interp, objv);
        }
    }
    else if (strcmp(Tcl_GetStringFromObj(objv[1], nullptr), "rows") == 0 && objc == 3)
    {
        return dbEvalDeepResults(ctx, interp, objv);
    }
    else if (strcmp(Tcl_GetStringFromObj(objv[1], nullptr), "onecolumn") == 0 && objc == 3)
    {
        return dbEvalOneColumn(ctx, interp, objv);
    }

    result = Tcl_NewStringObj(tr("Invalid '%1' command syntax. Should be: %2").arg("db", "db eval sql").toUtf8().constData(), -1);
    Tcl_SetObjResult(interp, result);
    return TCL_ERROR;
}

int ScriptingTcl::initTclCommand(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    UNUSED(clientData);
    UNUSED(objv);

    if (objc > 1)
    {
        Tcl_Obj* result = Tcl_NewStringObj(tr("Error from Tcl's '%1' command: %2").arg("tcl_init", "invalid # args: tcl_init").toUtf8().constData(), -1);
        Tcl_SetObjResult(interp, result);
        return TCL_ERROR;
    }

    int res = Tcl_Init(interp);
    if (res != TCL_OK)
    {
        ScriptObject codeObj("set tcl_library $tcl_pkgPath");
        Tcl_EvalObjEx(interp, codeObj.getTclObj(), TCL_EVAL_GLOBAL);
        res = Tcl_Init(interp);
    }
    return res;
}

int ScriptingTcl::dbEval(ContextTcl* ctx, Tcl_Interp* interp, Tcl_Obj* const objv[])
{
    SqlQueryPtr execResults = dbCommonEval(ctx, interp, objv);
    if (execResults->isError())
        return TCL_ERROR;

    Tcl_Obj* result = nullptr;
    QList<QVariant> cells;
    SqlResultsRowPtr row;
    while (execResults->hasNext())
    {
        row = execResults->next();
        cells += row->valueList();
    }
    result = variantToTclObj(cells);

    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

int ScriptingTcl::dbEvalRowByRow(ContextTcl* ctx, Tcl_Interp* interp, Tcl_Obj* const objv[])
{
    SqlQueryPtr execResults = dbCommonEval(ctx, interp, objv);
    if (execResults->isError())
        return TCL_ERROR;

    Tcl_Obj* code = objv[4];
    QString arrayName = tclObjToString(objv[3]);
    const char* arrayCharName = arrayName.toUtf8().constData();
    SqlResultsRowPtr row;
    int resCode = TCL_OK;
    QHash<QString, QVariant> valueMap;
    while (execResults->hasNext())
    {
        row = execResults->next();

        Tcl_UnsetVar2(interp, arrayCharName, nullptr, 0);
        valueMap = row->valueMap();
        valueMap["*"] = QStringList(valueMap.keys());
        if (setArrayVariable(interp, arrayName, valueMap) != TCL_OK)
            return TCL_ERROR;

        resCode = Tcl_EvalObjEx(interp, code, 0);

        if (resCode == TCL_ERROR)
            return TCL_ERROR;
        else if (resCode == TCL_BREAK)
            break;
        else if (resCode == TCL_RETURN)
            return TCL_RETURN;
    }

    return TCL_OK;
}

int ScriptingTcl::dbEvalDeepResults(ContextTcl* ctx, Tcl_Interp* interp, Tcl_Obj* const objv[])
{
    SqlQueryPtr execResults = dbCommonEval(ctx, interp, objv);
    if (execResults->isError())
        return TCL_ERROR;

    Tcl_Obj* result = nullptr;
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

int ScriptingTcl::dbEvalOneColumn(ScriptingTcl::ContextTcl* ctx, Tcl_Interp* interp, Tcl_Obj* const objv[])
{
    SqlQueryPtr execResults = dbCommonEval(ctx, interp, objv);
    if (execResults->isError())
        return TCL_ERROR;

    Tcl_Obj* result = nullptr;
    QVariant resultValue;
    if (execResults->hasNext())
        resultValue = execResults->getSingleCell();

    result = variantToTclObj(resultValue);

    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

SqlQueryPtr ScriptingTcl::dbCommonEval(ContextTcl* ctx, Tcl_Interp* interp, Tcl_Obj* const objv[])
{
    Db::Flags flags;
    if (!ctx->useDbLocking)
        flags |= Db::Flag::NO_LOCK;

    Tcl_Obj* result = nullptr;
    QString sql = QString::fromUtf8(Tcl_GetStringFromObj(objv[2], nullptr));

    TokenList bindTokens = Lexer::tokenize(sql).filter(Token::BIND_PARAM);
    QString bindVarName;
    QHash<QString, QVariant> queryArgs;
    for (const TokenPtr& token : bindTokens)
    {
        bindVarName = getBindTokenName(token);
        if (bindVarName == "?")
            continue;

        queryArgs[token->value] = getVariable(interp, bindVarName);
    }

    SqlQueryPtr execResults = ctx->db->exec(sql, queryArgs, flags);
    if (execResults->isError())
    {
        result = Tcl_NewStringObj(tr("Error from Tcl's' '%1' command: %2").arg("db", execResults->getErrorText()).toUtf8().constData(), -1);
        Tcl_SetObjResult(interp, result);
    }
    return execResults;
}

int ScriptingTcl::setArrayVariable(Tcl_Interp* interp, const QString& arrayName, const QHash<QString, QVariant>& hash)
{
    Tcl_Obj* varName = Tcl_NewStringObj(arrayName.toUtf8().constData(), -1);
    Tcl_IncrRefCount(varName);

    Tcl_Obj* key = nullptr;
    Tcl_Obj* value = nullptr;
    Tcl_Obj* res = nullptr;

    QHashIterator<QString, QVariant> it(hash);
    while (it.hasNext())
    {
        it.next();
        key = variantToTclObj(it.key());
        value = variantToTclObj(it.value());

        Tcl_IncrRefCount(key);
        Tcl_IncrRefCount(value);

        res = Tcl_ObjSetVar2(interp, varName, key, value, 0);

        Tcl_DecrRefCount(key);
        Tcl_DecrRefCount(value);

        if (!res)
            return TCL_ERROR;
    }
    return TCL_OK;
}

void ScriptingTcl::setVariable(Tcl_Interp* interp, const QString& name, const QVariant& value)
{
    Tcl_Obj* varName = Tcl_NewStringObj(name.toUtf8().constData(), -1);
    Tcl_IncrRefCount(varName);
    Tcl_Obj* tclObjValue = variantToTclObj(value);
    Tcl_IncrRefCount(tclObjValue);
    Tcl_ObjSetVar2(interp, varName, nullptr, tclObjValue, 0);
    Tcl_DecrRefCount(tclObjValue);
    Tcl_DecrRefCount(varName);
}

QVariant ScriptingTcl::getVariable(Tcl_Interp* interp, const QString& name)
{
    Tcl_Obj* varName = Tcl_NewStringObj(name.toUtf8().constData(), -1);
    Tcl_IncrRefCount(varName);
    Tcl_Obj* obj = Tcl_ObjGetVar2(interp, varName, nullptr, 0);
    if (!obj)
        return QVariant();

    Tcl_IncrRefCount(obj);
    QVariant val = tclObjToVariant(obj);
    Tcl_DecrRefCount(varName);
    Tcl_DecrRefCount(obj);
    return val;
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
    Tcl_CreateObjCommand(interp, "db", ScriptingTcl::dbCommand, reinterpret_cast<ClientData>(this), nullptr);
    Tcl_CreateObjCommand(interp, "tcl_init", ScriptingTcl::initTclCommand, reinterpret_cast<ClientData>(this), nullptr);
}
