#pragma once

#include "Communication/Client/TransportConnectivityManager/ITransportConnection.h"

#include <gmock/gmock.h>

namespace Challenge::Communication::Client::Mock {

    class ITransportConnection : public Client::ITransportConnection {
    public:
        MOCK_CONST_METHOD0(isValid, bool() );
        MOCK_METHOD1(registerConnectionExpiredCallback, bool(ConnectionExpiredCallback));
        MOCK_METHOD1(registerNewDataReadyToReadCallback, bool(NewDataReadyToReadCallback));
        MOCK_METHOD0(receive, std::optional<Payload>() );
        MOCK_METHOD1(send, std::optional<uint32_t>(const Payload&) );
    };

} // namespace Challenge::Communication::Client::Mock

