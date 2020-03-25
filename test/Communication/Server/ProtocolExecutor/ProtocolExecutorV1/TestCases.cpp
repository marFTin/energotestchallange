#include "ProtocolExecutorV1.h"

#include "Mock/EventsStorage/IEventsStorage.h"
#include "Mock/Communication/Server/ITransportConnection.h"
#include "Mock/Communication/Server/IHandshake.h"

#include "Lib/PacketCoderV1/PacketFactory.h"

#include "Lib/TestUtils/CallbackCatcher.h"

#include "Event/EventData.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace Challenge::Communication::Server;

template std::shared_ptr<Challenge::EventsStorage::IEventsStorage> Challenge::EventsStorage::IEventsStorage::create();
template std::shared_ptr<ITransportConnection> ITransportConnection::create();

#define RETURN_CONNECTION(_connection) testing::ReturnRef(_connection)
#define RETURN_STORAGE(_storage) testing::Return<std::shared_ptr<Challenge::EventsStorage::IEventsStorage>>( _storage )
#define RETURN_PAYLOAD(_payload) testing::Return<std::optional<ITransportConnection::Payload>>(_payload)


using ConnectionExpiredCallback = std::function<void(void)>;
using NewDataReadyToReadCallback = std::function<void(void)>;


class ProtocolExecutorV1Test : public ::testing::Test {
public:
    ProtocolExecutorV1Test() : m_handshakeIdentifire( Challenge::PacketCoderV1::handshakeIdToByteVector( HandshakeId )) {
    }

    void SetUp() override {
        m_handshakeMock = std::make_shared< Mock::IHandshake >();
        m_connectionMock = std::make_shared< Mock::ITransportConnection >();
        m_storageMock = std::make_shared< Challenge::EventsStorage::Mock::IEventsStorage>();



        EXPECT_CALL( *getHandshakeMock(), identifier )
                .WillRepeatedly(testing::ReturnRef(m_handshakeIdentifire));
    }

    void TearDown() override {
        m_handshakeMock.reset();
        m_connectionMock.reset();
        m_storageMock.reset();
    }

    std::shared_ptr<Mock::IHandshake> getHandshakeMock() { return m_handshakeMock; }
    std::shared_ptr<Mock::ITransportConnection> getConnectionMock() { return m_connectionMock; }
    std::shared_ptr<Challenge::EventsStorage::Mock::IEventsStorage> getStorageMock() { return m_storageMock; }

    static constexpr uint32_t HandshakeId = 13;

private:
    std::shared_ptr<Mock::IHandshake> m_handshakeMock;
    std::shared_ptr< Mock::ITransportConnection > m_connectionMock;
    std::shared_ptr<Challenge::EventsStorage::Mock::StorageFactoryMethodMock> m_storageFactoryMock;
    std::shared_ptr<Challenge::EventsStorage::Mock::IEventsStorage> m_storageMock;

    IHandshake::Identifier m_handshakeIdentifire;


};


TEST_F( ProtocolExecutorV1Test, constructionFailed ) {
    ASSERT_THROW(ProtocolExecutorV1(nullptr, getStorageMock()), std::runtime_error );

    EXPECT_CALL( *getHandshakeMock(), connection )
        .WillRepeatedly(RETURN_CONNECTION( *getConnectionMock() ));

    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(false));

    // no connection
    ASSERT_THROW(ProtocolExecutorV1 appProtocol(getHandshakeMock(), getStorageMock()), std::runtime_error );

    // no storage
    ASSERT_THROW(ProtocolExecutorV1 appProtocol(getHandshakeMock(), nullptr), std::runtime_error );
}

TEST_F( ProtocolExecutorV1Test, constructionSuccess ) {
    using namespace testing;

    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));

    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));

    EXPECT_CALL( *getConnectionMock(), registerNewDataReadyToReadCallback(testing::_) ).Times(2);
    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _)).Times(2);

    ASSERT_NO_THROW(ProtocolExecutorV1 appProtocol(getHandshakeMock(), getStorageMock()) );
}

