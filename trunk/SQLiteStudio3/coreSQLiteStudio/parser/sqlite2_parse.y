%token_prefix TK2_
%token_type {Token*}
%default_type {Token*}
%extra_argument {ParserContext* parserContext}
%name sqlite2_parse

%syntax_error {
    UNUSED_PARAMETER(yymajor);
    parserContext->error(TOKEN, QObject::tr("Syntax error"));
}

%stack_overflow {
    UNUSED_PARAMETER(yypMinor);
    parserContext->error(QObject::tr("Parser stack overflow"));
}

%include {
#include "token.h"
#include "parsercontext.h"
#include "parser/ast/sqlitealtertable.h"
#include "parser/ast/sqliteanalyze.h"
#include "parser/ast/sqliteattach.h"
#include "parser/ast/sqlitebegintrans.h"
#include "parser/ast/sqlitecommittrans.h"
#include "parser/ast/sqlitecopy.h"
#include "parser/ast/sqlitecreateindex.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "parser/ast/sqlitecreateview.h"
#include "parser/ast/sqlitecreatevirtualtable.h"
#include "parser/ast/sqlitedelete.h"
#include "parser/ast/sqlitedetach.h"
#include "parser/ast/sqlitedropindex.h"
#include "parser/ast/sqlitedroptable.h"
#include "parser/ast/sqlitedroptrigger.h"
#include "parser/ast/sqlitedropview.h"
#include "parser/ast/sqliteemptyquery.h"
#include "parser/ast/sqliteinsert.h"
#include "parser/ast/sqlitepragma.h"
#include "parser/ast/sqlitereindex.h"
#include "parser/ast/sqliterelease.h"
#include "parser/ast/sqliterollback.h"
#include "parser/ast/sqlitesavepoint.h"
#include "parser/ast/sqliteselect.h"
#include "parser/ast/sqliteupdate.h"
#include "parser/ast/sqlitevacuum.h"
#include "parser/ast/sqliteexpr.h"
#include "parser/ast/sqlitecolumntype.h"
#include "parser/ast/sqliteconflictalgo.h"
#include "parser/ast/sqlitesortorder.h"
#include "parser/ast/sqliteindexedcolumn.h"
#include "parser/ast/sqliteforeignkey.h"
#include "parser_helper_stubs.h"
#include "common/utils_sql.h"
#include <QObject>
#include <QDebug>

#define assert(X) Q_ASSERT(X)
#define UNUSED_PARAMETER(X) (void)(X)
#define DONT_INHERIT_TOKENS(X) noTokenInheritanceFields << X
}

// These are extra tokens used by the lexer but never seen by the
// parser.  We put them in a rule so that the parser generator will
// add them to the parse.h output file.

%nonassoc ILLEGAL COMMENT SPACE.

// The following directive causes tokens ABORT, AFTER, ASC, etc. to
// fallback to ID if they will not parse as their original value.
// This obviates the need for the "id" nonterminal.

%fallback ID
  ABORT AFTER ASC ATTACH BEFORE BEGIN CASCADE CLUSTER CONFLICT
  COPY DATABASE DEFERRED DELIMITERS DESC DETACH EACH END EXPLAIN FAIL FOR
  GLOB IGNORE IMMEDIATE INITIALLY INSTEAD LIKE MATCH KEY
  OF OFFSET PRAGMA RAISE REPLACE RESTRICT ROW STATEMENT
  TEMP TRIGGER VACUUM VIEW.

// Define operator precedence early so that this is the first occurance
// of the operator tokens in the grammer.  Keeping the operators together
// causes them to be assigned integer values that are close together,
// which keeps parser tables smaller.

%left OR.
%left AND.
%right NOT.
%left EQ NE ISNULL NOTNULL IS LIKE GLOB BETWEEN IN.
%left GT GE LT LE.
%left BITAND BITOR LSHIFT RSHIFT.
%left PLUS MINUS.
%left STAR SLASH REM.
%left CONCAT.
%right UMINUS UPLUS BITNOT.

// Input is a single SQL command
%type cmd {SqliteQuery*}
%destructor cmd {delete $$;}

input ::= cmdlist.

cmdlist ::= cmdlist ecmd(C).                {parserContext->addQuery(C); DONT_INHERIT_TOKENS("cmdlist");}
cmdlist ::= ecmd(C).                        {parserContext->addQuery(C);}

%type ecmd {SqliteQuery*}
%destructor ecmd {delete $$;}
ecmd(X) ::= SEMI.                           {X = new SqliteEmptyQuery();}
ecmd(X) ::= explain(E) cmdx(C) SEMI.        {
                                                X = C;
                                                X->explain = E->explain;
                                                X->queryPlan = E->queryPlan;
                                                delete E;
                                                objectForTokens = X;
                                            }

%type explain {ParserStubExplain*}
%destructor explain {delete $$;}
explain(X) ::= .                            {X = new ParserStubExplain(false, false);}
explain(X) ::= EXPLAIN.                     {X = new ParserStubExplain(true, false);}

%type cmdx {SqliteQuery*}
%destructor cmdx {delete $$;}
cmdx(X) ::= cmd(C).                         {X = C;}

///////////////////// Begin and end transactions. ////////////////////////////

cmd(X) ::= BEGIN trans_opt(TO) onconf(C).   {
                                                X = new SqliteBeginTrans(
                                                        TO->transactionKw,
                                                        TO->name,
                                                        *(C)
                                                    );
                                                delete TO;
                                                delete C;
                                                objectForTokens = X;
                                            }

%type trans_opt {ParserStubTransDetails*}
%destructor trans_opt {delete $$;}
trans_opt(X) ::= .                          {X = new ParserStubTransDetails();}
trans_opt(X) ::= TRANSACTION.               {
                                                X = new ParserStubTransDetails();
                                                X->transactionKw = true;
                                            }
trans_opt(X) ::= TRANSACTION nm(N).         {
                                                X = new ParserStubTransDetails();
                                                X->transactionKw = true;
                                                X->name = *(N);
                                                delete N;
                                            }
trans_opt ::= TRANSACTION ID_TRANS.         {}

cmd(X) ::= COMMIT trans_opt(T).             {
                                                X = new SqliteCommitTrans(
                                                        T->transactionKw,
                                                        T->name,
                                                        false
                                                    );
                                                delete T;
                                                objectForTokens = X;
                                            }
cmd(X) ::= END trans_opt(T).                {
                                                X = new SqliteCommitTrans(
                                                        T->transactionKw,
                                                        T->name,
                                                        true
                                                    );
                                                delete T;
                                                objectForTokens = X;
                                            }
cmd(X) ::= ROLLBACK trans_opt(T).           {
                                                X = new SqliteRollback(
                                                        T->transactionKw,
                                                        T->name
                                                    );
                                                delete T;
                                                objectForTokens = X;
                                            }

///////////////////// The CREATE TABLE statement ////////////////////////////

cmd(X) ::= CREATE temp(T) TABLE
            fullname(N)
            LP columnlist(CL)
            conslist_opt(CS) RP.            {
                                                X = new SqliteCreateTable(
                                                        *(T),
                                                        false,
                                                        N->name1,
                                                        N->name2,
                                                        *(CL),
                                                        *(CS)
                                                    );
                                                delete T;
                                                delete CL;
                                                delete CS;
                                                delete N;
                                                objectForTokens = X;
                                            }
cmd(X) ::= CREATE temp(T) TABLE
            fullname(N)
            AS select(S).                   {
                                                X = new SqliteCreateTable(
                                                        *(T),
                                                        false,
                                                        N->name1,
                                                        N->name2,
                                                        S
                                                    );
                                                delete T;
                                                delete N;
                                                objectForTokens = X;
                                            }
cmd ::= CREATE temp TABLE
            nm DOT ID_TAB_NEW.              {}
cmd ::= CREATE temp TABLE
            ID_DB|ID_TAB_NEW.               {}

%type temp {int*}
%destructor temp {delete $$;}
temp(X) ::= TEMP(T).                        {X = new int( (T->value.length() > 4) ? 2 : 1 );}
temp(X) ::= .                               {X = new int(0);}

%type columnlist {ParserCreateTableColumnList*}
%destructor columnlist {delete $$;}
columnlist(X) ::= columnlist(L)
                COMMA column(C).            {
                                                L->append(C);
                                                X = L;
                                                DONT_INHERIT_TOKENS("columnlist");
                                            }
columnlist(X) ::= column(C).                {
                                                X = new ParserCreateTableColumnList();
                                                X->append(C);
                                            }

%type column {SqliteCreateTable::Column*}
%destructor column {delete $$;}
column(X) ::= columnid(C) type(T)
                carglist(L).                {
                                                X = new SqliteCreateTable::Column(*(C), T, *(L));
                                                delete C;
                                                delete L;
                                                objectForTokens = X;
                                            }

%type columnid {QString*}
%destructor columnid {delete $$;}
columnid(X) ::= nm(N).                      {X = N;}
columnid ::= ID_COL_NEW.                    {}

// An IDENTIFIER can be a generic identifier, or one of several
// keywords.  Any non-standard keyword can also be an identifier.

%type id {QString*}
%destructor id {delete $$;}
id(X) ::= ID(T).                            {
                                                X = new QString(
                                                    stripObjName(
                                                        T->value,
                                                        parserContext->dialect
                                                    )
                                                );
                                            }

