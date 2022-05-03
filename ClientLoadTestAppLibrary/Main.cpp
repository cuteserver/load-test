//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#include "ClientLoadAppManager.h"
#include "ThreadFactory.h"
#include <QCoreApplication>
#include <QThread>

using namespace Cute::Tests;

extern "C" __attribute__((visibility("default"))) int lib_main(int argc, char ** argv, QThread*(*threadFactoryFcn)())
{
    QCoreApplication app(argc, argv);
    ThreadFactory::setThreadFactoryFcn(threadFactoryFcn);
    QScopedPointer<ClientLoadAppManager, QScopedPointerDeleteLater> clientLoadAppManager(new ClientLoadAppManager);
    QObject::connect(clientLoadAppManager.data(), &ClientLoadAppManager::instantiatedRemoteObjects, [](quint32 remoteObjectsCount, quint64 elapsedTimeInMSecs)
    {
        QTextStream stream(stdout);
        stream << QStringLiteral("Instantiated %1 remote objects in %2 msecs. %3 instances per second.").arg(QString::number(remoteObjectsCount), QString::number(elapsedTimeInMSecs), QString::number(1000*remoteObjectsCount/elapsedTimeInMSecs));
        stream.flush();
    });
    QObject::connect(clientLoadAppManager.data(), &ClientLoadAppManager::receivedResponses, [](quint64 responsesCount, quint64 elapsedTimeInMSecs)
    {
        QTextStream stream(stdout);
        stream << QStringLiteral("Received %1 remote slot call responses in %2 msecs. %3 responses per second.").arg(QString::number(responsesCount), QString::number(elapsedTimeInMSecs), QString::number(1000*responsesCount/elapsedTimeInMSecs));
        stream.flush();
        QCoreApplication::quit();
    });
    QObject::connect(clientLoadAppManager.data(), &ClientLoadAppManager::error, [](QString errorMessage)
    {
        QTextStream stream(stdout);
        stream << QStringLiteral("Failed to load test server. %1").arg(errorMessage);
        stream.flush();
        QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
    });
    clientLoadAppManager->loadTestServer(QCoreApplication::arguments());
    return QCoreApplication::exec();
}
