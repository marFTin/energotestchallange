#pragma once

#include "Communication/Client/IProtocolExecutor.h"

#include <gmock/gmock.h>

namespace Challenge::Communication::Client::Mock {
    class IProtocolExecutor : public Challenge::Communication::Client::IProtocolExecutor{
    public:
        MOCK_METHOD2( sendEvent, bool(const std::string&, uint32_t) );
        MOCK_METHOD1( registerNewEventAddedCallback, bool(Challenge::Communication::Client::IProtocolExecutor::NewEventAddedCallback) ) ;
        MOCK_METHOD2( getSavedEvents, std::optional<Events>(uint64_t, uint64_t) );
        MOCK_METHOD0( getNumberOfSavedEvents, std::optional<uint64_t>() );
    };
}
