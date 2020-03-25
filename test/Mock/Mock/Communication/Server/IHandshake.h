#pragma once

#include "Communication/Server/IHandshake.h"
#include "Communication/Server/TransportConnectivityManager/ITransportConnection.h"


#include <gmock/gmock.h>

namespace Challenge::Communication::Server::Mock {


    class HandshakeStartMethodMock {
    public:
        MOCK_METHOD0(start, std::shared_ptr<Server::IHandshake>() );
    };

    class IHandshake : public Server::IHandshake {
    public:
        MOCK_CONST_METHOD0( identifier, const Identifier&() );
        MOCK_CONST_METHOD0( isValid,  bool() );
        MOCK_CONST_METHOD0( connection, Server::ITransportConnection&() );

        static std::shared_ptr<HandshakeStartMethodMock> getStartMock();

    private:
        static std::weak_ptr<HandshakeStartMethodMock> ms_startMethodMock;
    };

    inline std::shared_ptr<HandshakeStartMethodMock> IHandshake::getStartMock() {
        auto startMock = ms_startMethodMock.lock();
        if ( startMock ) {
            return startMock;
        }
        startMock.reset( new HandshakeStartMethodMock );
        ms_startMethodMock = startMock;
        return startMock;
    }
} // namespace Challenge::Communication::Server::Mock

namespace Challenge::Communication::Server {
    template<typename... _Args>
    std::shared_ptr<IHandshake> IHandshake::start([[maybe_unused]] std::shared_ptr<ITransportConnection> _connection, _Args...) {
        return Challenge::Communication::Server::Mock::IHandshake::getStartMock()->start();
    }
} // namespace Challenge::Communication::Server