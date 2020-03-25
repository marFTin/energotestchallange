#pragma once

#include "Communication/Client/TransportConnectivityManager/ITransportConnectivityManager.h"

namespace Challenge::Communication::Client::Mock {

    class ITransportConnectivityManager : public Client::ITransportConnectivityManager {
    public:
        MOCK_CONST_METHOD0( connectToServer,  std::shared_ptr<Client::ITransportConnection>() );
    };

} // namespace Challenge::Communication::Client::Mock

