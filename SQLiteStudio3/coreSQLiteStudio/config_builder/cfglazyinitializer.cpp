#include "cfglazyinitializer.h"
#include "common/unused.h"

QList<CfgLazyInitializer*>* CfgLazyInitializer::instances = nullptr;

CfgLazyInitializer::CfgLazyInitializer(std::function<void ()> initFunc, const char *name) :
    initFunc(initFunc)
{
    UNUSED(name);
    if (!instances)
        instances = new QList<CfgLazyInitializer*>();

    *instances << this;
}

void CfgLazyInitializer::init()
{
    if (!instances)
        instances = new QList<CfgLazyInitializer*>();

    for (CfgLazyInitializer*& initializer : *instances)
        initializer->doInitialize();
}

void CfgLazyInitializer::doInitialize()
{
    initFunc();
}
