#ifndef GLOBAL_H
#define GLOBAL_H

#define DEEP_COPY_FIELD(T, F) \
    if (other.F) \
    { \
        F = new T(*other.F); \
        F->setParent(this); \
    }

#define DEEP_COPY_COLLECTION(T, F) \
    T* _new##T; \
    foreach (T* _element, other.F) \
    { \
        _new##T = new T(*_element); \
        _new##T->setParent(this); \
        F << _new##T; \
    }

#define safe_delete(var) \
    if (var) \
    { \
        delete var; \
        var = nullptr; \
    }

#define static_char static constexpr const char

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

#endif // GLOBAL_H
