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

#define static_char static constexpr const char

#endif // GLOBAL_H