// And "ids" is an identifer-or-string.

%type ids {QString*}
%destructor ids {delete $$;}
ids(X) ::= ID|STRING(T).                    {X = new QString(T->value);}

// The name of a column or table can be any of the following:

%type nm {QString*}
%destructor nm {delete $$;}
nm(X) ::= id(N).                            {X = N;}
nm(X) ::= STRING(N).                        {X = new QString(stripString(N->value));}
nm(X) ::= JOIN_KW(N).                       {X = new QString(N->value);}

%type type {SqliteColumnType*}
%destructor type {delete $$;}
type(X) ::= .                               {X = nullptr;}
type(X) ::= typetoken(T).                   {X = T;}

%type typetoken {SqliteColumnType*}
%destructor typetoken {delete $$;}
typetoken(X) ::= typename(N).               {
                                                X = new SqliteColumnType(*(N));
                                                delete N;
                                                objectForTokens = X;
                                            }
typetoken(X) ::= typename(N)
                LP signed(P) RP.            {
                                                X = new SqliteColumnType(*(N), *(P));
                                                delete N;
                                                delete P;
                                                objectForTokens = X;
                                            }
typetoken(X) ::= typename(N) LP signed(P)
                    COMMA signed(S) RP.     {
                                                X = new SqliteColumnType(*(N), *(P), *(S));
                                                delete N;
                                                delete P;
                                                delete S;
                                                objectForTokens = X;
                                            }

%type typename {QString*}
%destructor typename {delete $$;}
typename(X) ::= ids(I).                     {X = I;}
typename(X) ::= typename(T) ids(I).         {
                                                T->append(" " + *(I));
                                                delete I;
                                                X = T;
                                            }
typename ::= ID_COL_TYPE.                   {}

%type signed {QVariant*}
%destructor signed {delete $$;}
signed(X) ::= plus_num(N).                  {X = N;}
signed(X) ::= minus_num(N).                 {X = N;}

%type carglist {ParserCreateTableColumnConstraintList*}
%destructor carglist {delete $$;}
carglist(X) ::= carglist(L) ccons(C).       {
                                                L->append(C);
                                                X = L;
                                                DONT_INHERIT_TOKENS("carglist");
                                            }
carglist(X) ::= carglist(L) ccons_nm(N)
                    ccons(C).               {
                                                L->append(N);
                                                L->append(C);
                                                X = L;
                                                DONT_INHERIT_TOKENS("carglist");
                                            }
carglist(X) ::= carglist(L) carg(C).        {
                                                L->append(C);
                                                X = L;
                                                DONT_INHERIT_TOKENS("carglist");
                                            }
carglist(X) ::= .                           {X = new ParserCreateTableColumnConstraintList();}

%type carg {SqliteCreateTable::Column::Constraint*}
%destructor carg {delete $$;}
carg(X) ::= DEFAULT STRING(S).              {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initDefId(stripObjName(
                                                    S->value,
                                                    parserContext->dialect
                                                ));
                                                objectForTokens = X;
                                            }
carg(X) ::= DEFAULT ID(I).                  {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initDefId(stripObjName(
                                                    I->value,
                                                    parserContext->dialect
                                                ));
                                                objectForTokens = X;

                                            }
carg(X) ::= DEFAULT INTEGER(I).             {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                QVariant val = QVariant(I->value).toLongLong();
                                                X->initDefTerm(val, false);
                                                objectForTokens = X;
                                            }
carg(X) ::= DEFAULT PLUS INTEGER(I).        {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                QVariant val = QVariant(I->value).toLongLong();
                                                X->initDefTerm(val, false);
                                                objectForTokens = X;
                                            }
carg(X) ::= DEFAULT MINUS INTEGER(I).       {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                QVariant val = QVariant(I->value).toLongLong();
                                                X->initDefTerm(val, true);
                                                objectForTokens = X;
                                            }
carg(X) ::= DEFAULT FLOAT(F).               {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                QVariant val = QVariant(F->value).toDouble();
                                                X->initDefTerm(val, false);
                                                objectForTokens = X;
                                            }
carg(X) ::= DEFAULT PLUS FLOAT(F).          {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                QVariant val = QVariant(F->value).toDouble();
                                                X->initDefTerm(val, false);
                                                objectForTokens = X;
                                            }
carg(X) ::= DEFAULT MINUS FLOAT(F).         {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                QVariant val = QVariant(F->value).toDouble();
                                                X->initDefTerm(val, true);
                                                objectForTokens = X;
                                            }
carg(X) ::= DEFAULT NULL.                   {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initDefTerm(QVariant(), false);
                                                objectForTokens = X;
                                            }

// In addition to the type name, we also care about the primary key and
// UNIQUE constraints.

%type ccons_nm {SqliteCreateTable::Column::Constraint*}
%destructor ccons_nm {delete $$;}
ccons_nm(X) ::= CONSTRAINT nm(N).           {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initDefNameOnly(*(N));
                                                delete N;
                                                objectForTokens = X;
                                            }

%type ccons {SqliteCreateTable::Column::Constraint*}
%destructor ccons {delete $$;}
ccons(X) ::= NULL onconf(C).                {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initNull(*(C));
                                                delete C;
                                                objectForTokens = X;
                                            }
ccons(X) ::= NOT NULL onconf(C).            {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initNotNull(*(C));
                                                delete C;
                                                objectForTokens = X;
                                            }
ccons(X) ::= PRIMARY KEY sortorder(O)
                onconf(C).                  {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initPk(*(O), *(C), false);
                                                delete O;
                                                delete C;
                                                objectForTokens = X;
                                            }
ccons(X) ::= UNIQUE onconf(C).              {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initUnique(*(C));
                                                delete C;
                                                objectForTokens = X;
                                            }
ccons(X) ::= CHECK LP expr(E) RP onconf(C). {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initCheck(E, *(C));
                                                delete C;
                                                objectForTokens = X;
                                            }
ccons(X) ::= REFERENCES nm(N)
                idxlist_opt(I) refargs(A).  {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initFk(*(N), *(I), *(A));
                                                delete N;
                                                delete A;
                                                delete I;
                                                objectForTokens = X;
                                            }
ccons(X) ::= defer_subclause(D).            {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initDefer(D->initially, D->deferrable);
                                                delete D;
                                                objectForTokens = X;
                                            }
ccons(X) ::= COLLATE id(I).                 {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initColl(*(I));
                                                delete I;
                                                objectForTokens = X;
                                            }
ccons(X) ::= CHECK LP RP.                   {
                                                X = new SqliteCreateTable::Column::Constraint();
                                                X->initCheck();
                                                objectForTokens = X;
                                                parserContext->minorErrorAfterLastToken("Syntax error");
                                            }

// The next group of rules parses the arguments to a REFERENCES clause
// that determine if the referential integrity checking is deferred or
// or immediate and which determine what action to take if a ref-integ
// check fails.

%type refargs {ParserFkConditionList*}
%destructor refargs {delete $$;}
refargs(X) ::= .                            {X = new ParserFkConditionList();}
refargs(X) ::= refargs(L) refarg(A).        {
                                                L->append(A);
                                                X = L;
                                                DONT_INHERIT_TOKENS("refargs");
                                            }

%type refarg {SqliteForeignKey::Condition*}
%destructor refarg {delete $$;}
refarg(X) ::= MATCH nm(N).                  {
                                                X = new SqliteForeignKey::Condition(*(N));
                                                delete N;
                                            }
refarg(X) ::= ON INSERT refact(R).          {X = new SqliteForeignKey::Condition(SqliteForeignKey::Condition::INSERT, *(R)); delete R;}
refarg(X) ::= ON DELETE refact(R).          {X = new SqliteForeignKey::Condition(SqliteForeignKey::Condition::DELETE, *(R)); delete R;}
refarg(X) ::= ON UPDATE refact(R).          {X = new SqliteForeignKey::Condition(SqliteForeignKey::Condition::UPDATE, *(R)); delete R;}
refarg ::= MATCH ID_FK_MATCH.               {}

%type refact {SqliteForeignKey::Condition::Reaction*}
%destructor refact {delete $$;}
refact(X) ::= SET NULL.                     {X = new SqliteForeignKey::Condition::Reaction(SqliteForeignKey::Condition::SET_NULL);}
refact(X) ::= SET DEFAULT.                  {X = new SqliteForeignKey::Condition::Reaction(SqliteForeignKey::Condition::SET_DEFAULT);}
refact(X) ::= CASCADE.                      {X = new SqliteForeignKey::Condition::Reaction(SqliteForeignKey::Condition::CASCADE);}
refact(X) ::= RESTRICT.                     {X = new SqliteForeignKey::Condition::Reaction(SqliteForeignKey::Condition::RESTRICT);}

%type defer_subclause {ParserDeferSubClause*}
%destructor defer_subclause {delete $$;}
defer_subclause(X) ::= NOT DEFERRABLE
                init_deferred_pred_opt(I).  {
                                                X = new ParserDeferSubClause(SqliteDeferrable::NOT_DEFERRABLE, *(I));
                                                delete I;
                                            }
