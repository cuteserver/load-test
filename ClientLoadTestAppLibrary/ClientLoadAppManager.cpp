//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#include "ClientLoadAppManager.h"
#include <QHostAddress>
#include <QCoreApplication>


namespace Cute::Tests
{

void ClientLoadAppManager::loadTestServer(const QStringList &arguments)
{
    if (!m_workers.isEmpty())
        return;
    if (6 != arguments.size())
    {
        emit error(QStringLiteral("Invalid command line arguments. DeploymentLoadTestClientApp expects the following arguments: --server-address, --server-ports, --threads, --connections and --iterationCount."));
        return;
    }
    bool hasAddress = false;
    bool hasPorts = false;
    bool hasThreads = false;
    bool hasConnections = false;
    bool hasIterationCount = false;
    for (const auto &arg : arguments)
    {
        if (arg.startsWith("--server-address="))
        {
            hasAddress = true;
            m_serverAddress = arg.right(arg.size() - 17);
        }
        else if (arg.startsWith("--server-ports="))
        {
            hasPorts = true;
            const QStringList portsAsText = arg.right(arg.size() - 15).split(';');
            for (const auto &portAsText: portsAsText)
                m_serverPorts.append(portAsText.toUShort());
        }
        else if (arg.startsWith("--threads="))
        {
            hasThreads = true;
            m_threadCount = arg.right(arg.size() - 10).toUInt();
        }
        else if (arg.startsWith("--connections="))
        {
            hasConnections = true;
            m_connectionCount = arg.right(arg.size() - 14).toUInt();
        }
        else if (arg.startsWith("--iterationCount="))
        {
            hasIterationCount = true;
            m_iterationCount = arg.right(arg.size() - 17).toUInt();
        }
    }
    if (!hasAddress || !hasPorts || !hasThreads || !hasConnections || !hasIterationCount)
    {
        emit error(QStringLiteral("Invalid command line arguments. DeploymentLoadTestClientApp expects the following arguments: --server-address, --server-ports, --threads, --connections and --iterationCount."));
        return;
    }
    if (QHostAddress(m_serverAddress).isNull())
        qFatal("Server address %s is not valid.", m_serverAddress.toUtf8().constData());
    if (m_serverPorts.isEmpty())
        qFatal("No ports were specified.");
    if (m_connectionCount / m_serverPorts.size() > 64000)
        qFatal("Failed to load test server. (connections count)/(server ports) > 64000.");
    if (0 == m_threadCount)
        qFatal("Thread count must be a positive integer.");
    m_elapsedTimer.start();
    for (int i = 0; i < m_threadCount; ++i)
    {
        m_workers.append(QSharedPointer<AsyncClientLoadAppWorker>(new AsyncClientLoadAppWorker, &QObject::deleteLater));
        QObject::connect(m_workers.last().data(), &AsyncClientLoadAppWorker::instantiatedRemoteObjects, this,
                         &ClientLoadAppManager::on_instantiatedRemoteObjects);
        QObject::connect(m_workers.last().data(), &AsyncClientLoadAppWorker::calledRemoteSlots, this, &ClientLoadAppManager::on_calledRemoteSlots);
        m_workers.last()->instantiateRemoteObjects(m_serverAddress, m_serverPorts, m_connectionCount / m_threadCount);
    }
}

void ClientLoadAppManager::on_instantiatedRemoteObjects()
{
    if (m_threadCount == ++m_instantiatedRemoteObjectsCount)
    {
        const auto elapsedTime = m_elapsedTimer.elapsed();
        qWarning("Created %d connections to remote objects in %lld msecs. %lld remote object connections per second.", m_connectionCount, elapsedTime, 1000*m_connectionCount/elapsedTime);
        m_elapsedTimer.start();
        for (auto &worker : m_workers)
            worker->callRemoteSlot(m_iterationCount);
    }
}

void ClientLoadAppManager::on_calledRemoteSlots()
{
    if (m_threadCount == ++m_remoteSlotsCalledCount)
    {
        const auto elapsedTime = m_elapsedTimer.elapsed();
        qWarning("Received %d requests in %lld msecs. %lld requests per second.", m_connectionCount * m_iterationCount, elapsedTime, 1000*(m_connectionCount * m_iterationCount)/elapsedTime);
        for (auto &worker : m_workers)
            QObject::connect(worker.data(), &QObject::destroyed, this, &ClientLoadAppManager::on_workerDestroyed);
        m_workers.clear();
    }
}

void ClientLoadAppManager::on_workerDestroyed()
{
    if (m_threadCount == ++m_destroyedWorkersCount)
        QCoreApplication::quit();
}

}
