#include "TcpConnection.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>

#include <gtest/gtest.h>

using namespace Challenge::Communication::Server;

class TcpConnectionTest : public ::testing::Test {
public:
    static constexpr uint16_t SERVER_PORT_EXPECTED_TO_BE_FREE = 54321;

    void SetUp() override {
        char const* params[] = { "app" };
        auto countParams = 1;
        m_app.reset( new QCoreApplication( countParams, const_cast<char**>(params) ) );
        m_server.reset( new QTcpServer );
        m_server->listen( QHostAddress::LocalHost, SERVER_PORT_EXPECTED_TO_BE_FREE );

        m_clientSocket = new QTcpSocket;
        m_clientSocket->connectToHost( QHostAddress::LocalHost, SERVER_PORT_EXPECTED_TO_BE_FREE );
        m_clientSocket->waitForConnected( 500 );
        m_app->processEvents();
        m_server->waitForNewConnection(500);
        m_app->processEvents();

        m_serverSocket = m_server->nextPendingConnection();

        assert(m_serverSocket);
    }

    void TearDown() override {
        m_server.reset();
        m_app->processEvents();
        m_app.reset();

    }

    QTcpSocket& getClientSocket() { return *m_clientSocket; }

private:
    std::shared_ptr< QCoreApplication > m_app;
    std::shared_ptr< QTcpServer > m_server;
    QPointer< QTcpSocket > m_clientSocket;
    QPointer< QTcpSocket > m_serverSocket;
};

TEST_F( TcpConnectionTest, CreateAbstraction ) {
    QTcpSocket notConnectedSocket;
    auto connectionNotConnected = ITransportConnection::create(&notConnectedSocket);
    ASSERT_EQ( connectionNotConnected, nullptr );

    auto connectionConnected = ITransportConnection::create(&getClientSocket());
    ASSERT_NE( connectionConnected, nullptr);
}

// Rest of test are done for QtTcpConnectionHelper, no sense to duplicate it here


