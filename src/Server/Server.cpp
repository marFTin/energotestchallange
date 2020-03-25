#include "Server.h"

#include "Communication/Server/TransportConnectivityManager/ITransportConnectivityManager.h"
#include "Communication/Server/TransportConnectivityManager/ITransportConnection.h"
#include "Communication/Server/IHandshake.h"
#include "Communication/Server/IProtocolExecutor.h"

#include "EventsStorage/IEventsStorage.h"

#include <stdexcept>

namespace Challenge::Communication::Server {

Server::Server() {
    m_connectivityManager = ITransportConnectivityManager::create();

    if ( !m_connectivityManager ) {
        throw std::runtime_error("Cannot create connectivity manager");
    }

    m_storage = Challenge::EventsStorage::IEventsStorage::create();

    if ( !m_storage ) {
        throw std::runtime_error("Cannot create storage");
    }

    m_connectivityManager->registerNewConnectionCallback([this](auto _connection){onNewConnection(_connection);});

    auto connectionResult = connect( &m_timer, &QTimer::timeout, this, &Server::onServicesCheck );
    if (!connectionResult ) {
        throw std::runtime_error( "Cannot connect slot with QTimer signal" );
    }

    m_timer.setInterval( 200 );
    m_timer.start();
}

void
Server::onNewConnection( std::shared_ptr<ITransportConnection> _newConnection ) {
    auto time = std::chrono::steady_clock::now();
    m_connectionWaitingForHandshake.push_back( std::make_pair(_newConnection, time) );
}

void
Server::agingConnections() {
    auto time = std::chrono::steady_clock::now();

    m_connectionWaitingForHandshake.erase(
            std::remove_if(
                    m_connectionWaitingForHandshake.begin()
                    , m_connectionWaitingForHandshake.end()
                    , [time](const auto& _connectionAndTime ) {
                        return (time - _connectionAndTime.second ) >= std::chrono::seconds(1);
                    }
            )
            , m_connectionWaitingForHandshake.end()
    );
}

void
Server::handshakeOnConnections() {
    auto createHandshake = [this]( ConnectionAndTime& _connectionAndTime ) {
        auto handshake = IHandshake::start( _connectionAndTime.first );

        if ( !handshake ) {
            return false;
        }

        auto protocolExecutor = IProtocolExecutor::create( handshake, m_storage );
        if (!protocolExecutor) {
            return false;
        }

        m_protocolsExecutors.push_back( protocolExecutor );
        return true;
    };

    m_connectionWaitingForHandshake.erase(
            std::remove_if(
                    m_connectionWaitingForHandshake.begin()
                    , m_connectionWaitingForHandshake.end()
                    , createHandshake
            )
            , m_connectionWaitingForHandshake.end()
    );
}


void
Server::checkPendingConnections() {
   agingConnections();
   handshakeOnConnections();
}

void
Server::checkProtocolsExecutors() {
    m_protocolsExecutors.erase(
            std::remove_if(
                    m_protocolsExecutors.begin()
                    , m_protocolsExecutors.end()
                    , [](auto _protocolExecutor ) {return !_protocolExecutor->isValid();}
            )
            , m_protocolsExecutors.end()
    );
}

void
Server::onServicesCheck() {
    checkPendingConnections();
    checkProtocolsExecutors();
}

} // namespace Challenge::Communication::Server
