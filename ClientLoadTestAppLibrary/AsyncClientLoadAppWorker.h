//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#ifndef ASYNC_CLIENT_LOAD_APP_WORKER_H
#define ASYNC_CLIENT_LOAD_APP_WORKER_H


#include "ClientLoadAppWorker.h"
#include "AsyncQObjectRunner.h"
#include <QObject>


namespace Cute::Tests
{

class AsyncClientLoadAppWorker : public QObject
{
Q_OBJECT
public:
    AsyncClientLoadAppWorker();
    ~AsyncClientLoadAppWorker() override = default;

public slots:
    void instantiateRemoteObjects(const QString &serverAddress,
                                  const QList<quint16> &serverPorts,
                                  quint32 remoteObjectsCount);
    void callRemoteSlot(quint32 iterations);

signals:
    void instantiatedRemoteObjects();
    void calledRemoteSlots();

private slots:
    void on_instantiatedRemoteObjects() {emit instantiatedRemoteObjects();}
    void on_calledRemoteSlots() {emit calledRemoteSlots();}

private:
    AsyncQObjectRunner<ClientLoadAppWorker> m_asyncWorker;
};

}

#endif // ASYNC_CLIENT_LOAD_APP_WORKER_H
