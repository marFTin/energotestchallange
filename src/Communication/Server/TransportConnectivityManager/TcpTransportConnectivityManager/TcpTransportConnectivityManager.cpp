#include "TcpTransportConnectivityManager.h"

#include "Communication/Server/TransportConnectivityManager/ITransportConnectivityManager.h"
#include "Communication/Server/TransportConnectivityManager/ITransportConnection.h"

#include "Configuration/Defines.h"
#include "Lib/Log/Logger.h"

#include <QHostAddress>
#include <QTcpSocket>

#include <stdexcept>

namespace Challenge::Communication::Server {

std::unique_ptr<ITransportConnectivityManager>
ITransportConnectivityManager::create() try {
    QHostAddress address(SERVER_IP);
    return std::unique_ptr<ITransportConnectivityManager>(new TcpTransportConnectivityManager(address, SERVER_PORT));

} catch (std::runtime_error &_exception) {
    LOG_ERROR(_exception.what());
    return nullptr;
}

TcpTransportConnectivityManager::TcpTransportConnectivityManager(const QHostAddress &_hostAddress, uint16_t _port) {
    auto connectionResult = connect(&m_server, &QTcpServer::newConnection, this, &TcpTransportConnectivityManager::onNewConnection);
    if (!connectionResult) {
        throw std::runtime_error("Cannot connect signal with slot");
    }


    if (!m_server.listen(_hostAddress, _port)) {
        throw std::runtime_error(m_server.errorString().toStdString());
    }

    LOG_INFORMATION( "Start listening to connections" );
}

void
TcpTransportConnectivityManager::onNewConnection() {
    while (auto connection = m_server.nextPendingConnection()) {
       auto newTransportConnection = ITransportConnection::create( connection );
       if ( !newTransportConnection ) {
           LOG_ERROR( "Cannot start TCP connection" );
           connection->disconnectFromHost();
           continue;
       }

       if ( m_connectionCallback != nullptr ) {
           m_connectionCallback(std::move(newTransportConnection));
       }
    }
}

bool
TcpTransportConnectivityManager::registerNewConnectionCallback(NewConnectionCallback _callback) {
    bool result = m_connectionCallback != nullptr;
    m_connectionCallback = _callback;
    return result;
}

}// Challenge::Communication::Server