defer_subclause(X) ::= DEFERRABLE
                init_deferred_pred_opt(I).  {
                                                X = new ParserDeferSubClause(SqliteDeferrable::DEFERRABLE, *(I));
                                                delete I;
                                            }

%type init_deferred_pred_opt {SqliteInitially*}
%destructor init_deferred_pred_opt {delete $$;}
init_deferred_pred_opt(X) ::= .             {X = new SqliteInitially(SqliteInitially::null);}
init_deferred_pred_opt(X) ::= INITIALLY
                                DEFERRED.   {X = new SqliteInitially(SqliteInitially::DEFERRED);}
init_deferred_pred_opt(X) ::= INITIALLY
                                IMMEDIATE.  {X = new SqliteInitially(SqliteInitially::IMMEDIATE);}


%type conslist_opt {ParserCreateTableConstraintList*}
%destructor conslist_opt {delete $$;}
conslist_opt(X) ::= .                       {X = new ParserCreateTableConstraintList();}
conslist_opt(X) ::= COMMA conslist(L).      {X = L;}

%type conslist {ParserCreateTableConstraintList*}
%destructor conslist {delete $$;}
conslist(X) ::= conslist(L) tconscomma(CM)
                tcons(C).                   {
                                                C->afterComma = *(CM);
                                                L->append(C);
                                                X = L;
                                                delete CM;
                                                DONT_INHERIT_TOKENS("conslist");
                                            }
conslist(X) ::= tcons(C).                   {
                                                X = new ParserCreateTableConstraintList();
                                                X->append(C);
                                            }

%type tconscomma {bool*}
%destructor tconscomma {delete $$;}
tconscomma(X) ::= COMMA.                    {X = new bool(true);}
tconscomma(X) ::= .                         {X = new bool(false);}

%type tcons {SqliteCreateTable::Constraint*}
%destructor tcons {delete $$;}
tcons(X) ::= CONSTRAINT nm(N).              {
                                                X = new SqliteCreateTable::Constraint();
                                                X->initNameOnly(*(N));
                                                delete N;
                                                objectForTokens = X;
                                            }
tcons(X) ::= PRIMARY KEY LP idxlist(L)
                RP onconf(C).               {
                                                X = new SqliteCreateTable::Constraint();
                                                X->initPk(*(L), false, *(C));
                                                delete C;
                                                delete L;
                                                objectForTokens = X;
                                            }
tcons(X) ::= UNIQUE LP idxlist(L) RP
            onconf(C).                      {
                                                X = new SqliteCreateTable::Constraint();
                                                X->initUnique(*(L), *(C));
                                                delete C;
                                                delete L;
                                                objectForTokens = X;
                                            }
tcons(X) ::= CHECK LP expr(E) RP onconf(C). {
                                                X = new SqliteCreateTable::Constraint();
                                                X->initCheck(E, *(C));
                                                objectForTokens = X;
                                            }
tcons(X) ::= FOREIGN KEY LP idxlist(L) RP
         REFERENCES nm(N) idxlist_opt(IL)
         refargs(R) defer_subclause_opt(D). {
                                                X = new SqliteCreateTable::Constraint();
                                                X->initFk(
                                                    *(L),
                                                    *(N),
                                                    *(IL),
                                                    *(R),
                                                    D->initially,
                                                    D->deferrable
                                                );
                                                delete N;
                                                delete R;
                                                delete D;
                                                delete IL;
                                                delete L;
                                                objectForTokens = X;
                                            }

tcons ::= CONSTRAINT ID_CONSTR.             {}
tcons ::= FOREIGN KEY LP idxlist RP
         REFERENCES ID_TAB.                 {}
tcons(X) ::= CHECK LP RP onconf.            {
                                                X = new SqliteCreateTable::Constraint();
                                                X->initCheck();
                                                objectForTokens = X;
                                                parserContext->minorErrorAfterLastToken("Syntax error");
                                            }

%type defer_subclause_opt {ParserDeferSubClause*}
%destructor defer_subclause_opt {delete $$;}
defer_subclause_opt(X) ::= .                {X = new ParserDeferSubClause(SqliteDeferrable::null, SqliteInitially::null);}
defer_subclause_opt(X) ::=
                    defer_subclause(D).     {X = D;}

// The following is a non-standard extension that allows us to declare the
// default behavior when there is a constraint conflict.
%type onconf {SqliteConflictAlgo*}
%destructor onconf {delete $$;}
onconf(X) ::= .                             {X = new SqliteConflictAlgo(SqliteConflictAlgo::null);}
onconf(X) ::= ON CONFLICT resolvetype(R).   {X = R;}

%type orconf {SqliteConflictAlgo*}
%destructor orconf {delete $$;}
orconf(X) ::= .                             {X = new SqliteConflictAlgo(SqliteConflictAlgo::null);}
orconf(X) ::= OR resolvetype(R).            {X = R;}

%type resolvetype {SqliteConflictAlgo*}
%destructor resolvetype {delete $$;}
resolvetype(X) ::= ROLLBACK(V).             {X = new SqliteConflictAlgo(sqliteConflictAlgo(V->value));}
resolvetype(X) ::= ABORT(V).                {X = new SqliteConflictAlgo(sqliteConflictAlgo(V->value));}
resolvetype(X) ::= FAIL(V).                 {X = new SqliteConflictAlgo(sqliteConflictAlgo(V->value));}
resolvetype(X) ::= IGNORE(V).               {X = new SqliteConflictAlgo(sqliteConflictAlgo(V->value));}
resolvetype(X) ::= REPLACE(V).              {X = new SqliteConflictAlgo(sqliteConflictAlgo(V->value));}

////////////////////////// The DROP TABLE /////////////////////////////////////

cmd(X) ::= DROP TABLE fullname(N).          {
                                                X = new SqliteDropTable(false, N->name1, N->name2);
                                                delete N;
                                                objectForTokens = X;
                                            }

cmd ::= DROP TABLE nm DOT
            ID_TAB.                         {}
cmd ::= DROP TABLE ID_DB|ID_TAB.   {}

///////////////////// The CREATE VIEW statement /////////////////////////////

cmd(X) ::= CREATE temp(T) VIEW
            nm(N)
            AS select(S).                   {
                                                X = new SqliteCreateView(*(T), false, *(N), QString::null, S);
                                                delete T;
                                                delete N;
                                                objectForTokens = X;
                                            }

cmd ::= CREATE temp VIEW ID_VIEW_NEW.       {}

cmd(X) ::= DROP VIEW nm(N).                 {
                                                X = new SqliteDropView(false, *(N), QString::null);
                                                delete N;
                                                objectForTokens = X;
                                            }

cmd ::= DROP VIEW ID_VIEW.                  {}

//////////////////////// The SELECT statement /////////////////////////////////

cmd(X) ::= select_stmt(S).                  {
                                                X = S;
                                                objectForTokens = X;
                                            }

%type select_stmt {SqliteQuery*}
%destructor select_stmt {delete $$;}
select_stmt(X) ::= select(S).               {
                                                X = S;
                                                // since it's used in trigger:
                                                objectForTokens = X;
                                            }

%type select {SqliteSelect*}
%destructor select {delete $$;}
select(X) ::= oneselect(S).                 {
                                                X = SqliteSelect::append(S);
                                                objectForTokens = X;
                                            }
select(X) ::= select(S1) multiselect_op(O)
                oneselect(S2).              {
                                                X = SqliteSelect::append(S1, *(O), S2);
                                                delete O;
                                                objectForTokens = X;
                                            }

%type multiselect_op {SqliteSelect::CompoundOperator*}
%destructor multiselect_op {delete $$;}
multiselect_op(X) ::= UNION.                {X = new SqliteSelect::CompoundOperator(SqliteSelect::CompoundOperator::UNION);}
multiselect_op(X) ::= UNION ALL.            {X = new SqliteSelect::CompoundOperator(SqliteSelect::CompoundOperator::UNION_ALL);}
multiselect_op(X) ::= EXCEPT.               {X = new SqliteSelect::CompoundOperator(SqliteSelect::CompoundOperator::EXCEPT);}
multiselect_op(X) ::= INTERSECT.            {X = new SqliteSelect::CompoundOperator(SqliteSelect::CompoundOperator::INTERSECT);}

%type oneselect {SqliteSelect::Core*}
%destructor oneselect {delete $$;}
oneselect(X) ::= SELECT distinct(D)
                selcollist(L) from(F)
                where_opt(W) groupby_opt(G)
                having_opt(H)
                orderby_opt(O)
                limit_opt(LI).              {
                                                X = new SqliteSelect::Core(
                                                        *(D),
                                                        *(L),
                                                        F,
                                                        W,
                                                        *(G),
                                                        H,
                                                        *(O),
                                                        LI
                                                    );
                                                delete L;
                                                delete D;
                                                delete G;
                                                delete O;
                                                objectForTokens = X;
                                            }

%type distinct {int*}
%destructor distinct {delete $$;}
distinct(X) ::= DISTINCT.                   {X = new int(1);}
distinct(X) ::= ALL.                        {X = new int(2);}
distinct(X) ::= .                           {X = new int(0);}

// selcollist is a list of expressions that are to become the return
// values of the SELECT statement.  The "*" in statements like
// "SELECT * FROM ..." is encoded as a special expression with an
// opcode of TK_ALL.

