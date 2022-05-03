//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#ifndef ASYNC_Q_OBJECT_RUNNER_H
#define ASYNC_Q_OBJECT_RUNNER_H

#include <QThread>
#include <QScopedPointer>
#include <QTimer>
#include <QSemaphore>


namespace Cute::Tests
{

template <class T_Object, class T_InitData = void>
class AsyncQObjectRunner
{
public:
    explicit AsyncQObjectRunner(QThread *qThread, T_InitData initData) :
        m_thread(qThread),
        m_initData(initData)
    {
        QObject::connect(m_thread.data(), &QThread::started, [this]() {this->on_thread_started();});
        QObject::connect(m_thread.data(), &QThread::finished, [this]() {this->on_thread_finished();});
    }

    ~AsyncQObjectRunner()
    {
        stop();
    }

    void run()
    {
        if (m_thread->isRunning()) return;
        m_thread->start();
        m_semaphore.acquire(1);
    }

    void stop()
    {
        if (!m_thread->isRunning())
            return;
        m_thread->quit();
        m_thread->wait();
    }

    T_Object * object() {return m_object.get();}

private:
    void on_thread_started()
    {
        m_timer.reset(new QTimer());
        QObject::connect(m_timer.data(), &QTimer::timeout, [this]() {this->on_timeout();});
        m_timer->setSingleShot(true);
        m_timer->start();
    }

    void on_timeout()
    {
        try
        {
            m_object.reset(new T_Object(m_initData));
            m_semaphore.release(1);
        }
        catch (...)
        {
            m_object.reset(nullptr);
            m_semaphore.release(1);
        }
    }

    void on_thread_finished()
    {
        if (m_object)
            m_object.reset(nullptr);
    }

private:
    QScopedPointer<T_Object, QScopedPointerDeleteLater> m_object;
    QScopedPointer<QThread> m_thread;
    QScopedPointer<QTimer> m_timer;
    QSemaphore m_semaphore;
    T_InitData m_initData;
};

template <class T_Object>
class AsyncQObjectRunner<T_Object, void>
{
public:
    explicit AsyncQObjectRunner(QThread *qThread) :
        m_thread(qThread)
    {
        QObject::connect(m_thread.data(), &QThread::started, [this]() {this->on_thread_started();});
        QObject::connect(m_thread.data(), &QThread::finished, [this]() {this->on_thread_finished();});
    }

    ~AsyncQObjectRunner()
    {
        stop();
    }

    void run()
    {
        if (m_thread->isRunning()) return;
        m_thread->start();
        m_semaphore.acquire(1);
    }

    void stop()
    {
        if (!m_thread->isRunning())
            return;
        m_thread->quit();
        m_thread->wait();
    }

    T_Object * object() {return m_object.get();}

private:
    void on_thread_started()
    {
        m_timer.reset(new QTimer());
        QObject::connect(m_timer.data(), &QTimer::timeout, [this]() {this->on_timeout();});
        m_timer->setSingleShot(true);
        m_timer->start();
    }

    void on_timeout()
    {
        try
        {
            m_object.reset(new T_Object);
            m_semaphore.release(1);
        }
        catch (...)
        {
            m_object.reset(nullptr);
            m_semaphore.release(1);
        }
    }

    void on_thread_finished()
    {
        if (m_object)
            m_object.reset(nullptr);
    }

private:
    QScopedPointer<T_Object, QScopedPointerDeleteLater> m_object;
    QScopedPointer<QThread> m_thread;
    QScopedPointer<QTimer> m_timer;
    QSemaphore m_semaphore;
};

}

#endif // ASYNC_Q_OBJECT_RUNNER_H