TEST_F( ProtocolExecutorV1Test, newEventReceiveAndSaved ) {
    using namespace testing;
    using namespace testing;
    const std::string eventText = "new event";
    const uint32_t eventPriority = 4;

    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));

    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));

    Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback > newDataCallback;
    // catch new data callback
    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(_))
            .Times(2)
            .WillRepeatedly(testing::Invoke(&newDataCallback, &Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback>::registerCallback));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto newEventPayload = packetFactory.createSendEvent( 3,HandshakeId, eventText, eventPriority).value();
    auto ackPayload = packetFactory.createAck( 3, HandshakeId );

    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL( *getStorageMock(), saveEvent(
            AllOf(
                      Field(&Challenge::EventData::text, eventText )
                    , Field(&Challenge::EventData::priority, eventPriority )
                    ))
        )
        .WillOnce(testing::Return(true));

    EXPECT_CALL(*getConnectionMock(), send(ackPayload))
        .WillOnce(testing::Return(true)); // save ack

    // returns new event
    EXPECT_CALL(*getConnectionMock(), receive())
            .WillOnce(RETURN_PAYLOAD(std::nullopt))
            .WillOnce(RETURN_PAYLOAD(ITransportConnection::Payload{}))
            .WillOnce(RETURN_PAYLOAD(newEventPayload))
            .WillRepeatedly(RETURN_PAYLOAD(std::nullopt));

    {
            ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());
            // std::nullopt
            newDataCallback.fireCallback();
            // empty payload
            newDataCallback.fireCallback();
            // new event
            newDataCallback.fireCallback();
    }

    // check if protocol unregister its callback
    ASSERT_FALSE( newDataCallback.isValid() );
}

TEST_F( ProtocolExecutorV1Test, newEventReceiveAndCannotBeSaved ) {
    using namespace testing;
    const std::string eventText = "new event";
    const uint32_t eventPriority = 4;

    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));

    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));

    Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback > newDataCallback;
    // catch new data callback
    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(testing::_))
            .Times(2)
            .WillRepeatedly(testing::Invoke(&newDataCallback, &Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback>::registerCallback));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto newEventPayload = packetFactory.createSendEvent( 3,HandshakeId, eventText, eventPriority).value();

    EXPECT_CALL( *getStorageMock(), saveEvent(_))
            .WillOnce(testing::Return(false));

    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _)).WillRepeatedly(Return(true));

    EXPECT_CALL(*getConnectionMock(), send(testing::_))
    .Times(0); // no ACK was sent

    // returns new event
    EXPECT_CALL(*getConnectionMock(), receive())
            .WillOnce(RETURN_PAYLOAD(newEventPayload))
            .WillRepeatedly(RETURN_PAYLOAD(std::nullopt));

    {
        ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());
        // new event
        newDataCallback.fireCallback();
    }

    // check if protocol unregister its callback
    ASSERT_FALSE( newDataCallback.isValid() );
}

TEST_F( ProtocolExecutorV1Test, newEventReceiveWrongHandshakeId ) {
    using namespace testing;
    const std::string eventText = "new event";
    const uint32_t eventPriority = 4;

    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _)).WillRepeatedly(Return(true));

    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));

    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));

    Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback > newDataCallback;
    // catch new data callback
    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(testing::_))
            .Times(2)
            .WillRepeatedly(testing::Invoke(&newDataCallback, &Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback>::registerCallback));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto newEventPayload = packetFactory.createSendEvent( 3,HandshakeId + 1, eventText, eventPriority).value();

    EXPECT_CALL( *getStorageMock(), saveEvent(_)).Times(0);
    EXPECT_CALL(*getConnectionMock(), send(testing::_)).Times(0); // no ACK was sent

    // returns new event
    EXPECT_CALL(*getConnectionMock(), receive())
            .WillOnce(RETURN_PAYLOAD(newEventPayload))
            .WillRepeatedly(RETURN_PAYLOAD(std::nullopt));

    {
        ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());
        // new event
        newDataCallback.fireCallback();
    }

    // check if protocol unregister its callback
    ASSERT_FALSE( newDataCallback.isValid() );
}

