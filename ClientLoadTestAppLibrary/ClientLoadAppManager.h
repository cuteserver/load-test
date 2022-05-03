//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#ifndef CLIENT_LOAD_APP_MANAGER_H
#define CLIENT_LOAD_APP_MANAGER_H

#include "AsyncClientLoadAppWorker.h"
#include <QObject>
#include <QList>
#include <QSharedPointer>
#include <QElapsedTimer>

namespace Cute::Tests
{

class ClientLoadAppManager : public QObject
{
Q_OBJECT
public:
    ClientLoadAppManager() = default;
    ~ClientLoadAppManager() override = default;

public slots:
    void loadTestServer(const QStringList &arguments);

signals:
    void instantiatedRemoteObjects(quint32 remoteObjectsCount, quint64 elapsedTimeInMSecs);
    void receivedResponses(quint64 responsesCount, quint64 elapsedTimeInMSecs);
    void error(QString errorMessage);

private slots:
    void on_instantiatedRemoteObjects();
    void on_calledRemoteSlots();
    void on_workerDestroyed();

private:
    QList<QSharedPointer<AsyncClientLoadAppWorker>> m_workers;
    QElapsedTimer m_elapsedTimer;
    QString m_serverAddress;
    QList<quint16> m_serverPorts;
    quint32 m_threadCount = 0;
    quint32 m_connectionCount = 0;
    quint32 m_iterationCount = 0;
    int m_instantiatedRemoteObjectsCount = 0;
    int m_remoteSlotsCalledCount = 0;
    int m_destroyedWorkersCount = 0;
};

}

#endif // CLIENT_LOAD_APP_MANAGER_H
