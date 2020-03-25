#pragma once

#include "Communication/Client/TransportConnectivityManager/ITransportConnection.h"
#include "Lib/QtTcpConnectionHelper/QtTcpConnectionHelper.h"

#include <QTcpSocket>
#include <QPointer>

#include <memory>
#include <mutex>

namespace Challenge {
namespace Communication {
namespace Client {

            class TcpConnection : public ITransportConnection {
            public:
                TcpConnection(QTcpSocket* _socket);
                ~TcpConnection() override;

                bool isValid() const override;
                bool registerConnectionExpiredCallback(ConnectionExpiredCallback _callback) override;
                bool registerNewDataReadyToReadCallback(NewDataReadyToReadCallback _callback) override;
                std::optional<Payload> receive() override;
                std::optional<uint32_t> send( const Payload& _payload ) override;

            private:
                QtTcpConnectionHelper m_qtConnectionHelper;
            };

} // namespace Client
} // namespace Communication
} // namespace Challenge