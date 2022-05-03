//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#ifndef LOAD_TEST_LIBRARY_H
#define LOAD_TEST_LIBRARY_H

#include <CuteServer.h>
#include <QSharedPointer>
#include <QScopedPointer>

namespace Cute::Tests
{

class LoadTest : public QObject
{
Q_OBJECT
public:
    explicit LoadTest(const QSharedPointer<Cute::IConnectionInformation> &ci);
    ~LoadTest() override = default;

public slots:
    REMOTE_SLOT qint32 addIntegers(qint32 a, qint32 b);
    HTTP_GET void ping(QSharedPointer<Cute::Server::HttpBroker> broker);
};

}

#endif // LOAD_TEST_LIBRARY_H