TEST_F( ProtocolExecutorV1Test, numberOfSavedEventsRequest ) {
    using namespace testing;

    constexpr uint64_t numberOfSavedEvents = 7ul;

    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _)).WillRepeatedly(Return(true));

    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));

    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));

    Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback > newDataCallback;
    // catch new data callback
    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(_))
            .Times(2)
            .WillRepeatedly(testing::Invoke(&newDataCallback, &Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback>::registerCallback));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto requestPayload = packetFactory.createNumberOfEventsRequest( 3,HandshakeId);
    auto responsePayload = packetFactory.createNumberOfEventsResponse(3, HandshakeId, numberOfSavedEvents );

    EXPECT_CALL( *getStorageMock(), saveEvent(_)).Times(0);
    EXPECT_CALL( *getStorageMock(), getNumberOfEvents)
        .Times(1)
        .WillOnce(Return(numberOfSavedEvents));

    EXPECT_CALL(*getConnectionMock(), send(responsePayload))
            .WillOnce(testing::Return(true)); // save ack

    // returns new event
    EXPECT_CALL(*getConnectionMock(), receive())
            .WillOnce(RETURN_PAYLOAD(requestPayload))
            .WillRepeatedly(RETURN_PAYLOAD(std::nullopt));

    {
        ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());
        // new request
        newDataCallback.fireCallback();
    }

    // check if protocol unregister its callback
    ASSERT_FALSE( newDataCallback.isValid() );
}

TEST_F( ProtocolExecutorV1Test, numberOfSavedEventsRequestStorageError ) {
    using namespace testing;

    constexpr uint32_t handshakeId = 13;

    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));
    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _)).WillRepeatedly(Return(true));

    Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback > newDataCallback;
    // catch new data callback
    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(_))
            .Times(2)
            .WillRepeatedly(testing::Invoke(&newDataCallback, &Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback>::registerCallback));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto requestPayload = packetFactory.createNumberOfEventsRequest( 3,handshakeId);

    EXPECT_CALL( *getStorageMock(), saveEvent(_)).Times(0);
    EXPECT_CALL( *getStorageMock(), getNumberOfEvents)
            .Times(1)
            .WillOnce(Return(std::nullopt));

    EXPECT_CALL(*getConnectionMock(), send(_)).Times(0);

    // returns new event
    EXPECT_CALL(*getConnectionMock(), receive())
            .WillOnce(RETURN_PAYLOAD(requestPayload))
            .WillRepeatedly(RETURN_PAYLOAD(std::nullopt));

    {
        ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());
        // new request
        newDataCallback.fireCallback();
    }

    // check if protocol unregister its callback
    ASSERT_FALSE( newDataCallback.isValid() );
}

TEST_F( ProtocolExecutorV1Test, numberOfSavedEventsRequestWrongHandshakeId ) {
    using namespace testing;

    constexpr uint32_t handshakeId = 13;

    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));
    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _)).WillRepeatedly(Return(true));

    Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback > newDataCallback;
    // catch new data callback
    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(_))
            .Times(2)
            .WillRepeatedly(testing::Invoke(&newDataCallback, &Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback>::registerCallback));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto requestPayload = packetFactory.createNumberOfEventsRequest( 3,HandshakeId + 1);

    EXPECT_CALL( *getStorageMock(), getNumberOfEvents).Times(0);
    EXPECT_CALL(*getConnectionMock(), send(_)).Times(0);

    // returns new event
    EXPECT_CALL(*getConnectionMock(), receive())
            .WillOnce(RETURN_PAYLOAD(requestPayload))
            .WillRepeatedly(RETURN_PAYLOAD(std::nullopt));

    {
        ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());
        // new request
        newDataCallback.fireCallback();
    }

    // check if protocol unregister its callback
    ASSERT_FALSE( newDataCallback.isValid() );
}


