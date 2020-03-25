#include "Lib/QtTcpConnectionHelper/QtTcpConnectionHelper.h"

#include "Lib/Log/Logger.h"

#include <cassert>

namespace Challenge {

QtTcpConnectionHelper::QtTcpConnectionHelper(QPointer<QTcpSocket> _socket) : m_connectedSocket( std::move(_socket) ) {
    assert( m_connectedSocket );

    if ( !m_connectedSocket->isValid() ) {
        throw std::runtime_error( "Socket is not connected" );
    }


    m_dataStream.setDevice( m_connectedSocket );
    m_dataStream.setVersion( QDataStream::Qt_4_0 );

    if ( !connect( m_connectedSocket, &QIODevice::readyRead, this, &QtTcpConnectionHelper::onDataArrived ) ) {
        throw std::runtime_error( "Cannot connect signals with slot" );
    }

    if ( !connect( m_connectedSocket, &QAbstractSocket::disconnected, this, &QtTcpConnectionHelper::onDisconnected ) ) {
        throw std::runtime_error( "Cannot connect signals with slot" );
    }

    LOG_INFORMATION( "New connection established");
}

QtTcpConnectionHelper::~QtTcpConnectionHelper() {
    assert(m_connectedSocket);

    m_connectedSocket->deleteLater();
}

bool
QtTcpConnectionHelper::isValid() const {
    assert( m_connectedSocket );

    return m_connectedSocket->isValid() && m_connectedSocket->state() == QAbstractSocket::ConnectedState;
}

bool
QtTcpConnectionHelper::registerConnectionExpiredCallback(ConnectionExpiredCallback _callback) {
    assert( m_connectedSocket );

    std::lock_guard lock(m_callbacksMutex);

    bool result = m_connectionExpiredCallback != nullptr;
    m_connectionExpiredCallback = _callback;
    return result;
}

bool
QtTcpConnectionHelper::registerNewDataReadyToReadCallback(NewDataReadyToReadCallback _callback) {
    assert( m_connectedSocket );

    std::lock_guard lock(m_callbacksMutex);

    bool result = m_newDataReadyToReadCallback != nullptr;
    m_newDataReadyToReadCallback = _callback;
    return result;
}

std::optional<QtTcpConnectionHelper::Payload>
QtTcpConnectionHelper::receive() {
    static_assert( sizeof(std::byte) == sizeof(char), "Char must be converted to std::byte"  );
    assert( m_connectedSocket );
    m_connectedSocket->waitForReadyRead(0);

    std::lock_guard guard( m_rawDataMutex );

    if (!isValid()) {
        return std::nullopt;
    }

    /* I know here copy is made, but now is important time of implementation, if system will start to work
     * then we can optimize this
     */
    // TODO: remove copy QByteArray to std::string
    auto beginIt = reinterpret_cast<const std::byte*>( (const char*)(m_rawDataFromSocket) );
    auto endIt = beginIt + m_rawDataFromSocket.size();
    Payload payload(beginIt, endIt);
    m_rawDataFromSocket.clear();
    return std::move( payload );
}

std::optional<uint32_t>
QtTcpConnectionHelper::send( const QtTcpConnectionHelper::Payload& _payload ) {
    assert( m_connectedSocket );
    static_assert( sizeof(std::byte) == sizeof(char), "Char must be converted to std::byte" );

    if ( _payload.size() > std::numeric_limits<uint32_t>::max() ) {
        return std::nullopt;
    }

    if ( !isValid() ) {
        return std::nullopt;
    }

    auto numberOfSent = m_connectedSocket->write(reinterpret_cast<const char*>(_payload.data()), _payload.size() );
    m_connectedSocket->flush();
    if ( numberOfSent == -1 ) {
        return std::nullopt;
    }

    return static_cast<uint32_t>(numberOfSent);
}

void
QtTcpConnectionHelper::onDataArrived() {
    assert(m_connectedSocket);

    {
        std::lock_guard lock(m_rawDataMutex);
        m_rawDataFromSocket.append(m_connectedSocket->readAll());
    }

    {
        std::lock_guard lock(m_callbacksMutex);
        if (m_newDataReadyToReadCallback != nullptr) {
            m_newDataReadyToReadCallback();
        }
    }
}

void
QtTcpConnectionHelper::onDisconnected() {
    LOG_INFORMATION( "Connection lost" );
    assert(m_connectedSocket);

    std::lock_guard lock(m_callbacksMutex);
    if (m_connectionExpiredCallback != nullptr) {
        m_connectionExpiredCallback();
    }
}

} // namespace Challenge
