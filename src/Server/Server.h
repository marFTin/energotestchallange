#pragma once

#include <QTimer>

#include <chrono>
#include <memory>
#include <vector>

namespace Challenge {
namespace EventsStorage {
        class IEventsStorage;
} // namespace Storage
} // namespace Challenge

namespace Challenge {
namespace Communication {
namespace Server {

            class ITransportConnectivityManager;

            class ITransportConnection;

            class IProtocolExecutor;

            class Server : public QObject {
            Q_OBJECT
            public:
                //! Constructor
                /*!
                *
                * @throw may throw std::runtime_error
                */
                Server();

                Server(const Server &) = delete;
                Server(Server &&) = delete;
                Server &operator=(Server &) = delete;
                Server &operator=(Server &&) = delete;

            private slots:
                void onServicesCheck();

            private:
                void onNewConnection(std::shared_ptr<ITransportConnection> _newConnection);
                void checkPendingConnections();
                void agingConnections();
                void handshakeOnConnections();
                void checkProtocolsExecutors();

            private:
                using ConnectionStartTimePoint = std::chrono::time_point<std::chrono::steady_clock>;
                using ConnectionAndTime = std::pair<std::shared_ptr<ITransportConnection>, ConnectionStartTimePoint>;
                using ConnectionsWaitingForHandshake = std::vector<ConnectionAndTime>;
                using ProtocolsExecutors = std::vector<std::shared_ptr<IProtocolExecutor> >;

                std::shared_ptr<ITransportConnectivityManager> m_connectivityManager;
                std::shared_ptr<Challenge::EventsStorage::IEventsStorage> m_storage;

                ConnectionsWaitingForHandshake m_connectionWaitingForHandshake;
                ProtocolsExecutors m_protocolsExecutors;

                QTimer m_timer;
            };
} //namespace Server
} // namespace Communication
} // namespace Challenge