TEST_F( ProtocolExecutorV1Test, savedEventsRangeRequest ) {
    using namespace testing;

    constexpr uint64_t numberOfSavedEvents = 7ul;

    auto timeStamp = std::chrono::system_clock::now();
    Challenge::EventData event1{ timeStamp, "A", 1 };
    Challenge::EventData event2{ timeStamp, "B", 2 };
    Challenge::EventData event3{ timeStamp, "C", 3 };

    Challenge::EventsStorage::IEventsStorage::Events  storageEventsAll = {event1, event2, event3 };
    Challenge::EventsStorage::IEventsStorage::Events  storageEvents2 = {event1, event2, event3 };


    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));
    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));

    Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback > newDataCallback;
    // catch new data callback
    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(_))
            .Times(2)
            .WillRepeatedly(testing::Invoke(&newDataCallback, &Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback>::registerCallback));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto requestPayload = packetFactory.createSavedEventsRequest(3, HandshakeId, 0,2);
    auto responsePayload1 = packetFactory.createSavedEventsResponse(
            3, HandshakeId, false
            , std::chrono::duration_cast<std::chrono::milliseconds>( event1.timeStamp.time_since_epoch()).count()
            , event1.priority
            , event1.text ).value();
    auto responsePayload2 = packetFactory.createSavedEventsResponse(
            3, HandshakeId, false
            , std::chrono::duration_cast<std::chrono::milliseconds>( event2.timeStamp.time_since_epoch()).count()
            , event2.priority
            , event2.text ).value();
    auto responsePayload3 = packetFactory.createSavedEventsResponse(
            3, HandshakeId, true
            , std::chrono::duration_cast<std::chrono::milliseconds>( event3.timeStamp.time_since_epoch()).count()
            , event3.priority
            , event3.text ).value();

    EXPECT_CALL( *getStorageMock(), getSavedEvents(0,2))
            .Times(1)
            .WillOnce(Return(storageEventsAll));
    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _)).WillRepeatedly(Return(true));

    EXPECT_CALL(*getConnectionMock(), send(responsePayload1))
            .WillOnce(testing::Return(responsePayload1.size()));
    EXPECT_CALL(*getConnectionMock(), send(responsePayload2))
            .WillOnce(testing::Return(responsePayload2.size()));
    EXPECT_CALL(*getConnectionMock(), send(responsePayload3))
            .WillOnce(testing::Return(responsePayload3.size()));

    // returns new event
    EXPECT_CALL(*getConnectionMock(), receive())
            .WillOnce(RETURN_PAYLOAD(requestPayload))
            .WillRepeatedly(RETURN_PAYLOAD(std::nullopt));

    {
        ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());
        // new request
        newDataCallback.fireCallback();
    }

    // check if protocol unregister its callback
    ASSERT_FALSE( newDataCallback.isValid() );
}

TEST_F( ProtocolExecutorV1Test, savedEventsRangeRequestStorageError ) {
    using namespace testing;

    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));
    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));

    Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback > newDataCallback;
    // catch new data callback
    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(_))
            .Times(2)
            .WillRepeatedly(testing::Invoke(&newDataCallback, &Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback>::registerCallback));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto requestPayload = packetFactory.createSavedEventsRequest(3, HandshakeId, 0,2);

    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL( *getStorageMock(), getSavedEvents(0,2))
            .Times(1)
            .WillOnce(Return(std::nullopt));

    EXPECT_CALL(*getConnectionMock(), send(_)).Times(0);

    // returns new event
    EXPECT_CALL(*getConnectionMock(), receive())
            .WillOnce(RETURN_PAYLOAD(requestPayload))
            .WillRepeatedly(RETURN_PAYLOAD(std::nullopt));

    {
        ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());
        // new request
        newDataCallback.fireCallback();
    }

    // check if protocol unregister its callback
    ASSERT_FALSE( newDataCallback.isValid() );
}

TEST_F( ProtocolExecutorV1Test, savedEventsRangeRequestWrongHandshakeId ) {
    using namespace testing;

    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));
    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));

    Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback > newDataCallback;
    // catch new data callback
    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(_))
            .Times(2)
            .WillRepeatedly(testing::Invoke(&newDataCallback, &Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback>::registerCallback));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto requestPayload = packetFactory.createSavedEventsRequest(3, HandshakeId + 1, 0,2);

    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL( *getStorageMock(), getSavedEvents(_,_)).Times(0);

    EXPECT_CALL(*getConnectionMock(), send(_)).Times(0);

    // returns new event
    EXPECT_CALL(*getConnectionMock(), receive())
            .WillOnce(RETURN_PAYLOAD(requestPayload))
            .WillRepeatedly(RETURN_PAYLOAD(std::nullopt));

    {
        ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());
        // new request
        newDataCallback.fireCallback();
    }

    // check if protocol unregister its callback
    ASSERT_FALSE( newDataCallback.isValid() );
}

