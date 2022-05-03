//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#ifndef CLIENT_LOAD_APP_WORKER_H
#define CLIENT_LOAD_APP_WORKER_H

#include <CuteClient.h>
#include <QObject>
#include <QList>
#include <QHostAddress>
#include <QVector>
#include <QSharedPointer>

using namespace Cute::Client;

namespace Cute::Tests
{

class ClientLoadAppWorker : public QObject
{
Q_OBJECT
public:
    ClientLoadAppWorker() = default;
    ~ClientLoadAppWorker() override = default;

public slots:
    void instantiateRemoteObjects(QString serverAddress,
                                  QList<quint16> serverPorts,
                                  quint32 remoteObjectsCount);
    void callRemoteSlot(quint32 iterations);

signals:
    void instantiatedRemoteObjects();
    void calledRemoteSlots();

private slots:
    void instantiateRemoteObjectPrivate();
    void on_remoteObjectConnected();
    void callRemoteSlotPrivate();
    void on_remoteSlotResponded();

private:
    QVector<QSharedPointer<RemoteObject>> m_remoteObjects;
    QVector<QSharedPointer<IRemoteSlotResponse>> m_remoteSlotResponses;
    quint32 m_remoteObjectsCount = 0;
    quint32 m_connectedRemoteObjectsCount = 0;
    quint32 m_remoteSlotCallsCount = 0;
    quint32 m_receivedRemoteSlotResponseCount = 0;
    quint32 m_currentRemoteObjectsCount = 0;
    int m_currentUrlIndex = 0;
    QList<QUrl> m_urls;
    quint32 m_currentRemoteObjectIndex = 0;
    quint32 m_iterationsCount = 0;
    quint32 m_currentRemoteSlotResponseIndex = 0;
};

}

#endif // CLIENT_LOAD_APP_WORKER_H
