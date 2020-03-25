#pragma once

#include "Communication/Client/IHandshake.h"

#include <gmock/gmock.h>

namespace Challenge::Communication::Client::Mock {
    class IHandshake : public Challenge::Communication::Client::IHandshake {
    public:
        MOCK_CONST_METHOD0( isValid, bool() );
        MOCK_CONST_METHOD0( identifier, const Identifier&() );
        MOCK_CONST_METHOD0( connection, ITransportConnection&() );
    };
}