TEST_F( ProtocolExecutorV1Test, savedEventsRangeRequestErrorDuringSendInSequence ) {
    using namespace testing;

    constexpr uint64_t numberOfSavedEvents = 7ul;

    auto timeStamp = std::chrono::system_clock::now();
    Challenge::EventData event1{ timeStamp, "A", 1 };
    Challenge::EventData event2{ timeStamp, "B", 2 };
    Challenge::EventData event3{ timeStamp, "C", 3 };

    Challenge::EventsStorage::IEventsStorage::Events  storageEventsAll = {event1, event2, event3 };


    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));
    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));

    Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback > newDataCallback;
    // catch new data callback
    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(_))
            .Times(2)
            .WillRepeatedly(testing::Invoke(&newDataCallback, &Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback>::registerCallback));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto requestPayload = packetFactory.createSavedEventsRequest(3, HandshakeId, 0,2);
    auto responsePayload1 = packetFactory.createSavedEventsResponse(
            3, HandshakeId, false
            , std::chrono::duration_cast<std::chrono::milliseconds>( event1.timeStamp.time_since_epoch()).count()
            , event1.priority
            , event1.text ).value();
    auto responsePayload2 = packetFactory.createSavedEventsResponse(
            3, HandshakeId, false
            , std::chrono::duration_cast<std::chrono::milliseconds>( event2.timeStamp.time_since_epoch()).count()
            , event2.priority
            , event2.text ).value();
    auto responsePayload3 = packetFactory.createSavedEventsResponse(
            3, HandshakeId, true
            , std::chrono::duration_cast<std::chrono::milliseconds>( event3.timeStamp.time_since_epoch()).count()
            , event3.priority
            , event3.text ).value();

    EXPECT_CALL( *getStorageMock(), getSavedEvents(0,2))
            .Times(1)
            .WillOnce(Return(storageEventsAll));
    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _)).WillRepeatedly(Return(true));

    EXPECT_CALL(*getConnectionMock(), send(responsePayload1))
            .WillOnce(testing::Return(responsePayload1.size()));
    EXPECT_CALL(*getConnectionMock(), send(responsePayload2))
            .WillOnce(testing::Return(std::nullopt)); // error
    EXPECT_CALL(*getConnectionMock(), send(responsePayload3)).Times(0);

    // returns new event
    EXPECT_CALL(*getConnectionMock(), receive())
            .WillOnce(RETURN_PAYLOAD(requestPayload))
            .WillRepeatedly(RETURN_PAYLOAD(std::nullopt));

    {
        ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());
        // new request
        newDataCallback.fireCallback();
    }

    // check if protocol unregister its callback
    ASSERT_FALSE( newDataCallback.isValid() );
}

TEST_F( ProtocolExecutorV1Test, newEventNotification ) {
    using namespace testing;

    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));
    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillRepeatedly(testing::Return(true));

    Challenge::Tests::CallbackArgument<Challenge::EventsStorage::IEventsStorage::EventSavedCallback> newEventCallback;
    // catch new data callback
    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _))
            .Times(2)
            .WillRepeatedly(testing::Invoke(&newEventCallback, &Challenge::Tests::CallbackArgument<Challenge::EventsStorage::IEventsStorage::EventSavedCallback>::registerCallback));

    EXPECT_CALL(*getStorageMock(), getNumberOfEvents).Times(1).WillOnce(Return(17));

    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(_)).Times(2);

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto notificationPayload = packetFactory.createNewEventsNotification(HandshakeId,17);

    EXPECT_CALL( *getConnectionMock(), send(notificationPayload) ).Times(1);

    {
            ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());
            // new request
            newEventCallback.fireCallback();
    }
}

TEST_F( ProtocolExecutorV1Test, isValid ) {
    using namespace testing;

    EXPECT_CALL( *getHandshakeMock(), connection )
            .WillRepeatedly(RETURN_CONNECTION(*getConnectionMock()));

    EXPECT_CALL( *getHandshakeMock(), isValid )
            .WillOnce(Return(true))
            .WillOnce(Return(false))
            .WillRepeatedly(Return(true));

    // catch new data callback
    EXPECT_CALL(*getStorageMock(), registerEventAddedCallback(_, _))
            .Times(2);

    EXPECT_CALL(*getConnectionMock(), registerNewDataReadyToReadCallback(_)).Times(2);


    {
        ProtocolExecutorV1 unitUnderTest(getHandshakeMock(), getStorageMock());

        ASSERT_FALSE( unitUnderTest.isValid() );
        ASSERT_TRUE( unitUnderTest.isValid() );
    }
}






