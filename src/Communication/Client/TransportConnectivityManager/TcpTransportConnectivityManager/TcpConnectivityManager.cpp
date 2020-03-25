#include "TcpConnectivityManager.h"

#include "Communication/Client/TransportConnectivityManager/ITransportConnection.h"

#include "Configuration/Defines.h"
#include "Lib/Log/Logger.h"

#include <QCoreApplication>
#include <QTcpSocket>

#include <cassert>
#include <stdexcept>

namespace Challenge::Communication::Client {

std::shared_ptr<ITransportConnectivityManager>
ITransportConnectivityManager::create() {
    return std::unique_ptr<ITransportConnectivityManager>(new TcpConnectivityManager());
}


std::shared_ptr<ITransportConnection>
TcpConnectivityManager::connectToServer() const {

    auto tcpSocket = new QTcpSocket(QCoreApplication::instance());
    tcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption, true);
    tcpSocket->connectToHost( QHostAddress(SERVER_IP), SERVER_PORT );
    bool isConnected = tcpSocket->waitForConnected( 3000 );

    if ( !isConnected ) {
        return nullptr;
    }

    return ITransportConnection::create(tcpSocket);
}



}// Challenge::Communication::Server
