//
// Copyright (c) 2022 Glauco Pacheco <glauco@cuteserver.io>
// All rights reserved
//

#include "LoadTestLibrary.h"

REGISTER_REMOTE_OBJECT("/load_test", Cute::Tests::LoadTest);


namespace Cute::Tests
{

LoadTest::LoadTest(const QSharedPointer<Cute::IConnectionInformation> &ci)
{
    Q_UNUSED(ci)
}

qint32 LoadTest::addIntegers(qint32 a, qint32 b)
{
    auto sum = a + b;
    return sum;
}

void LoadTest::ping(QSharedPointer<Cute::Server::HttpBroker> broker)
{
    Cute::Server::HttpResponse response(Cute::Server::HttpStatusCode::OK);
    response.addHeader("Connection", "close");
    broker->sendResponse(response);
}

}

