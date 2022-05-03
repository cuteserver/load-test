//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#include "ThreadFactory.h"
#include <QMutexLocker>


namespace Cute::Tests
{

QThread*(*ThreadFactory::m_factoryFcn)() = nullptr;

QThread *ThreadFactory::create()
{
    QMutexLocker locker(&mainLock());
    if (nullptr == m_factoryFcn)
        qFatal("ThreadFactory::create failed to create thread. Thread factory function is null.");
    else
        return m_factoryFcn();
}

void ThreadFactory::setThreadFactoryFcn(QThread *(*factoryFcn)())
{
    QMutexLocker locker(&mainLock());
    if (nullptr == factoryFcn)
        qFatal("ThreadFactory::setThreadFactoryFcn failed to set thread factory function. Given function is null.");
    m_factoryFcn = factoryFcn;
}

QMutex &ThreadFactory::mainLock()
{
    try
    {
        static QMutex mutex;
        return mutex;
    }
    catch (...)
    {
        qFatal("ThreadFactory::mainLock: QMutex threw an exception on its constructor.");
    }
    Q_UNREACHABLE();
}

}
