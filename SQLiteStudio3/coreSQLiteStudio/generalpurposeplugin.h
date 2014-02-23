#ifndef GENERALPURPOSEPLUGIN_H
#define GENERALPURPOSEPLUGIN_H

#include "plugin.h"

/**
 * @brief The general purpose plugin interface.
 *
 * General purpose plugins are not designated for some specific function.
 * They rely on init() and deinit() implementations to add some menubar entries,
 * or toolbar entries (or anything else), so user can interact with the plugin.
 *
 * @see Plugin
 * @see GenericPlugin
 */
class API_EXPORT GeneralPurposePlugin : virtual public Plugin
{
};

#endif // GENERALPURPOSEPLUGIN_H
