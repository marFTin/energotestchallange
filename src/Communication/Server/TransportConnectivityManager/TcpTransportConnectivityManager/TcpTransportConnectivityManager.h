#pragma once

#include "Communication/Server/TransportConnectivityManager/ITransportConnectivityManager.h"

#include <QObject>
#include <QTcpServer>

#include <cinttypes>

class QHostAddress;

// moc does not support a::b::c ? wtf ?
namespace Challenge {
namespace Communication {
namespace Server {

    class TcpTransportConnectivityManager : public QObject, public ITransportConnectivityManager {
        Q_OBJECT
        public:
            //! Constructor
            /*!
             *
             * @param _hostAddress
             * @param port tcp port
             * @throw std::runtime_error is cannot start to listen on given address and port
             */
            TcpTransportConnectivityManager(const QHostAddress& _hostAddress, uint16_t _port);
            ~TcpTransportConnectivityManager() override = default;

            bool registerNewConnectionCallback( NewConnectionCallback _callback) override;

        private slots:
            void onNewConnection();

        private:
            NewConnectionCallback m_connectionCallback;
            QTcpServer m_server;
    };

} // Communication
} // Server
} // Challenge



