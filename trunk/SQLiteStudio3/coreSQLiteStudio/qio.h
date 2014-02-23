#ifndef QIO_H
#define QIO_H

#include "coreSQLiteStudio_global.h"
#include <QTextStream>

/** @file */

/**
 * @brief Standard output stream.
 *
 * This is pretty much the same as std::cout, except it's based on QTextStream,
 * therefore accepts larger range of data types.
 */
API_EXPORT extern QTextStream qOut;

/**
 * @brief Standard input stream.
 *
 * This is pretty much the same as std::cin, except it's based on QTextStream,
 * therefore accepts larger range of data types.
 */
API_EXPORT extern QTextStream qIn;

/**
 * @brief Standard error stream.
 *
 * This is pretty much the same as std::cerr, except it's based on QTextStream,
 * therefore accepts larger range of data types.
 */
API_EXPORT extern QTextStream qErr;

#endif // QIO_H