%type sclp {ParserResultColumnList*}
%destructor sclp {delete $$;}
sclp(X) ::= selcollist(L) COMMA.            {X = L;}
sclp(X) ::= .                               {X = new ParserResultColumnList();}

%type selcollist {ParserResultColumnList*}
%destructor selcollist {delete $$;}
selcollist(X) ::= sclp(L) expr(E) as(N).    {
                                                SqliteSelect::Core::ResultColumn* obj =
                                                    new SqliteSelect::Core::ResultColumn(
                                                        E,
                                                        N ? N->asKw : false,
                                                        N ? N->name : QString::null
                                                    );

                                                L->append(obj);
                                                X = L;
                                                delete N;
                                                objectForTokens = obj;
                                                DONT_INHERIT_TOKENS("sclp");
                                            }
selcollist(X) ::= sclp(L) STAR.             {
                                                SqliteSelect::Core::ResultColumn* obj =
                                                    new SqliteSelect::Core::ResultColumn(true);

                                                L->append(obj);
                                                X = L;
                                                objectForTokens = obj;
                                                DONT_INHERIT_TOKENS("sclp");
                                            }
selcollist(X) ::= sclp(L) nm(N) DOT STAR.   {
                                                SqliteSelect::Core::ResultColumn* obj =
                                                    new SqliteSelect::Core::ResultColumn(
                                                        true,
                                                        *(N)
                                                    );
                                                L->append(obj);
                                                X = L;
                                                delete N;
                                                objectForTokens = obj;
                                                DONT_INHERIT_TOKENS("sclp");
                                            }
selcollist(X) ::= sclp(L).                  {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                X = L;
                                            }
selcollist ::= sclp ID_TAB DOT STAR.        {}

// An option "AS <id>" phrase that can follow one of the expressions that
// define the result set, or one of the tables in the FROM clause.

%type as {ParserStubAlias*}
%destructor as {delete $$;}
as(X) ::= AS nm(N).                         {
                                                X = new ParserStubAlias(*(N), true);
                                                delete N;
                                            }
as(X) ::= ids(N).                           {
                                                X = new ParserStubAlias(*(N), false);
                                                delete N;
                                            }
as ::= AS ID_ALIAS.                         {}
as ::= ID_ALIAS.                            {}
as(X) ::= .                                 {X = nullptr;}

// A complete FROM clause.
%type from {SqliteSelect::Core::JoinSource*}
%destructor from {delete $$;}
from(X) ::= .                               {X = nullptr;}
from(X) ::= FROM joinsrc(L).                {X = L;}

%type joinsrc {SqliteSelect::Core::JoinSource*}
%destructor joinsrc {delete $$;}
joinsrc(X) ::= singlesrc(S) seltablist(L).  {
                                                X = new SqliteSelect::Core::JoinSource(
                                                        S,
                                                        *(L)
                                                    );
                                                delete L;
                                                objectForTokens = X;
                                            }
joinsrc(X) ::= .                            {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                X = new SqliteSelect::Core::JoinSource();
                                                objectForTokens = X;
                                            }

%type seltablist {ParserOtherSourceList*}
%destructor seltablist {delete $$;}
seltablist(X) ::= seltablist(L) joinop(O)
                singlesrc(S)
                joinconstr_opt(C).          {
                                                SqliteSelect::Core::JoinSourceOther* src =
                                                    new SqliteSelect::Core::JoinSourceOther(O, S, C);

                                                L->append(src);
                                                X = L;
                                                objectForTokens = src;
                                                DONT_INHERIT_TOKENS("seltablist");
                                            }
seltablist(X) ::= .                         {
                                                X = new ParserOtherSourceList();
                                            }

%type singlesrc {SqliteSelect::Core::SingleSource*}
%destructor singlesrc {delete $$;}
singlesrc(X) ::= nm(N1) dbnm(N2) as(A).     {
                                                X = new SqliteSelect::Core::SingleSource(
                                                        *(N1),
                                                        *(N2),
                                                        A ? A->asKw : false,
                                                        A ? A->name : QString::null,
                                                        false,
                                                        QString::null
                                                    );
                                                delete N1;
                                                delete N2;
                                                delete A;
                                                objectForTokens = X;
                                            }
singlesrc(X) ::= LP select(S) RP as(A).     {
                                                X = new SqliteSelect::Core::SingleSource(
                                                        S,
                                                        A ? A->asKw : false,
                                                        A ? A->name : QString::null
                                                    );
                                                delete A;
                                                objectForTokens = X;
                                            }
singlesrc(X) ::= LP joinsrc(J) RP as(A).    {
                                                X = new SqliteSelect::Core::SingleSource(
                                                        J,
                                                        A ? A->asKw : false,
                                                        A ? A->name : QString::null
                                                    );
                                                delete A;
                                                objectForTokens = X;
                                            }
singlesrc(X) ::= .                          {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                X = new SqliteSelect::Core::SingleSource();
                                                objectForTokens = X;
                                            }
singlesrc(X) ::= nm(N) DOT.                 {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                X = new SqliteSelect::Core::SingleSource();
                                                X->database = *(N);
                                                delete N;
                                                objectForTokens = X;
                                            }

singlesrc ::= nm DOT ID_TAB.                {}
singlesrc ::= ID_DB|ID_TAB.                 {}
singlesrc ::= nm DOT ID_VIEW.               {}
singlesrc ::= ID_DB|ID_VIEW.                {}

%type joinconstr_opt {SqliteSelect::Core::JoinConstraint*}
%destructor joinconstr_opt {delete $$;}
joinconstr_opt(X) ::= ON expr(E).           {
                                                X = new SqliteSelect::Core::JoinConstraint(E);
                                                objectForTokens = X;
                                            }
joinconstr_opt(X) ::= USING LP
                    inscollist(L) RP.       {
                                                X = new SqliteSelect::Core::JoinConstraint(*(L));
                                                delete L;
                                                objectForTokens = X;
                                            }
joinconstr_opt(X) ::= .                     {X = nullptr;}

%type dbnm {QString*}
%destructor dbnm {delete $$;}
dbnm(X) ::= .                               {X = new QString();}
dbnm(X) ::= DOT nm(N).                      {X = N;}

%type fullname {ParserFullName*}
%destructor fullname {delete $$;}
fullname(X) ::= nm(N1) dbnm(N2).            {
                                                X = new ParserFullName();
                                                X->name1 = *(N1);
                                                X->name2 = *(N2);
                                                delete N1;
                                                delete N2;
                                            }

%type joinop {SqliteSelect::Core::JoinOp*}
%destructor joinop {delete $$;}
joinop(X) ::= COMMA.                        {
                                                X = new SqliteSelect::Core::JoinOp(true);
                                                objectForTokens = X;
                                            }
joinop(X) ::= JOIN.                         {
                                                X = new SqliteSelect::Core::JoinOp(false);
                                                objectForTokens = X;
                                            }
joinop(X) ::= JOIN_KW(K) JOIN.              {
                                                X = new SqliteSelect::Core::JoinOp(K->value);
                                                objectForTokens = X;
                                            }
joinop(X) ::= JOIN_KW(K) nm(N) JOIN.        {
                                                X = new SqliteSelect::Core::JoinOp(K->value, *(N));
                                                delete N;
                                                objectForTokens = X;
                                            }
joinop(X) ::= JOIN_KW(K) nm(N1) nm(N2)
                JOIN.                       {
                                                X = new SqliteSelect::Core::JoinOp(K->value, *(N1), *(N2));
                                                delete N1;
                                                delete N1;
                                                objectForTokens = X;
                                            }

joinop ::= ID_JOIN_OPTS.                    {}

%type orderby_opt {ParserOrderByList*}
%destructor orderby_opt {delete $$;}
orderby_opt(X) ::= .                        {X = new ParserOrderByList();}
orderby_opt(X) ::= ORDER BY sortlist(L).    {X = L;}

%type sortlist {ParserOrderByList*}
%destructor sortlist {delete $$;}
sortlist(X) ::= sortlist(L) COMMA
                collate(C)
                expr(E) sortorder(O).       {
                                                SqliteOrderBy* obj;
                                                if (C)
                                                {
                                                    SqliteExpr* coll = new SqliteExpr();
                                                    coll->initCollate(E, *(C));
                                                    delete C;
                                                    obj = new SqliteOrderBy(coll, *(O));
                                                }
                                                else
                                                {
                                                    obj = new SqliteOrderBy(E, *(O));
                                                }
                                                L->append(obj);
                                                X = L;
                                                delete O;
                                                objectForTokens = obj;
                                                DONT_INHERIT_TOKENS("sortlist");
                                            }
sortlist(X) ::= expr(E) collate(C)
                    sortorder(O).           {
                                                SqliteOrderBy* obj;
                                                if (C)
                                                {
                                                    SqliteExpr* coll = new SqliteExpr();
                                                    coll->initCollate(E, *(C));
                                                    delete C;
                                                    obj = new SqliteOrderBy(coll, *(O));
                                                }
                                                else
                                                {
                                                    obj = new SqliteOrderBy(E, *(O));
                                                }
                                                X = new ParserOrderByList();
                                                X->append(obj);
                                                delete O;
                                                objectForTokens = obj;
                                            }

