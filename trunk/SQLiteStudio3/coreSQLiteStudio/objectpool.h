#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H

#include <QHash>
#include <QHashIterator>
#include <QMutex>
#include <QWaitCondition>

template <class T>
class ObjectPool
{
    public:
        ObjectPool(quint32 min, quint32 max);

        T* reserve();
        void release(T* obj);

    private:
        QHash<T*, bool> pool;
        QMutex mutex;
        QWaitCondition waitCond;
        int min;
        int max;
};

template <class T>
ObjectPool::ObjectPool(quint32 min, quint32 max)
    : min(min), max(max)
{
    Q_ASSERT(min > 0);
    T* obj;
    for (int i = 0; i < min; i++)
    {
        obj = new T();
        pool[obj] = false;
    }
}

T* ObjectPool::reserve()
{
    mutex.lock();

    forever
    {
        QHashIterator<T*, bool> i(pool);
        while (i.hasNext())
        {
            i.next();
            if (!i.value())
            {
                pool[i.key()] = true;
                T* obj = i.key();
                mutex.unlock();
                return obj;
            }
        }

        // Check if we can enlarge the pool
        if (pool.size() < max)
        {
            T* obj = new T();
            pool[i.key()] = true;
            mutex.unlock();
            return obj;
        }

        // Wait for release
        waitCond.wait(&mutex);
    }

    // no need to unlock, because the loop will repeat
    // until the free obj is found and then mutex is unlocked.
}

template <class T>
void ObjectPool::release(T* obj)
{
    mutex.lock();
    pool[obj] = false;
    mutex.unlock();
    waitCond.wakeOne();
}

#endif // OBJECTPOOL_H
