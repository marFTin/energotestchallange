#include "TcpConnectivityManager.h"

#include "Configuration/Defines.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpSocket>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace Challenge::Communication::Client;

TEST( ClientTcpTranportConnectivityManager, CreateAbstraction ) {
    auto manager = ITransportConnectivityManager::create();
    ASSERT_NE( manager, nullptr );
}

TEST( ClientTcpTranportConnectivityManager, CannotConnectToServer ) {
    char const* parameters[] = { "app" };
    auto parametersNumber = 1;
    QCoreApplication app(parametersNumber, const_cast<char**>(parameters));

    TcpConnectivityManager unitUnderTest;

    ASSERT_EQ( unitUnderTest.connectToServer(), nullptr );
}


TEST( ClientTcpTranportConnectivityManager, StartNewConnectionPositive ) {
    char const* parameters[] = { "app" };
    auto parametersNumber = 1;
    QCoreApplication app(parametersNumber, const_cast<char**>(parameters));

    QTcpServer server;
    server.listen(QHostAddress(SERVER_IP), SERVER_PORT );

    TcpConnectivityManager connectivityManager;

    app.processEvents();

    auto newConnection = connectivityManager.connectToServer();

    ASSERT_NE( newConnection, nullptr );
}