%type collate {QString*}
%destructor collate {if ($$) delete $$;}
collate(X) ::= .                            {X = nullptr;}
collate(X) ::= COLLATE id(I).               {X = I;}

%type sortorder {SqliteSortOrder*}
%destructor sortorder {delete $$;}
sortorder(X) ::= ASC.                       {X = new SqliteSortOrder(SqliteSortOrder::ASC);}
sortorder(X) ::= DESC.                      {X = new SqliteSortOrder(SqliteSortOrder::DESC);}
sortorder(X) ::= .                          {X = new SqliteSortOrder(SqliteSortOrder::null);}

%type groupby_opt {ParserExprList*}
%destructor groupby_opt {delete $$;}
groupby_opt(X) ::= .                        {X = new ParserExprList();}
groupby_opt(X) ::= GROUP BY nexprlist(L).   {X = L;}
groupby_opt(X) ::= GROUP BY.                {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                X = new ParserExprList();
                                            }
%type having_opt {SqliteExpr*}
%destructor having_opt {delete $$;}
having_opt(X) ::= .                         {X = nullptr;}
having_opt(X) ::= HAVING expr(E).           {X = E;}

%type limit_opt {SqliteLimit*}
%destructor limit_opt {delete $$;}
limit_opt(X) ::= .                          {X = nullptr;}
limit_opt(X) ::= LIMIT signed(V).           {
                                                X = new SqliteLimit(*(V));
                                                delete V;
                                                objectForTokens = X;
                                            }
limit_opt(X) ::= LIMIT signed(V1) OFFSET
                    signed(V2).             {
                                                SqliteExpr* expr1 = new SqliteExpr();
                                                expr1->initLiteral(*(V1));
                                                expr1->setParent(X);

                                                SqliteExpr* expr2 = new SqliteExpr();
                                                expr1->initLiteral(*(V2));
                                                expr1->setParent(X);

                                                X = new SqliteLimit(expr1, expr2, true);

                                                TokenPtr limitToken = TokenPtr::create(Token::INTEGER, V1->toString());
                                                parserContext->addManagedToken(limitToken);
                                                expr1->tokens << limitToken;
                                                expr1->tokensMap["term"] << limitToken;

                                                TokenPtr offsetToken = TokenPtr::create(Token::INTEGER, V2->toString());
                                                parserContext->addManagedToken(offsetToken);
                                                expr2->tokens << offsetToken;
                                                expr2->tokensMap["term"] << offsetToken;

                                                delete V1;
                                                delete V2;
                                                objectForTokens = X;
                                            }
limit_opt(X) ::= LIMIT signed(V1) COMMA
                    signed(V2).             {
                                                SqliteExpr* expr1 = new SqliteExpr();
                                                expr1->initLiteral(*(V1));
                                                expr1->setParent(X);

                                                SqliteExpr* expr2 = new SqliteExpr();
                                                expr1->initLiteral(*(V2));
                                                expr1->setParent(X);

                                                X = new SqliteLimit(expr1, expr2, false);

                                                TokenPtr limitToken = TokenPtr::create(Token::INTEGER, V1->toString());
                                                parserContext->addManagedToken(limitToken);
                                                expr1->tokens << limitToken;
                                                expr1->tokensMap["term"] << limitToken;

                                                TokenPtr offsetToken = TokenPtr::create(Token::INTEGER, V2->toString());
                                                parserContext->addManagedToken(offsetToken);
                                                expr2->tokens << offsetToken;
                                                expr2->tokensMap["term"] << offsetToken;

                                                delete V1;
                                                delete V2;
                                                objectForTokens = X;
                                            }

/////////////////////////// The DELETE statement /////////////////////////////

cmd(X) ::= delete_stmt(S).                  {
                                                X = S;
                                                objectForTokens = X;
                                            }

%type delete_stmt {SqliteQuery*}
%destructor delete_stmt {delete $$;}
delete_stmt(X) ::= DELETE FROM fullname(N)
             where_opt(W).                  {
                                                X = new SqliteDelete(
                                                        N->name1,
                                                        N->name2,
                                                        false,
                                                        W,
                                                        nullptr
                                                    );
                                                delete N;
                                                // since it's used in trigger:
                                                objectForTokens = X;
                                            }

delete_stmt(X) ::= DELETE FROM.             {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                SqliteDelete* q = new SqliteDelete();
                                                X = q;
                                                objectForTokens = X;
                                            }
delete_stmt(X) ::= DELETE FROM nm(N) DOT.   {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                SqliteDelete* q = new SqliteDelete();
                                                q->database = *(N);
                                                X = q;
                                                objectForTokens = X;
                                                delete N;
                                            }
delete_stmt ::= DELETE FROM nm DOT ID_TAB.  {}
delete_stmt ::= DELETE FROM ID_DB|ID_TAB.   {}

%type where_opt {SqliteExpr*}
%destructor where_opt {delete $$;}
where_opt(X) ::= .                          {X = nullptr;}
where_opt(X) ::= WHERE expr(E).             {X = E;}
where_opt(X) ::= WHERE.                     {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                X = new SqliteExpr();
                                            }


////////////////////////// The UPDATE command ////////////////////////////////

cmd(X) ::= update_stmt(S).                  {
                                                X = S;
                                                objectForTokens = X;
                                            }

%type update_stmt {SqliteQuery*}
%destructor update_stmt {delete $$;}
update_stmt(X) ::= UPDATE orconf(C)
            fullname(N) SET
            setlist(L) where_opt(W).        {
                                                X = new SqliteUpdate(
                                                        *(C),
                                                        N->name1,
                                                        N->name2,
                                                        false,
                                                        QString::null,
                                                        *(L),
                                                        W,
                                                        nullptr
                                                    );
                                                delete C;
                                                delete N;
                                                delete L;
                                                // since it's used in trigger:
                                                objectForTokens = X;
                                            }

update_stmt(X) ::= UPDATE
            orconf(C).                      {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                X = new SqliteUpdate();
                                                objectForTokens = X;
                                                delete C;
                                            }
update_stmt(X) ::= UPDATE
            orconf(C) nm(N) DOT.            {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                SqliteUpdate* q = new SqliteUpdate();
                                                q->database = *(N);
                                                X = q;
                                                objectForTokens = X;
                                                delete C;
                                                delete N;
                                            }
update_stmt ::= UPDATE orconf nm DOT
                    ID_TAB.                 {}
update_stmt ::= UPDATE orconf ID_DB|ID_TAB. {}

%type setlist {ParserSetValueList*}
%destructor setlist {delete $$;}
setlist(X) ::= setlist(L) COMMA nm(N) EQ
                expr(E).                    {
                                                L->append(ParserSetValue(*(N), E));
                                                X = L;
                                                delete N;
                                                DONT_INHERIT_TOKENS("setlist");
                                            }
setlist(X) ::= nm(N) EQ expr(E).            {
                                                X = new ParserSetValueList();
                                                X->append(ParserSetValue(*(N), E));
                                                delete N;
                                            }
setlist(X) ::= .                            {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                X = new ParserSetValueList();
                                            }
setlist(X) ::= setlist(L) COMMA.            {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                X = L;
                                            }

setlist ::= setlist COMMA ID_COL.           {}
setlist ::= ID_COL.                         {}

////////////////////////// The INSERT command /////////////////////////////////

cmd(X) ::= insert_stmt(S).                  {
                                                X = S;
                                                objectForTokens = X;
                                            }

%type insert_stmt {SqliteQuery*}
%destructor insert_stmt {delete $$;}
insert_stmt(X) ::= insert_cmd(C) INTO
            fullname(N) inscollist_opt(I)
            VALUES LP exprlist(L) RP.       {
                                                X = new SqliteInsert(
                                                        C->replace,
                                                        C->orConflict,
                                                        N->name1,
                                                        N->name2,
                                                        *(I),
                                                        *(L),
                                                        nullptr
                                                    );
                                                delete N;
                                                delete C;
                                                delete L;
                                                delete I;
                                                // since it's used in trigger:
                                                objectForTokens = X;
                                            }
insert_stmt(X) ::= insert_cmd(C) INTO
            fullname(N) inscollist_opt(I)
            select(S).                      {
                                                X = new SqliteInsert(
                                                        C->replace,
                                                        C->orConflict,
                                                        N->name1,
                                                        N->name2,
                                                        *(I),
                                                        S,
                                                        nullptr
                                                    );
                                                delete N;
                                                delete C;
                                                delete I;
                                                // since it's used in trigger:
                                                objectForTokens = X;
                                            }

insert_stmt(X) ::= insert_cmd(C) INTO.      {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                SqliteInsert* q = new SqliteInsert();
                                                q->replaceKw = C->replace;
                                                q->onConflict = C->orConflict;
                                                X = q;
                                                objectForTokens = X;
                                                delete C;
                                            }
insert_stmt(X) ::= insert_cmd(C) INTO nm(N)
            DOT.                            {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                SqliteInsert* q = new SqliteInsert();
                                                q->replaceKw = C->replace;
                                                q->onConflict = C->orConflict;
                                                q->database = *(N);
                                                X = q;
                                                objectForTokens = X;
                                                delete C;
                                                delete N;
                                            }
insert_stmt ::= insert_cmd INTO
                    ID_DB|ID_TAB.           {}
