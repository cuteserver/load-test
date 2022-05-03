//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#include "ClientLoadAppWorker.h"


namespace Cute::Tests
{

void ClientLoadAppWorker::instantiateRemoteObjects(QString serverAddress,
                                                   QList<quint16> serverPorts,
                                                   quint32 remoteObjectsCount)
{
    if (!m_remoteObjects.isEmpty())
        return;
    m_remoteObjectsCount = remoteObjectsCount;
    m_remoteObjects.resize(m_remoteObjectsCount);
    for (const auto &port : serverPorts)
        m_urls.append(QUrl(QStringLiteral("cute://%1:%2/load_test").arg(serverAddress, QString::number(port))));
    QMetaObject::invokeMethod(this, "instantiateRemoteObjectPrivate", Qt::QueuedConnection);
}

void ClientLoadAppWorker::callRemoteSlot(quint32 iterations)
{
    m_remoteSlotCallsCount = m_remoteObjectsCount * iterations;
    m_remoteSlotResponses.resize(m_remoteSlotCallsCount);
    m_iterationsCount = iterations;
    QMetaObject::invokeMethod(this, "callRemoteSlotPrivate", Qt::QueuedConnection);
}

void ClientLoadAppWorker::instantiateRemoteObjectPrivate()
{
    const bool exclusiveWebsocketConnection = true;
    m_remoteObjects[m_currentRemoteObjectsCount] =
            QSharedPointer<RemoteObject>(new RemoteObject("Cute::Tests::LoadTest",
                                                          m_urls[m_currentUrlIndex],
                                                          QVariant(),
                                                          QSslConfiguration::defaultConfiguration(),
                                                          QNetworkProxy::applicationProxy(),
                                                          exclusiveWebsocketConnection),
                                         &QObject::deleteLater);
    if (++m_currentUrlIndex == m_urls.size())
        m_currentUrlIndex = 0;
    QObject::connect(m_remoteObjects[m_currentRemoteObjectsCount].data(), &RemoteObject::connected, [this](){this->on_remoteObjectConnected();});
    QObject::connect(m_remoteObjects[m_currentRemoteObjectsCount].data(), &RemoteObject::error, [](const ErrorInfo &errorInfo){qFatal("Failed to instantiate remote object. %s",
                                                                                                                                                              errorInfoDescription(errorInfo).toUtf8().constData());});
}

void ClientLoadAppWorker::on_remoteObjectConnected()
{
    if (m_remoteObjectsCount != ++m_currentRemoteObjectsCount)
        QMetaObject::invokeMethod(this, "instantiateRemoteObjectPrivate", Qt::QueuedConnection);
    if (++m_connectedRemoteObjectsCount == m_remoteObjectsCount)
        emit instantiatedRemoteObjects();
}

void ClientLoadAppWorker::callRemoteSlotPrivate()
{
    auto &remoteObject = m_remoteObjects[m_currentRemoteObjectIndex];
    for (int i = 0; i < m_iterationsCount; ++i)
    {
        qint32 a = 1;
        qint32 b = 2;
        auto slotResponse = remoteObject->callRemoteSlot("addIntegers", a, b);
        QObject::connect(slotResponse.data(), &IRemoteSlotResponse::responded, [this](){this->on_remoteSlotResponded();});
        QObject::connect(slotResponse.data(), &IRemoteSlotResponse::failed, [](){qFatal("Failed to call remote slot.");});
        m_remoteSlotResponses[m_currentRemoteSlotResponseIndex++] = slotResponse;
    }
    if (++m_currentRemoteObjectIndex < m_remoteObjects.size())
        QMetaObject::invokeMethod(this, "callRemoteSlotPrivate", Qt::QueuedConnection);
}

void ClientLoadAppWorker::on_remoteSlotResponded()
{
    if (++m_receivedRemoteSlotResponseCount == m_remoteSlotCallsCount)
        emit calledRemoteSlots();
}

}
