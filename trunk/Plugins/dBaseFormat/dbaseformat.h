#ifndef DBASEFORMAT_H
#define DBASEFORMAT_H

#include "dbaseformat_global.h"
#include "plugins/genericplugin.h"
#include "plugins/generalpurposeplugin.h"

class DBASEFORMATSHARED_EXPORT DBaseFormat : public GenericPlugin, public GeneralPurposePlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("dbaseformat.json")

    public:
        DBaseFormat();
};

#endif // DBASEFORMAT_H