insert_stmt ::= insert_cmd INTO
                    nm DOT ID_TAB.          {}

%type insert_cmd {ParserStubInsertOrReplace*}
%destructor insert_cmd {delete $$;}
insert_cmd(X) ::= INSERT orconf(C).         {
                                                X = new ParserStubInsertOrReplace(false, *(C));
                                                delete C;
                                            }
insert_cmd(X) ::= REPLACE.                  {X = new ParserStubInsertOrReplace(true);}

%type inscollist_opt {ParserStringList*}
%destructor inscollist_opt {delete $$;}
inscollist_opt(X) ::= .                     {X = new ParserStringList();}
inscollist_opt(X) ::= LP inscollist(L) RP.  {X = L;}

%type inscollist {ParserStringList*}
%destructor inscollist {delete $$;}
inscollist(X) ::= inscollist(L) COMMA
                    nm(N).                  {
                                                L->append(*(N));
                                                X = L;
                                                delete N;
                                                DONT_INHERIT_TOKENS("inscollist");
                                            }
inscollist(X) ::= nm(N).                    {
                                                X = new ParserStringList();
                                                X->append(*(N));
                                                delete N;
                                            }
inscollist(X) ::= .                         {
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                                X = new ParserStringList();
                                            }

inscollist ::= inscollist COMMA ID_COL.     {}
inscollist ::= ID_COL.                      {}

/////////////////////////// Expression Processing /////////////////////////////

%type exprx {SqliteExpr*}
%destructor exprx {delete $$;}
exprx(X) ::= NULL.                       {
                                                X = new SqliteExpr();
                                                X->initNull();
                                                objectForTokens = X;
                                            }
exprx(X) ::= INTEGER(I).                    {
                                                X = new SqliteExpr();
                                                QVariant val = QVariant(I->value).toLongLong();
                                                X->initLiteral(val);
                                                objectForTokens = X;
                                            }
exprx(X) ::= FLOAT(F).                      {
                                                X = new SqliteExpr();
                                                QVariant val = QVariant(F->value).toDouble();
                                                X->initLiteral(val);
                                                objectForTokens = X;
                                            }
exprx(X) ::= STRING(S).                     {
                                                X = new SqliteExpr();
                                                X->initLiteral(QVariant(S->value));
                                                objectForTokens = X;
                                            }
exprx(X) ::= LP expr(E) RP.                 {
                                                X = new SqliteExpr();
                                                X->initSubExpr(E);
                                                objectForTokens = X;
                                            }
exprx(X) ::= id(N).                         {
                                                X = new SqliteExpr();
                                                X->initId(*(N));
                                                delete N;
                                                objectForTokens = X;
                                            }
exprx(X) ::= JOIN_KW(N).                    {
                                                X = new SqliteExpr();
                                                X->initId(N->value);
                                                objectForTokens = X;
                                            }
exprx(X) ::= nm(N1) DOT nm(N2).             {
                                                X = new SqliteExpr();
                                                X->initId(*(N1), *(N2));
                                                delete N1;
                                                delete N2;
                                                objectForTokens = X;
                                            }
exprx(X) ::= nm(N1) DOT nm(N2) DOT nm(N3).  {
                                                X = new SqliteExpr();
                                                X->initId(*(N1), *(N2), *(N3));
                                                delete N1;
                                                delete N2;
                                                delete N3;
                                                objectForTokens = X;
                                            }
exprx(X) ::= VARIABLE(V).                   {
                                                X = new SqliteExpr();
                                                X->initBindParam(V->value);
                                                objectForTokens = X;
                                            }
exprx(X) ::= ID(I) LP exprlist(L) RP.       {
                                                X = new SqliteExpr();
                                                X->initFunction(I->value, false, *(L));
                                                delete L;
                                                objectForTokens = X;
                                            }
exprx(X) ::= ID(I) LP STAR RP.              {
                                                X = new SqliteExpr();
                                                X->initFunction(I->value, true);
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E1) AND(O) expr(E2).      {
                                                X = new SqliteExpr();
                                                X->initBinOp(E1, O->value, E2);
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E1) OR(O) expr(E2).       {
                                                X = new SqliteExpr();
                                                X->initBinOp(E1, O->value, E2);
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E1) LT|GT|GE|LE(O)
                expr(E2).                   {
                                                X = new SqliteExpr();
                                                X->initBinOp(E1, O->value, E2);
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E1) EQ|NE(O) expr(E2).    {
                                                X = new SqliteExpr();
                                                X->initBinOp(E1, O->value, E2);
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E1)
            BITAND|BITOR|LSHIFT|RSHIFT(O)
            expr(E2).                       {
                                                X = new SqliteExpr();
                                                X->initBinOp(E1, O->value, E2);
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E1) PLUS|MINUS(O)
                expr(E2).                   {
                                                X = new SqliteExpr();
                                                X->initBinOp(E1, O->value, E2);
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E1) STAR|SLASH|REM(O)
                expr(E2).                   {
                                                X = new SqliteExpr();
                                                X->initBinOp(E1, O->value, E2);
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E1) CONCAT(O) expr(E2).   {
                                                X = new SqliteExpr();
                                                X->initBinOp(E1, O->value, E2);
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E1) not_opt(N) likeop(L)
            expr(E2).  [LIKE_KW]            {
                                                X = new SqliteExpr();
                                                X->initLike(E1, *(N), *(L), E2);
                                                delete N;
                                                delete L;
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E) ISNULL|NOTNULL(N).     {
                                                X = new SqliteExpr();
                                                X->initNull(E, N->value);
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E) NOT NULL.              {
                                                X = new SqliteExpr();
                                                X->initNull(E, "NOT NULL");
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E1) IS not_opt(N)
                expr(E2).                   {
                                                X = new SqliteExpr();
                                                X->initIs(E1, *(N), E2);
                                                delete N;
                                                objectForTokens = X;
                                            }
exprx(X) ::= NOT(O) expr(E).                {
                                                X = new SqliteExpr();
                                                X->initUnaryOp(E, O->value);
                                            }
exprx(X) ::= BITNOT(O) expr(E).             {
                                                X = new SqliteExpr();
                                                X->initUnaryOp(E, O->value);
                                                objectForTokens = X;
                                            }
exprx(X) ::= MINUS(O) expr(E). [BITNOT]     {
                                                X = new SqliteExpr();
                                                X->initUnaryOp(E, O->value);
                                                objectForTokens = X;
                                            }
exprx(X) ::= PLUS(O) expr(E). [BITNOT]      {
                                                X = new SqliteExpr();
                                                X->initUnaryOp(E, O->value);
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E1) not_opt(N) BETWEEN
                expr(E2) AND
                expr(E3). [BETWEEN]         {
                                                X = new SqliteExpr();
                                                X->initBetween(E1, *(N), E2, E3);
                                                delete N;
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E) not_opt(N) IN LP
                exprlist(L) RP. [IN]        {
                                                X = new SqliteExpr();
                                                X->initIn(E, *(N), *(L));
                                                delete N;
                                                delete L;
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E) not_opt(N) IN LP
                select(S) RP.  [IN]         {
                                                X = new SqliteExpr();
                                                X->initIn(E, *(N), S);
                                                delete N;
                                                objectForTokens = X;
                                            }
exprx(X) ::= expr(E) not_opt(N) IN nm(N1)
                dbnm(N2). [IN]              {
                                                X = new SqliteExpr();
                                                X->initIn(E, N, *(N1), *(N2));
                                                delete N;
                                                delete N1;
                                                objectForTokens = X;
                                            }
exprx(X) ::= LP select(S) RP.               {
                                                X = new SqliteExpr();
                                                X->initSubSelect(S);
                                                objectForTokens = X;
                                            }
exprx(X) ::= CASE case_operand(O)
                case_exprlist(L)
                case_else(E) END.           {
                                                X = new SqliteExpr();
                                                X->initCase(O, *(L), E);
                                                delete L;
                                                objectForTokens = X;
                                            }
exprx(X) ::= RAISE LP raisetype(R) COMMA
                        nm(N) RP.           {
                                                X = new SqliteExpr();
                                                X->initRaise(R->value, *(N));
                                                delete N;
                                                objectForTokens = X;
                                            }
exprx(X) ::= RAISE LP IGNORE(R) RP.         {
                                                X = new SqliteExpr();
                                                X->initRaise(R->value);
                                                objectForTokens = X;
                                            }
exprx(X) ::= nm(N1) DOT.                    {
                                                X = new SqliteExpr();
                                                X->initId(*(N1), QString::null, QString::null);
                                                delete N1;
                                                objectForTokens = X;
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                            }
exprx(X) ::= nm(N1) DOT nm(N2) DOT.         {
                                                X = new SqliteExpr();
                                                X->initId(*(N1), *(N2), QString::null);
                                                delete N1;
                                                delete N2;
                                                objectForTokens = X;
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                            }
exprx(X) ::= expr(E1) not_opt(N) BETWEEN
                expr(E2) AND.               {
                                                X = new SqliteExpr();
                                                delete N;
                                                delete E1;
                                                delete E2;
                                                objectForTokens = X;
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                            }
exprx(X) ::= CASE case_operand(O)
                case_exprlist(L)
                case_else(E).               {
                                                X = new SqliteExpr();
                                                delete L;
                                                delete O;
                                                delete E;
                                                objectForTokens = X;
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                            }
exprx(X) ::= expr(E) not_opt(N) IN LP
                exprlist(L). [IN]           {
                                                X = new SqliteExpr();
                                                delete N;
                                                delete L;
                                                delete E;
                                                objectForTokens = X;
                                                parserContext->minorErrorBeforeNextToken("Syntax error");
                                            }

