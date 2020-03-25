#include "Lib/QtTcpConnectionHelper/QtTcpConnectionHelper.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

using namespace Challenge;

class QtTcpConnectionHelperTest : public ::testing::Test {
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
    QTcpSocket& getServerSocket() { return *m_serverSocket; }
    void processEvents() { m_app->processEvents(); }

    void sendBytesFromClient(uint32_t _numberOfBytes ) {
        assert( _numberOfBytes < 4 );

        static std::vector<char> dataToSend = { 'A', 'B', 'C' };
        getClientSocket().write( dataToSend.data(), _numberOfBytes );
        processEvents();
        getServerSocket().waitForReadyRead(500);
        processEvents();
    }

private:
    std::shared_ptr< QCoreApplication > m_app;
    std::shared_ptr< QTcpServer > m_server;
    QPointer< QTcpSocket > m_clientSocket;
    QPointer< QTcpSocket > m_serverSocket;
};

TEST_F( QtTcpConnectionHelperTest, Construction ) {
    QTcpSocket notConnectedSocket;
    EXPECT_THROW( QtTcpConnectionHelper( QPointer(&notConnectedSocket) ), std::runtime_error );

    EXPECT_NO_THROW( QtTcpConnectionHelper( QPointer( &getServerSocket() ) ) );

}

TEST_F( QtTcpConnectionHelperTest, ReceivePayload ) {
    QtTcpConnectionHelper connectionUnderTest( &getServerSocket() );

    sendBytesFromClient(1); // 'A'
    auto receivedData1 = connectionUnderTest.receive();

    ASSERT_TRUE( receivedData1.has_value() );
    ASSERT_EQ( receivedData1.value().size(), 1 );
    ASSERT_EQ( (char)receivedData1.value()[0], 'A' );

    sendBytesFromClient(3); // 'A', 'B', 'C'
    auto receivedData3 = connectionUnderTest.receive();

    ASSERT_TRUE( receivedData3.has_value() );
    ASSERT_EQ( receivedData3.value().size(), 3 );
    ASSERT_EQ( (char)receivedData3.value()[0], 'A' );
    ASSERT_EQ( (char)receivedData3.value()[1], 'B' );
    ASSERT_EQ( (char)receivedData3.value()[2], 'C' );

    auto noDataReceived = connectionUnderTest.receive();
    ASSERT_TRUE( noDataReceived.has_value() );
    ASSERT_EQ( noDataReceived->size(), 0 );
}

TEST_F( QtTcpConnectionHelperTest, ReceivePayloadFailed ) {
    QtTcpConnectionHelper connectionUnderTest( &getServerSocket() );

    getClientSocket().close();
    processEvents();

    auto receivedData = connectionUnderTest.receive();
    ASSERT_FALSE( receivedData.has_value() );
}

TEST_F( QtTcpConnectionHelperTest, OnNewDataCallbackFired ) {
    QtTcpConnectionHelper connectionUnderTest( &getServerSocket() );

    bool callbackFired = false;
    auto callback = [&callbackFired](){
        callbackFired = true;
    };

    ASSERT_FALSE( connectionUnderTest.registerNewDataReadyToReadCallback( callback ) );

    sendBytesFromClient(1);

    ASSERT_TRUE( callbackFired );
}

TEST_F( QtTcpConnectionHelperTest, OnConnectionExpiredCallback ) {
    QtTcpConnectionHelper connectionUnderTest( &getServerSocket() );

    bool callbackFired = false;
    auto callback = [&callbackFired](){
        callbackFired = true;
    };

    ASSERT_FALSE( connectionUnderTest.registerConnectionExpiredCallback( callback ) );

    getClientSocket().close();
    processEvents();

    ASSERT_TRUE( callbackFired );
}

TEST_F( QtTcpConnectionHelperTest, SendPayload ) {
    QtTcpConnectionHelper connectionUnderTest( &getServerSocket() );

    QtTcpConnectionHelper::Payload dataToSend = { std::byte('A'), std::byte('B'), std::byte('C') };

    auto result = connectionUnderTest.send( dataToSend );
    processEvents();
    getClientSocket().waitForBytesWritten(500);
    processEvents();
    auto receivedData = getClientSocket().readAll();

    ASSERT_TRUE( result.has_value() );
    ASSERT_EQ( result.value(), dataToSend.size() );
    ASSERT_EQ( receivedData.size(), dataToSend.size() );
    ASSERT_EQ( std::byte( char( receivedData[0] ) ), std::byte('A') );
    ASSERT_EQ( std::byte( char( receivedData[1] ) ), std::byte('B') );
    ASSERT_EQ( std::byte( char( receivedData[2] ) ), std::byte('C') );
}


