#ifndef VALUELOCKER_H
#define VALUELOCKER_H

/**
 * ValueLocker is similar to QMutexLocker, but it's not intended for multithreaded locks.
 * It's rather for event loop locking.
 * It can be created as local scope variable with a pointer to member variable.
 * It will set "unlockedValue" to that variable once the locker is destroyed (goes out of scope).
 * Usually the variable used will be of boolean type, but it can be virtually any other type.
 * You can also provide initial value for locked state. Otherwise you will need to set the locked
 * by yourself before the lock is created.
 */
template <class T>
class ValueLocker {
    public:
        ValueLocker(T* valueToLock, const T& lockedValue, const T& unlockedValue);
        ValueLocker(T* valueToLock, const T& unlockedValue);
        ~ValueLocker();

    private:
        T* valueToLock;
        T unlockedValue;
};

template<class T>
ValueLocker<T>::ValueLocker(T *valueToLock, const T &lockedValue, const T &unlockedValue) :
    ValueLocker(valueToLock, unlockedValue)
{
    *valueToLock = lockedValue;
}

template<class T>
ValueLocker<T>::ValueLocker(T *valueToLock, const T &unlockedValue) :
    valueToLock(valueToLock), unlockedValue(unlockedValue)
{
}

template<class T>
ValueLocker<T>::~ValueLocker()
{
    *valueToLock = unlockedValue;
}

#endif // VALUELOCKER_H
