#pragma once

#include "Communication/Server/TransportConnectivityManager/ITransportConnection.h"
#include "Lib/QtTcpConnectionHelper/QtTcpConnectionHelper.h"

#include <QTcpSocket>
#include <QPointer>

#include <mutex>

namespace Challenge {
namespace Communication {
namespace Server {

            class TcpConnection : public ITransportConnection {
            public:
                TcpConnection(QPointer<QTcpSocket> _socket);
                ~TcpConnection() override = default;

                bool isValid() const override;
                bool registerConnectionExpiredCallback(ConnectionExpiredCallback _callback) override;
                bool registerNewDataReadyToReadCallback(NewDataReadyToReadCallback _callback) override;
                std::optional<Payload> receive() override;
                std::optional<uint32_t> send( const Payload& _payload ) override;

            private:
                QtTcpConnectionHelper m_qtConnectionHelper;
            };

} // namespace Server
} // namespace Communication
} // namespace Challenge