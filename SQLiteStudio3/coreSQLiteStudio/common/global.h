#ifndef GLOBAL_H
#define GLOBAL_H

/** @file */

#define DEEP_COPY_FIELD(T, F) \
    if (other.F) \
    { \
        F = new T(*other.F); \
        F->setParent(this); \
    }

#define DEEP_COPY_COLLECTION(T, F) \
    T* _new##T; \
    for (T* _element : other.F) \
    { \
        _new##T = new T(*_element); \
        _new##T->setParent(this); \
        F << _new##T; \
    }

/**
 * @brief Deletes object and sets the pointer to null.
 *
 * Deletes object under given pointer, but only if the pointer is not null.
 * Also sets the pointer to the null after deleting is done.
 */
#define safe_delete(var) \
    if (var) \
    { \
        delete var; \
        var = nullptr; \
    }

#define qobject_safe_delete(var) \
    if (var) \
    { \
        var->deleteLater(); \
        var = nullptr; \
    }

#define parser_safe_delete(var) delete var

#define static_char static constexpr const char

#define static_qstring(N,V) const static QString N = QStringLiteral(V)

#define DECLARE_SINGLETON(Cls) \
    public: \
        static Cls* getInstance(); \
        static void destroy(); \
    \
    private: \
        static Cls* _instance;

#define DEFINE_SINGLETON(Cls) \
    Cls* Cls::_instance = nullptr; \
    \
    Cls* Cls::getInstance() \
    { \
        if (!_instance) \
            _instance = new Cls(); \
    \
        return _instance; \
    } \
    \
    void Cls::destroy() \
    { \
        safe_delete(_instance); \
    }

#define STRINGIFY(s) _STRINGIFY(s)
#define _STRINGIFY(s) #s

#define __SQLS_INIT_RESOURCE(proj, name) Q_INIT_RESOURCE(proj ## _ ## name)
#define __SQLS_CLEANUP_RESOURCE(proj, name) Q_CLEANUP_RESOURCE(proj ## _ ## name)
#define _SQLS_INIT_RESOURCE(pname, name) __SQLS_INIT_RESOURCE(pname, name)
#define _SQLS_CLEANUP_RESOURCE(pname, name) __SQLS_CLEANUP_RESOURCE(pname, name)

// These are replacements for Qt's macros to cover customized resource naming,
// which is used to avoid duplication of qmake_qmake_qm_files resource initialization function across different shared libraries.
#ifdef PROJECT_MODULE_NAME
#    define SQLS_INIT_RESOURCE(name) _SQLS_INIT_RESOURCE(PROJECT_MODULE_NAME, name)
#    define SQLS_CLEANUP_RESOURCE(name) _SQLS_CLEANUP_RESOURCE(PROJECT_MODULE_NAME, name)
#else
#    define SQLS_INIT_RESOURCE(name)
#    define SQLS_CLEANUP_RESOURCE(name)
#endif

#endif // GLOBAL_H
