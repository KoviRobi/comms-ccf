/**
\file
\brief Simple wrapper around the FreeRTOS mutex.
*/
#pragma once

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

#include <assert.h>

#include <optional>
#include <utility>

template<typename T>
class Guard;

template<typename T>
class Mutex
{
public:
    using Underlying = T;
    using GuardT = Guard<T>;

    template<typename... Args>
    Mutex(Args && ... args) : value(std::forward<Args>(args)...)
    {
        mutex = xSemaphoreCreateMutexStatic(&buffer);
    }

    std::optional<GuardT> lock(TickType_t timeout)
    {
        if (xSemaphoreTake(mutex, timeout))
        {
            return std::optional{GuardT{this}};
        }
        return std::optional<GuardT>{};
    }

    GuardT lock()
    {
        while (!xSemaphoreTake(mutex, portMAX_DELAY)) { }
        return GuardT{this};
    }

    bool unlock() const
    {
        return xSemaphoreGive(mutex) == pdTRUE;
    }

    T & unsafeGetUnderlying()
    {
        return value;
    }

private:
    SemaphoreHandle_t mutex;
    StaticSemaphore_t buffer;
    T value;
};

template<typename T>
class Guard
{
public:
    using MutexT = Mutex<T>;

    Guard(MutexT * mutex_) : mutex(mutex_) { }
    Guard(const Guard &) = delete;
    Guard & operator=(const Guard &) = delete;
    Guard(Guard &&) = default;
    Guard & operator=(Guard &&) = default;
    ~Guard()
    {
        bool success = mutex->unlock();
        assert(success);
    }

    T * operator->()
    {
        return &mutex->unsafeGetUnderlying();
    }

    const T * operator->() const
    {
        return &mutex->unsafeGetUnderlying();
    }

private:
    MutexT * mutex;
};
