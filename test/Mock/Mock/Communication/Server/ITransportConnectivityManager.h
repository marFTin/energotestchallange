#pragma once

#include "Communication/Server/TransportConnectivityManager/ITransportConnectivityManager.h"

namespace Challenge::Communication::Server::Mock {

    class TransportConnectivityManagerFactoryMethodMock {
    public:
        MOCK_METHOD0(create, std::unique_ptr<Server::ITransportConnectivityManager>() );
    };

    class ITransportConnectivityManager : public Server::ITransportConnectivityManager {
    public:
        MOCK_CONST_METHOD1( registerNewConnectionCallback,  bool(NewConnectionCallback) );

        static TransportConnectivityManagerFactoryMethodMock& getFactoryMock() { return ms_factoryMock; }

    private:
        static TransportConnectivityManagerFactoryMethodMock ms_factoryMock;
    };

    inline TransportConnectivityManagerFactoryMethodMock ITransportConnectivityManager::ms_factoryMock;


} // namespace Challenge::Communication::Server::Mock

namespace Challenge::Communication::Server {
    inline std::unique_ptr<ITransportConnectivityManager>
    ITransportConnectivityManager::create() {
        return Mock::ITransportConnectivityManager::getFactoryMock().create();
    }
}