exprx ::= expr not_opt IN ID_DB. [IN]       {}
exprx ::= expr not_opt IN nm DOT
            ID_TAB. [IN]                    {}
exprx ::= ID_DB|ID_TAB|ID_COL|ID_FN.        {}
exprx ::= nm DOT ID_TAB|ID_COL.             {}
exprx ::= nm DOT nm DOT ID_COL.             {}
exprx ::= RAISE LP raisetype COMMA
            ID_ERR_MSG RP.                  {}

%type expr {SqliteExpr*}
%destructor expr {delete $$;}
expr(X) ::= exprx(E).                       {X = E;}
expr(X) ::= .                               {
                                                X = new SqliteExpr();
                                                objectForTokens = X;
                                                parserContext->minorErrorAfterLastToken("Syntax error");
                                            }
%type not_opt {bool*}
%destructor not_opt {delete $$;}
not_opt(X) ::= .                            {X = new bool(false);}
not_opt(X) ::= NOT.                         {X = new bool(true);}

%type likeop {SqliteExpr::LikeOp*}
%destructor likeop {delete $$;}
likeop(X) ::= LIKE|GLOB(T).                 {X = new SqliteExpr::LikeOp(SqliteExpr::likeOp(T->value));}

%type case_exprlist {ParserExprList*}
%destructor case_exprlist {delete $$;}
case_exprlist(X) ::= case_exprlist(L) WHEN
                    expr(E1) THEN expr(E2). {
                                                L->append(E1);
                                                L->append(E2);
                                                X = L;
                                            }
case_exprlist(X) ::= WHEN expr(E1) THEN
                    expr(E2).               {
                                                X = new ParserExprList();
                                                X->append(E1);
                                                X->append(E2);
                                            }

%type case_else {SqliteExpr*}
%destructor case_else {delete $$;}
case_else(X) ::=  ELSE expr(E).             {X = E;}
case_else(X) ::=  .                         {X = nullptr;}

%type case_operand {SqliteExpr*}
%destructor case_operand {delete $$;}
case_operand(X) ::= exprx(E).               {X = E;}
case_operand(X) ::= .                       {X = nullptr;}

%type exprlist {ParserExprList*}
%destructor exprlist {delete $$;}
exprlist(X) ::= nexprlist(L).               {X = L;}
exprlist(X) ::= .                           {X = new ParserExprList();}

%type nexprlist {ParserExprList*}
%destructor nexprlist {delete $$;}
nexprlist(X) ::= nexprlist(L) COMMA
                    expr(E).                {
                                                L->append(E);
                                                X = L;
                                                DONT_INHERIT_TOKENS("nexprlist");
                                            }
nexprlist(X) ::= exprx(E).                  {
                                                X = new ParserExprList();
                                                X->append(E);
                                            }

///////////////////////////// The CREATE INDEX command ///////////////////////

cmd(X) ::= CREATE uniqueflag(U)  INDEX
            nm(N1) ON nm(N2) dbnm(N3)
            LP idxlist(L) RP onconf(C).     {
                                                X = new SqliteCreateIndex(
                                                        *(U),
                                                        false,
                                                        *(N1),
                                                        *(N2),
                                                        *(N3),
                                                        *(L),
                                                        *(C)
                                                    );
                                                delete U;
                                                delete N1;
                                                delete N2;
                                                delete N3;
                                                delete L;
                                                delete C;
                                                objectForTokens = X;
                                            }

cmd ::= CREATE uniqueflag INDEX nm dbnm
        ON ID_TAB.                          {}
cmd ::= CREATE uniqueflag INDEX nm DOT
        ID_IDX_NEW.                         {}
cmd ::= CREATE uniqueflag INDEX
        ID_DB|ID_IDX_NEW.                   {}

%type uniqueflag {bool*}
%destructor uniqueflag {delete $$;}
uniqueflag(X) ::= UNIQUE.                   {X = new bool(true);}
uniqueflag(X) ::= .                         {X = new bool(false);}

%type idxlist_opt {ParserIndexedColumnList*}
%destructor idxlist_opt {delete $$;}
idxlist_opt(X) ::= .                        {X = new ParserIndexedColumnList();}
idxlist_opt(X) ::= LP idxlist(I) RP.        {X = I;}

%type idxlist {ParserIndexedColumnList*}
%destructor idxlist {delete $$;}
idxlist(X) ::= idxlist(L) COMMA
                idxlist_single(S).          {
                                                L->append(S);
                                                X = L;
                                                DONT_INHERIT_TOKENS("idxlist");
                                            }
idxlist(X) ::= idxlist_single(S).           {
                                                X = new ParserIndexedColumnList();
                                                X->append(S);
                                            }

%type idxlist_single {SqliteIndexedColumn*}
%destructor idxlist_single {delete $$;}
idxlist_single(X) ::= nm(N) sortorder(S).   {
                                                SqliteIndexedColumn* obj =
                                                    new SqliteIndexedColumn(
                                                        *(N),
                                                        QString::null,
                                                        *(S)
                                                    );
                                                X = obj;
                                                delete S;
                                                delete N;
                                                objectForTokens = X;
                                            }

idxlist_single ::= ID_COL.                  {}

///////////////////////////// The DROP INDEX command /////////////////////////

cmd(X) ::= DROP INDEX fullname(N).          {
                                                X = new SqliteDropIndex(false, N->name1, N->name2);
                                                delete N;
                                                objectForTokens = X;
                                            }

cmd ::= DROP INDEX nm DOT ID_IDX.           {}
cmd ::= DROP INDEX ID_DB|ID_IDX.            {}

///////////////////////////// The COPY command ///////////////////////////////

cmd(X) ::= COPY orconf(C) nm(N1) dbnm(N2)
            FROM nm(N3) USING DELIMITERS
            STRING(S).                      {
                                                X = new SqliteCopy(
                                                    *(C),
                                                    *(N1),
                                                    *(N2),
                                                    *(N3),
                                                    S->value
                                                );
                                                delete C;
                                                delete N1;
                                                delete N2;
                                                delete N3;
                                                objectForTokens = X;
                                            }
cmd(X) ::= COPY orconf(C) nm(N1) dbnm(N2)
            FROM nm(N3).                    {
                                                X = new SqliteCopy(
                                                    *(C),
                                                    *(N1),
                                                    *(N2),
                                                    *(N3)
                                                );
                                                delete C;
                                                delete N1;
                                                delete N2;
                                                delete N3;
                                                objectForTokens = X;
                                            }

///////////////////////////// The VACUUM command /////////////////////////////

cmd(X) ::= VACUUM.                          {
                                                X = new SqliteVacuum();
                                                objectForTokens = X;
                                            }
cmd(X) ::= VACUUM nm(N).                    {
                                                X = new SqliteVacuum(*(N));
                                                delete N;
                                                objectForTokens = X;
                                            }

///////////////////////////// The PRAGMA command /////////////////////////////

cmd(X) ::= PRAGMA ids(I).                   {
                                                X = new SqlitePragma(*(I), QString::null);
                                                delete I;
                                                objectForTokens = X;
                                            }

cmd(X) ::= PRAGMA nm(N) EQ nmnum(V).        {
                                                X = new SqlitePragma(*(N), QString::null, *(V), true);
                                                delete N;
                                                delete V;
                                                objectForTokens = X;
                                            }
cmd(X) ::= PRAGMA nm(N) LP nmnum(V) RP.     {
                                                X = new SqlitePragma(*(N), QString::null, *(V), false);
                                                delete N;
                                                delete V;
                                                objectForTokens = X;
                                            }
cmd(X) ::= PRAGMA nm(N) EQ minus_num(V).    {
                                                X = new SqlitePragma(*(N), QString::null, *(V), true);
                                                delete N;
                                                delete V;
                                                objectForTokens = X;
                                            }
cmd(X) ::= PRAGMA nm(N) LP minus_num(V) RP. {
                                                X = new SqlitePragma(*(N), QString::null, *(V), false);
                                                delete N;
                                                delete V;
                                                objectForTokens = X;
                                            }

cmd ::= PRAGMA nm DOT ID_PRAGMA.            {}
cmd ::= PRAGMA ID_DB|ID_PRAGMA.             {}

%type nmnum {QVariant*}
%destructor nmnum {delete $$;}
nmnum(X) ::= plus_num(N).                   {X = N;}
nmnum(X) ::= nm(N).                         {
                                                X = new QVariant(*(N));
                                                delete N;
                                            }
nmnum(X) ::= ON(T).                         {X = new QVariant(T->value);}
nmnum(X) ::= DELETE(T).                     {X = new QVariant(T->value);}
nmnum(X) ::= DEFAULT(T).                    {X = new QVariant(T->value);}

%type plus_num {QVariant*}
%destructor plus_num {delete $$;}
plus_num(X) ::= PLUS number(N).             {X = N;}
plus_num(X) ::= number(N).                  {X = N;}

