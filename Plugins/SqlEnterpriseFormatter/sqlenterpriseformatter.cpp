#include "sqlenterpriseformatter.h"


SqlEnterpriseFormatter::SqlEnterpriseFormatter()
{
}

QString SqlEnterpriseFormatter::format(SqliteQueryPtr query)
{
    return query->detokenize();
}

bool SqlEnterpriseFormatter::init()
{
    return true;
}

void SqlEnterpriseFormatter::deinit()
{

}
