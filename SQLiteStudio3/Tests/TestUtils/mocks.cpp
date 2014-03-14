#include "mocks.h"
#include "common/global.h"
#include "sqlitestudio.h"
#include "configmock.h"
#include "pluginmanagermock.h"
#include "functionmanagermock.h"
#include "dbattachermock.h"
#include "dbmanagermock.h"

MockRepository* mockRepository = nullptr;

MockRepository& mockRepo()
{
    if (!mockRepository)
    {
        mockRepository = new MockRepository;
        mockRepository->autoExpect = false;
    }

    return *mockRepository;
}

void deleteMockRepo()
{
    safe_delete(mockRepository);
}

void initMocks()
{
    SQLITESTUDIO->setConfig(new ConfigMock());
    SQLITESTUDIO->setFunctionManager(new FunctionManagerMock());
    SQLITESTUDIO->setPluginManager(new PluginManagerMock());
    SQLITESTUDIO->setDbAttacherFactory(new DbAttacherFactoryMock());
    SQLITESTUDIO->setDbManager(new DbManagerMock());
}
