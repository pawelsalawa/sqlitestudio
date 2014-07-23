#ifndef SCRIPTINGTCL_H
#define SCRIPTINGTCL_H

#include "scriptingtcl_global.h"
#include "plugins/genericplugin.h"
#include "plugins/scriptingplugin.h"
#include "db/sqlquery.h"
#include <QCache>
#include <tcl.h>

class QMutex;
struct Tcl_Interp;
struct Tcl_Obj;

class SCRIPTINGTCLSHARED_EXPORT ScriptingTcl : public GenericPlugin, public DbAwareScriptingPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("scriptingtcl.json")

    public:
        ScriptingTcl();
        ~ScriptingTcl();

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
        class ScriptObject
        {
            public:
                ScriptObject(const QString& code);
                ~ScriptObject();

                Tcl_Obj* getTclObj();

            private:
                Tcl_Obj* obj = nullptr;
        };

        class ContextTcl : public ScriptingPlugin::Context
        {
            public:
                ContextTcl();
                ~ContextTcl();

                void reset();

                Tcl_Interp* interp = nullptr;
                QCache<QString,ScriptObject> scriptCache;
                QString error;
                Db* db = nullptr;
                bool useDbLocking = false;

            private:
                void init();
        };

        enum class TclDataType
        {
            Boolean,
            BooleanString,
            Double,
            Int,
            WideInt,
            Bignum,
            Bytearray,
            String,
            List,
            Dict,
            UNKNOWN
        };

        ContextTcl* getContext(ScriptingPlugin::Context* context) const;
        QVariant compileAndEval(ContextTcl* ctx, const QString& code, Db* db, bool locking);
        QVariant extractResult(ContextTcl* ctx);
        void setArgs(ContextTcl* ctx, const QList<QVariant>& args);

        static Tcl_Obj* argsToList(const QList<QVariant>& args);
        static QVariant tclObjToVariant(Tcl_Obj* obj);
        static QString tclObjToString(Tcl_Obj* obj);
        static Tcl_Obj* variantToTclObj(const QVariant& value);
        static Tcl_Obj* stringToTclObj(const QString& value);
        static int dbCommand(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
        static int dbEval(ContextTcl* ctx, Tcl_Interp* interp, Tcl_Obj* const objv[]);
        static int dbEvalRowByRow(ContextTcl* ctx, Tcl_Interp* interp, Tcl_Obj* const objv[]);
        static int dbEvalDeepResults(ContextTcl* ctx, Tcl_Interp* interp, Tcl_Obj* const objv[]);
        static int dbEvalOneColumn(ContextTcl* ctx, Tcl_Interp* interp, Tcl_Obj* const objv[]);
        static SqlQueryPtr dbCommonEval(ContextTcl* ctx, Tcl_Interp* interp, Tcl_Obj* const objv[]);
        static int setArrayVariable(Tcl_Interp* interp, const QString& arrayName, const QHash<QString,QVariant>& hash);
        static void setVariable(Tcl_Interp* interp, const QString& name, const QVariant& value);
        static QVariant getVariable(Tcl_Interp* interp, const QString& name);

        static const constexpr int cacheSize = 5;

        ContextTcl* mainContext = nullptr;
        QList<Context*> contexts;
        QMutex* mainInterpMutex = nullptr;
};

#endif // SCRIPTINGTCL_H
