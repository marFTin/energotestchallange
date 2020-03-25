#pragma once

#include <QObject>

#include <QByteArray>
#include <QDataStream>
#include <QTcpSocket>
#include <QPointer>

#include <functional>
#include <mutex>
#include <cstddef>
#include <optional>

namespace Challenge {

    class QtTcpConnectionHelper : public QObject {
                Q_OBJECT
            public:
                using Payload = std::vector<std::byte>;
                using ConnectionExpiredCallback = std::function<void(void)>;
                using NewDataReadyToReadCallback = std::function<void(void)>;

                QtTcpConnectionHelper(QPointer<QTcpSocket> _socket);
                ~QtTcpConnectionHelper() override;

                bool isValid() const;
                bool registerConnectionExpiredCallback(ConnectionExpiredCallback _callback);
                bool registerNewDataReadyToReadCallback(NewDataReadyToReadCallback _callback);
                std::optional<Payload> receive();
                std::optional<uint32_t> send( const Payload& _payload );

            private slots:
                void onDataArrived();
                void onDisconnected();

            private:
                QPointer<QTcpSocket> m_connectedSocket;
                QDataStream m_dataStream;

                ConnectionExpiredCallback m_connectionExpiredCallback;
                NewDataReadyToReadCallback m_newDataReadyToReadCallback;

                QByteArray m_rawDataFromSocket;

                std::mutex m_rawDataMutex;
                std::mutex m_callbacksMutex;
            };

} // namespace Challenge
