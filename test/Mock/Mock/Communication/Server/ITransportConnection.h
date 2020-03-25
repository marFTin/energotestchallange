#pragma once

#include "Communication/Server/TransportConnectivityManager/ITransportConnection.h"

#include <gmock/gmock.h>

namespace Challenge::Communication::Server::Mock {

    class TransportConnectionFactoryMethodMock {
    public:
        MOCK_METHOD0(create, std::shared_ptr<Server::ITransportConnection>() );
    };

    class ITransportConnection : public Server::ITransportConnection {
    public:
        ~ITransportConnection() override = default;

        MOCK_CONST_METHOD0(isValid, bool() );
        MOCK_METHOD1(registerConnectionExpiredCallback, bool(ConnectionExpiredCallback _callback));
        MOCK_METHOD0(receive, std::optional<Payload>() );
        MOCK_METHOD1(send, std::optional<uint32_t>(const Payload&) );
        MOCK_METHOD1(registerNewDataReadyToReadCallback, bool(NewDataReadyToReadCallback));

        static std::shared_ptr<TransportConnectionFactoryMethodMock> getFactoryMock();

    private:
        static std::weak_ptr<TransportConnectionFactoryMethodMock> ms_factoryMethodMock;
    };

    inline std::shared_ptr<TransportConnectionFactoryMethodMock> ITransportConnection::getFactoryMock() {
        auto factoryMock = ms_factoryMethodMock.lock();
        if ( factoryMock ) {
            return factoryMock;
        }
        factoryMock.reset( new TransportConnectionFactoryMethodMock );
        ms_factoryMethodMock = factoryMock;
        return factoryMock;
    }

    inline std::weak_ptr<TransportConnectionFactoryMethodMock> ITransportConnection::ms_factoryMethodMock;

} // namespace Challenge::Communication::Server::Mock

namespace Challenge::Communication::Server {
    template<typename... _Args>
    std::shared_ptr<ITransportConnection> ITransportConnection::create(_Args...) {
        return Challenge::Communication::Server::Mock::ITransportConnection::getFactoryMock()->create();
    }
} // namespace Challenge::Communication::Server


