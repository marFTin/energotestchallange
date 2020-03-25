#include "Lib/TableEventsModel/TableEventsModel.h"

#include "Mock/Communication/Client/IAppProtocol.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <future>
#include <memory>

using namespace Challenge;
using namespace testing;

TEST( TableEventsModel, throwConstructor ) {
    ASSERT_THROW( TableEventsModel unitUnderTest( nullptr ) , std::runtime_error );
}

TEST( TableEventsModel, numberOfColumns ) {
    auto protocolExecutorMock = std::make_shared< NiceMock<Communication::Client::Mock::IProtocolExecutor> >();

    TableEventsModel unitUnderTest( protocolExecutorMock );

    ASSERT_EQ( unitUnderTest.columnCount(), 3 );
}



