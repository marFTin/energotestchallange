#include "TcpConnection.h"

#include "Lib/Log/Logger.h"

namespace Challenge::Communication::Client {

template<>
std::shared_ptr<ITransportConnection> ITransportConnection::create<QTcpSocket*>(QTcpSocket* _socket) try {
    return std::make_shared<TcpConnection>( _socket );
} catch ( std::runtime_error& _exception ) {
    LOG_ERROR(_exception.what());
    return nullptr;
}

template std::shared_ptr<ITransportConnection> ITransportConnection::create<QTcpSocket*>(QTcpSocket*);

TcpConnection::TcpConnection(QTcpSocket* _socket) :
    m_qtConnectionHelper( _socket ) {
}

TcpConnection::~TcpConnection() {

}

bool
TcpConnection::isValid() const {
    return m_qtConnectionHelper.isValid();
}

bool
TcpConnection::registerConnectionExpiredCallback(ConnectionExpiredCallback _callback) {
    return m_qtConnectionHelper.registerConnectionExpiredCallback(_callback);
}

bool
TcpConnection::registerNewDataReadyToReadCallback(NewDataReadyToReadCallback _callback) {
    return m_qtConnectionHelper.registerNewDataReadyToReadCallback(_callback);
}

std::optional<ITransportConnection::Payload>
TcpConnection::receive() {
    return m_qtConnectionHelper.receive();
}

std::optional<uint32_t>
TcpConnection::send( const ITransportConnection::Payload& _payload ) {
    return m_qtConnectionHelper.send(_payload);
}

} // namespace Challenge::Communication::Client