%type minus_num {QVariant*}
%destructor minus_num {delete $$;}
minus_num(X) ::= MINUS number(N).           {
                                                if (N->type() == QVariant::Double)
                                                    *(N) = -(N->toDouble());
                                                else if (N->type() == QVariant::LongLong)
                                                    *(N) = -(N->toLongLong());
                                                else
                                                    Q_ASSERT_X(true, "producing minus number", "QVariant is neither of Double or LongLong.");

                                                X = N;
                                            }

%type number {QVariant*}
%destructor number {delete $$;}
number(X) ::= INTEGER(N).                   {X = new QVariant(QVariant(N->value).toLongLong());}
number(X) ::= FLOAT(N).                     {X = new QVariant(QVariant(N->value).toDouble());}

//////////////////////////// The CREATE TRIGGER command /////////////////////

cmd(X) ::= CREATE temp(T) TRIGGER
        nm(N) trigger_time(TT)
        trigger_event(EV) ON nm(N1)
        dbnm(N2) foreach_clause(FC)
        when_clause(WC) BEGIN
        trigger_cmd_list(CL) END.           {
                                                X = new SqliteCreateTrigger(
                                                        *(T),
                                                        false,
                                                        *(N),
                                                        *(N1),
                                                        *(N2),
                                                        *(TT),
                                                        EV,
                                                        *(FC),
                                                        WC,
                                                        *(CL),
                                                        2
                                                    );
                                                delete T;
                                                delete TT;
                                                delete FC;
                                                delete N1;
                                                delete N;
                                                delete N2;
                                                delete CL;
                                                objectForTokens = X;
                                            }

// Support full parsing when no BEGIN and END are present (for completion helper)
cmd(X) ::= CREATE temp(T) TRIGGER
        nm(N) trigger_time(TT)
        trigger_event(EV) ON nm(N1)
        dbnm(N2) foreach_clause(FC)
        when_clause(WC).                    {
                                                QList<SqliteQuery *> CL;

                                                X = new SqliteCreateTrigger(
                                                        *(T),
                                                        false,
                                                        *(N),
                                                        *(N1),
                                                        *(N2),
                                                        *(TT),
                                                        EV,
                                                        *(FC),
                                                        WC,
                                                        CL,
                                                        2
                                                    );
                                                delete T;
                                                delete TT;
                                                delete FC;
                                                delete N1;
                                                delete N;
                                                delete N2;
                                                objectForTokens = X;
                                                parserContext->minorErrorAfterLastToken("Syntax error");
                                            }

// Support full parsing when no END is present (for completion helper)
cmd(X) ::= CREATE temp(T) TRIGGER
        nm(N) trigger_time(TT)
        trigger_event(EV) ON nm(N1)
        dbnm(N2) foreach_clause(FC)
        when_clause(WC) BEGIN
        trigger_cmd_list(CL).               {
                                                X = new SqliteCreateTrigger(
                                                        *(T),
                                                        false,
                                                        *(N),
                                                        *(N1),
                                                        *(N2),
                                                        *(TT),
                                                        EV,
                                                        *(FC),
                                                        WC,
                                                        *(CL),
                                                        2
                                                    );
                                                delete T;
                                                delete TT;
                                                delete FC;
                                                delete N1;
                                                delete N;
                                                delete N2;
                                                delete CL;
                                                objectForTokens = X;
                                                parserContext->minorErrorAfterLastToken("Syntax error");
                                            }

cmd ::= CREATE temp TRIGGER nm
            trigger_time trigger_event
            ON ID_TAB|ID_DB.                {}
cmd ::= CREATE temp TRIGGER nm
            trigger_time trigger_event
            ON nm DOT ID_TAB.               {}
cmd ::= CREATE temp TRIGGER ID_TRIG_NEW.    {}

%type trigger_time {SqliteCreateTrigger::Time*}
%destructor trigger_time {delete $$;}
trigger_time(X) ::= BEFORE.                 {X = new SqliteCreateTrigger::Time(SqliteCreateTrigger::Time::BEFORE);}
trigger_time(X) ::= AFTER.                  {X = new SqliteCreateTrigger::Time(SqliteCreateTrigger::Time::AFTER);}
trigger_time(X) ::= INSTEAD OF.             {X = new SqliteCreateTrigger::Time(SqliteCreateTrigger::Time::INSTEAD_OF);}
trigger_time(X) ::= .                       {X = new SqliteCreateTrigger::Time(SqliteCreateTrigger::Time::null);}

%type trigger_event {SqliteCreateTrigger::Event*}
%destructor trigger_event {delete $$;}
trigger_event(X) ::= DELETE.                {
                                                X = new SqliteCreateTrigger::Event(SqliteCreateTrigger::Event::DELETE);
                                                objectForTokens = X;
                                            }
trigger_event(X) ::= INSERT.                {
                                                X = new SqliteCreateTrigger::Event(SqliteCreateTrigger::Event::INSERT);
                                                objectForTokens = X;
                                            }
trigger_event(X) ::= UPDATE.                {
                                                X = new SqliteCreateTrigger::Event(SqliteCreateTrigger::Event::UPDATE);
                                                objectForTokens = X;
                                            }
trigger_event(X) ::= UPDATE OF
                    inscollist(L).          {
                                                X = new SqliteCreateTrigger::Event(*(L));
                                                delete L;
                                                objectForTokens = X;
                                            }

%type foreach_clause {SqliteCreateTrigger::Scope*}
%destructor foreach_clause {delete $$;}
foreach_clause(X) ::= .                     {X = new SqliteCreateTrigger::Scope(SqliteCreateTrigger::Scope::null);}
foreach_clause(X) ::= FOR EACH ROW.         {X = new SqliteCreateTrigger::Scope(SqliteCreateTrigger::Scope::FOR_EACH_ROW);}
foreach_clause(X) ::= FOR EACH STATEMENT.   {X = new SqliteCreateTrigger::Scope(SqliteCreateTrigger::Scope::FOR_EACH_STATEMENT);}

%type when_clause {SqliteExpr*}
%destructor when_clause {if ($$) delete $$;}
when_clause(X) ::= .                        {X = nullptr;}
when_clause(X) ::= WHEN expr(E).            {X = E;}

%type trigger_cmd_list {ParserQueryList*}
%destructor trigger_cmd_list {delete $$;}
trigger_cmd_list(X) ::= trigger_cmd_list(L)
                    trigger_cmd(C) SEMI.    {
                                                L->append(C);
                                                X = L;
                                                DONT_INHERIT_TOKENS("trigger_cmd_list");
                                            }
trigger_cmd_list(X) ::= trigger_cmd(C)
                    SEMI.                   {
                                                X = new ParserQueryList();
                                                X->append(C);
                                            }

%type trigger_cmd {SqliteQuery*}
%destructor trigger_cmd {delete $$;}
trigger_cmd(X) ::= update_stmt(S).          {X = S;}
trigger_cmd(X) ::= insert_stmt(S).          {X = S;}
trigger_cmd(X) ::= delete_stmt(S).          {X = S;}
trigger_cmd(X) ::= select_stmt(S).          {X = S;}

%type raisetype {Token*}
raisetype(X) ::= ROLLBACK|ABORT|FAIL(V).    {X = V;}

////////////////////////  DROP TRIGGER statement //////////////////////////////
cmd(X) ::= DROP TRIGGER fullname(N).        {
                                                X = new SqliteDropTrigger(false, N->name1, N->name2);
                                                delete N;
                                                objectForTokens = X;
                                            }

cmd ::= DROP TRIGGER nm DOT ID_TRIG.        {}
cmd ::= DROP TRIGGER ID_DB|ID_TRIG.         {}

//////////////////////// ATTACH DATABASE file AS name /////////////////////////
cmd(X) ::= ATTACH database_kw_opt(D)
        ids(I1) AS ids(I2) key_opt(K).      {
                                                SqliteExpr* e1 = new SqliteExpr();
                                                SqliteExpr* e2 = new SqliteExpr();
                                                e1->initLiteral(*(I1));
                                                e2->initLiteral(*(I2));
                                                X = new SqliteAttach(*(D), e1, e2, K);
                                                delete D;
                                                delete I1;
                                                delete I2;
                                                objectForTokens = X;
                                            }

%type key_opt {SqliteExpr*}
%destructor key_opt {if ($$) delete $$;}
key_opt(X) ::= .                            {X = nullptr;}
key_opt(X) ::= USING ids(I).                {
                                                SqliteExpr* e = new SqliteExpr();
                                                e->initLiteral(*(I));
                                                delete I;
                                                X = e;
                                            }

%type database_kw_opt {bool*}
%destructor database_kw_opt {delete $$;}
database_kw_opt(X) ::= DATABASE.            {X = new bool(true);}
database_kw_opt(X) ::= .                    {X = new bool(false);}


//////////////////////// DETACH DATABASE name /////////////////////////////////
cmd(X) ::= DETACH database_kw_opt(D)
        nm(N).                              {
                                                SqliteExpr* e = new SqliteExpr();
                                                e->initLiteral(*(N));
                                                delete N;
                                                X = new SqliteDetach(*(D), e);
                                                delete D;
                                                objectForTokens = X;
                                            }
