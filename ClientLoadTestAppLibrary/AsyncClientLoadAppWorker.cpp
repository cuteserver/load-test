//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#include "AsyncClientLoadAppWorker.h"
#include "ThreadFactory.h"
#include <QMetaObject>

namespace Cute::Tests
{

AsyncClientLoadAppWorker::AsyncClientLoadAppWorker()
        : m_asyncWorker(ThreadFactory::create())
{
    m_asyncWorker.run();
    QObject::connect(m_asyncWorker.object(), &ClientLoadAppWorker::instantiatedRemoteObjects, this, &AsyncClientLoadAppWorker::on_instantiatedRemoteObjects);
    QObject::connect(m_asyncWorker.object(), &ClientLoadAppWorker::calledRemoteSlots, this, &AsyncClientLoadAppWorker::on_calledRemoteSlots);
}

void AsyncClientLoadAppWorker::instantiateRemoteObjects(const QString &serverAddress,
                                                        const QList<quint16> &serverPorts,
                                                        const quint32 remoteObjectsCount)
{
    QMetaObject::invokeMethod(m_asyncWorker.object(),
                              "instantiateRemoteObjects",
                              Qt::QueuedConnection,
                              Q_ARG(QString, serverAddress),
                              Q_ARG(QList<quint16>, serverPorts),
                              Q_ARG(quint32, remoteObjectsCount));
}

void AsyncClientLoadAppWorker::callRemoteSlot(quint32 iterations)
{
    QMetaObject::invokeMethod(m_asyncWorker.object(),
                              "callRemoteSlot",
                              Qt::QueuedConnection,
                              Q_ARG(quint32, iterations));
}

}


