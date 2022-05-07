//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#include "ClientLoadAppManager.h"
#include <QHostAddress>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>


namespace Cute::Tests
{

const static quint16 clientPort = 2208;

static void deleteLaterFcn(QObject *obj)
{
    obj->deleteLater();
}

void ClientLoadAppManager::loadTestServer(const QStringList &arguments)
{
    if (!m_workers.isEmpty())
        return;
    if (1 == arguments.size())
        setupClient();
    else
        setupManager(arguments);
}

void ClientLoadAppManager::on_instantiatedRemoteObjects()
{
    if (m_threadCount == ++m_instantiatedRemoteObjectsCount)
    {
        const auto elapsedTime = m_elapsedTimer.elapsed();
        qWarning("Created %d connections to remote objects in %lld msecs. %lld remote object connections per second.", m_connectionCount, elapsedTime, 1000*m_connectionCount/elapsedTime);
        QJsonObject jsonObj;
        jsonObj["sentTo"] = "manager";
        jsonObj["action"] = "instantiated remote objects";
        jsonObj["host"] = m_sockets[0]->localAddress().toString();
        jsonObj["connectionCount"] = static_cast<qint64>(m_connectionCount);
        jsonObj["elapsedTime"] = elapsedTime;
        const QJsonDocument msg(jsonObj);
        m_sockets[0]->sendTextMessage(msg.toJson(QJsonDocument::Compact));
    }
}

void ClientLoadAppManager::on_calledRemoteSlots()
{
    if (m_threadCount == ++m_remoteSlotsCalledCount)
    {
        const auto elapsedTime = m_elapsedTimer.elapsed();
        qWarning("Received %d requests in %lld msecs. %lld requests per second.", m_connectionCount * m_iterationCount, elapsedTime, 1000*(m_connectionCount * m_iterationCount)/elapsedTime);
        QJsonObject jsonObj;
        jsonObj["sentTo"] = "manager";
        jsonObj["action"] = "called remote slots";
        jsonObj["host"] = m_sockets[0]->localAddress().toString();
        jsonObj["iterationCount"] = static_cast<qint64>(m_iterationCount);
        jsonObj["connectionCount"] = static_cast<qint64>(m_connectionCount);
        jsonObj["elapsedTime"] = elapsedTime;
        const QJsonDocument msg(jsonObj);
        QObject::connect(m_sockets[0].data(), &QWebSocket::disconnected, this, &ClientLoadAppManager::quitClient);
        m_sockets[0]->sendTextMessage(msg.toJson(QJsonDocument::Compact));
        m_sockets[0]->close();
    }
}

void ClientLoadAppManager::on_workerDestroyed()
{
    if (m_threadCount == ++m_destroyedWorkersCount)
        QCoreApplication::quit();
}

void ClientLoadAppManager::on_newConnection()
{
    if (!m_sockets.isEmpty())
        qFatal("Clients expect only one connection.");
    auto socket = m_server->nextPendingConnection();
    if (!socket)
        qFatal("Socket is null.");
    m_sockets.append(QSharedPointer<QWebSocket>(socket, deleteLaterFcn));
    if (nullptr != m_server->nextPendingConnection())
        qFatal("Clients expect only one connection.");
    qWarning("Client has connected to manager at: %s", m_sockets[0]->peerAddress().toString().toUtf8().constData());
    QObject::connect(m_sockets[0].data(), &QWebSocket::textMessageReceived, this, &ClientLoadAppManager::on_newMessage);
}

void ClientLoadAppManager::quitClient()
{
    for (auto &worker : m_workers)
        QObject::connect(worker.data(), &QObject::destroyed, this, &ClientLoadAppManager::on_workerDestroyed);
    m_workers.clear();
}

void ClientLoadAppManager::on_newMessage(const QString &message)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
    if (!jsonDoc.isObject() || jsonDoc.isEmpty())
        qFatal("Client received an invalid message.");
    const auto jsonObj = jsonDoc.object();
    if (!jsonObj.contains("sentTo"))
        qFatal("Client/Manager received an invalid message.");
    if ("client" == jsonObj["sentTo"].toString())
    {
        if ("instantiate remote objects" == jsonObj["action"].toString())
        {
            m_serverAddress = jsonObj["serverAddress"].toString();
            const QStringList portsAsStr = jsonObj["serverPorts"].toString().split(';');
            for (const auto &portAsStr : portsAsStr)
                m_serverPorts.append(portAsStr.toUShort());
            m_threadCount = jsonObj["threadCount"].toInt();
            m_connectionCount = jsonObj["connectionCount"].toInt();
            m_iterationCount = jsonObj["iterationCount"].toInt();
            qWarning("Instantiating remote objects with the following configuration:");
            qWarning("    server address: %s", m_serverAddress.toUtf8().constData());
            qWarning("    server ports: %s", jsonObj["serverPorts"].toString().toUtf8().constData());
            qWarning("    thread count: %d", m_threadCount);
            qWarning("    connection count: %d", m_connectionCount);
            qWarning("    iteration count: %d", m_iterationCount);
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
        else if ("call remote slots" == jsonObj["action"].toString())
        {
            m_elapsedTimer.start();
            for (auto &worker : m_workers)
                worker->callRemoteSlot(m_iterationCount);
        }
        else
            qFatal("Client received an invalid message.");
    }
    else if ("manager" == jsonObj["sentTo"].toString())
    {
        if ("instantiated remote objects" == jsonObj["action"].toString())
        {
            if (m_clients.size() == ++m_instantiatedRemoteObjectsCount)
            {
                const auto elapsedTime = m_elapsedTimer.elapsed();
                qWarning("Created %lld connections to remote objects in %lld msecs. "
                         "%lld remote object connections per second.",
                         m_connectionCount * m_clients.size(), elapsedTime,
                         1000*(m_connectionCount * m_clients.size())/elapsedTime);
                QJsonObject msgJsonObj;
                msgJsonObj["sentTo"] = "client";
                msgJsonObj["action"] = "call remote slots";
                const QByteArray msg = QJsonDocument(msgJsonObj).toJson(QJsonDocument::Compact);
                for (auto &socket : m_sockets)
                    socket->sendTextMessage(msg);
                m_elapsedTimer.start();
            }
            else
                qWarning("Client at %s has instantiated all of its remote objects in %d msecs.",
                         jsonObj["host"].toString().toUtf8().constData(),
                         jsonObj["elapsedTime"].toInt());
        }
        else if ("called remote slots" == jsonObj["action"].toString())
        {
            if (m_clients.size() == ++m_remoteSlotsCalledCount)
            {
                const auto elapsedTime = m_elapsedTimer.elapsed();
                qWarning("Remote slots were called %lld times in %lld msecs. "
                         "%lld remote slot calls per second.",
                         m_connectionCount * m_iterationCount * m_clients.size(), elapsedTime,
                         1000*(m_connectionCount * m_iterationCount * m_clients.size())/elapsedTime);
                for (auto &socket : m_sockets)
                    socket->close();
                QCoreApplication::quit();
            }
            else
                qWarning("Client at %s has called remote slots in %d msecs.",
                         jsonObj["host"].toString().toUtf8().constData(),
                         jsonObj["elapsedTime"].toInt());
        }
    }
    else
        qFatal("Client/Manager received an invalid message.");

}

void ClientLoadAppManager::on_connectedToClient()
{
    if (m_clients.size() == ++m_connectedClients)
    {
        qWarning("Manager has connected to clients.\n"
                 "Instantiating remote objects.");
        QJsonObject jsonObj;
        jsonObj["sentTo"] = "client";
        jsonObj["action"] = "instantiate remote objects";
        jsonObj["serverAddress"] = m_serverAddress;
        QStringList serverPorts;
        for (const auto &serverPort : m_serverPorts)
            serverPorts.append(QString::number(serverPort));
        jsonObj["serverPorts"] = serverPorts.join(';');
        jsonObj["threadCount"] = static_cast<int>(m_threadCount);
        jsonObj["connectionCount"] = static_cast<int>(m_connectionCount);
        jsonObj["iterationCount"] = static_cast<int>(m_iterationCount);
        const QByteArray msg = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
        for (auto &socket : m_sockets)
            socket->sendTextMessage(msg);
        m_elapsedTimer.start();
    }
}

void ClientLoadAppManager::setupClient()
{
    if (!m_server.isNull())
        qFatal("Failed to setup QWebSocketServer. Server already exists.");
    m_server.reset(new QWebSocketServer(QString(), QWebSocketServer::NonSecureMode));
    QObject::connect(m_server.data(), &QWebSocketServer::newConnection, this, &ClientLoadAppManager::on_newConnection);
    if (!m_server->listen(QHostAddress::AnyIPv4, clientPort))
        qFatal("Failed to setup QWebSocketServer. Failed to listen.");
    qWarning("Client in listening for manager on port %d.", clientPort);
}

void ClientLoadAppManager::setupManager(const QStringList &arguments)
{
    if (7 != arguments.size())
    {
        emit error(QStringLiteral("Invalid command line arguments. When running as manager, Client Test App expects the following arguments: --clients, --server-address, --server-ports, --threads, --connections and --iterationCount."));
        return;
    }
    bool hasClients = false;
    bool hasAddress = false;
    bool hasPorts = false;
    bool hasThreads = false;
    bool hasConnections = false;
    bool hasIterationCount = false;
    for (const auto &arg : arguments)
    {
        if (arg.startsWith("--clients="))
        {
            hasClients = true;
            m_clients = arg.right(arg.size() - 10).split(';');
        }
        else if (arg.startsWith("--server-address="))
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
    if (!hasClients || !hasAddress || !hasPorts || !hasThreads || !hasConnections || !hasIterationCount)
    {
        emit error(QStringLiteral("Invalid command line arguments. When running as manager, "
                                  "Client Test App expects the following arguments: "
                                  "--clients, --server-address, --server-ports, --threads, "
                                  "--connections and --iterationCount."));
        return;
    }
    if (m_clients.isEmpty())
        qFatal("Manager expects clients to be specified in command line "
               "as follows: --clients=ip1;ip2;ip3;...");
    for (const auto &client : m_clients)
    {
        if (QHostAddress(client).isNull())
            qFatal("Client %s is not valid.", client.toUtf8().constData());
    }
    if (QHostAddress(m_serverAddress).isNull())
        qFatal("Server address %s is not valid.", m_serverAddress.toUtf8().constData());
    if (m_serverPorts.isEmpty())
        qFatal("No ports were specified.");
    if (m_connectionCount / m_serverPorts.size() > 64000)
        qFatal("Failed to load test server. (connections count)/(server ports) > 64000.");
    if (0 == m_threadCount)
        qFatal("Thread count must be a positive integer.");
    for (const auto &client : m_clients)
    {
        m_sockets.append(QSharedPointer<QWebSocket>(new QWebSocket, deleteLaterFcn));
        QObject::connect(m_sockets.last().data(), &QWebSocket::connected, this, &ClientLoadAppManager::on_connectedToClient);
        QObject::connect(m_sockets.last().data(), &QWebSocket::textMessageReceived, this, &ClientLoadAppManager::on_newMessage);
        QObject::connect(m_sockets.last().data(),  QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), [](QAbstractSocket::SocketError se)
        {
            qFatal("Manager failed to connect to client. %d", se);
        });
        m_sockets.last()->open(QUrl(QStringLiteral("ws://%1:%2").arg(client, QString::number(clientPort))));
    }
}

}
