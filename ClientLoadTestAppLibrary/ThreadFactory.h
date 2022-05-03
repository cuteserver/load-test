//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#ifndef THREAD_FACTORY_H
#define THREAD_FACTORY_H

#include <QThread>
#include <QMutex>

namespace Cute::Tests
{

class ThreadFactory
{
public:
    static QThread *create();
    static void setThreadFactoryFcn(QThread*(*factoryFcn)());

private:
    static QMutex &mainLock();
    static QThread*(*m_factoryFcn)();
};

}

#endif // THREAD_FACTORY_H
