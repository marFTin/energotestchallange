#pragma once

#include "Communication/Client/TransportConnectivityManager/ITransportConnectivityManager.h"

#include <QObject>
#include <QTcpServer>

#include <cinttypes>

namespace Challenge {
namespace Communication {
namespace Client {

    class TcpConnectivityManager : public ITransportConnectivityManager {
        public:
            TcpConnectivityManager() = default;
            ~TcpConnectivityManager() override = default;

            virtual std::shared_ptr<ITransportConnection> connectToServer() const override;
    };

} // Client
} // Communication
} // Challenge



