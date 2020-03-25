#include <gtest/gtest.h>

#include "SqliteStorage.h"

#include <experimental/filesystem>


using namespace Challenge::EventsStorage;

class SqliteStorageTest : public ::testing::Test {
public:
    static constexpr auto TEST_DB_PATH =  "/tmp/energotest.db";

    SqliteStorageTest() { std::experimental::filesystem::remove(TEST_DB_PATH); }


    void SetUp() override {
        assert(!m_storage);
        std::experimental::filesystem::remove(TEST_DB_PATH);
        m_storage.reset( new SqliteStorage(TEST_DB_PATH) );
    }

    void TearDown() override {
        assert(m_storage);
        m_storage.reset();
        std::experimental::filesystem::remove(TEST_DB_PATH);
    }

    SqliteStorage& getStorage() { assert(m_storage); return *m_storage; }
private:
    std::shared_ptr< SqliteStorage > m_storage;
};

TEST( SqliteStorageCreation, CreateDatabase ) {
    EXPECT_NO_THROW( SqliteStorage() );

    // relative path will throw
    EXPECT_THROW( SqliteStorage( "../tmp/file.db"), std::runtime_error );

    // unexisted path will throw
    EXPECT_THROW( SqliteStorage( "/unexisted_path/unexisted_path/blalala/tmp/db" ), std::runtime_error );
}

TEST_F( SqliteStorageTest, SaveEvent ) {
    auto timeStamp = std::chrono::system_clock::now();
    Challenge::EventData eventToSave{ timeStamp, "text", 0 };

    auto result = getStorage().saveEvent( eventToSave );
    EXPECT_TRUE( result );
}

TEST_F( SqliteStorageTest, GetNumberOfEvents ) {
    auto zeroEvents = getStorage().getNumberOfEvents();
    ASSERT_TRUE(zeroEvents.has_value());
    ASSERT_EQ(zeroEvents.value(), 0);

    auto timeStamp = std::chrono::system_clock::now();
    Challenge::EventData eventToSave1{ timeStamp, "text1", 0 };
    Challenge::EventData eventToSave2{ timeStamp, "text2", 0 };
    Challenge::EventData eventToSave3{ timeStamp, "text3", 0 };

    getStorage().saveEvent( eventToSave1 );
    getStorage().saveEvent( eventToSave2 );
    getStorage().saveEvent( eventToSave3 );

    auto threeEvents = getStorage().getNumberOfEvents();
    ASSERT_TRUE( threeEvents.has_value() );
    ASSERT_EQ( threeEvents.value(), 3 );
}

TEST_F( SqliteStorageTest, GetSavedEvents ) {
    auto zeroEvents = getStorage().getSavedEvents(IEventsStorage::FIRST_EVENT_NUMBER, IEventsStorage::LAST_EVENT_NUMBER);
    ASSERT_TRUE(zeroEvents.has_value());
    ASSERT_TRUE(zeroEvents.value().empty() );

    auto timeStamp = std::chrono::system_clock::now();
    Challenge::EventData eventToSave1{ timeStamp, "text1", 0 };
    Challenge::EventData eventToSave2{ timeStamp, "text2", 1 };
    Challenge::EventData eventToSave3{ timeStamp, "text3", 2 };

    getStorage().saveEvent( eventToSave1 );
    getStorage().saveEvent( eventToSave2 );
    getStorage().saveEvent( eventToSave3 );

    auto threeEvents = getStorage().getSavedEvents(IEventsStorage::FIRST_EVENT_NUMBER, IEventsStorage::LAST_EVENT_NUMBER);
    ASSERT_TRUE( threeEvents.has_value() );
    ASSERT_EQ( threeEvents.value().size(), 3 );

    auto event0 = getStorage().getSavedEvents(0, 0);
    ASSERT_TRUE( event0.has_value() );
    ASSERT_EQ( event0.value().size(), 1 );
    ASSERT_EQ( event0.value().front().priority, 0 );

    auto event1and1 = getStorage().getSavedEvents(1, 1);
    ASSERT_TRUE( event1and1.has_value() );
    ASSERT_EQ( event1and1.value().size(), 1 );
    ASSERT_EQ( event1and1.value()[0].priority, 1 );

    auto event1and2 = getStorage().getSavedEvents(1, 2);
    ASSERT_TRUE( event1and2.has_value() );
    ASSERT_EQ( event1and2.value().size(), 2 );
    ASSERT_EQ( event1and2.value()[0].priority, 1 );
    ASSERT_EQ( event1and2.value()[1].priority, 2 );

    auto wrongRange = getStorage().getSavedEvents(3, 1);
    ASSERT_FALSE( wrongRange.has_value() );
}

TEST_F( SqliteStorageTest, savedEventsCallback ) {
    auto zeroEvents = getStorage().getNumberOfEvents();
    ASSERT_TRUE(zeroEvents.has_value());
    ASSERT_EQ(zeroEvents.value(), 0);

    auto timeStamp = std::chrono::system_clock::now();
    Challenge::EventData eventToSave1{ timeStamp, "text1", 0 };
    Challenge::EventData eventToSave2{ timeStamp, "text2", 0 };
    Challenge::EventData eventToSave3{ timeStamp, "text3", 0 };

    auto callback1FireCounter = 0;
    auto callback2FireCounter = 0;
    auto newEventSavedCallback1 = [&callback1FireCounter](){ ++callback1FireCounter; };
    auto newEventSavedCallback2 = [&callback2FireCounter](){ ++callback2FireCounter; };

    getStorage().registerEventAddedCallback(newEventSavedCallback1, this);
    getStorage().registerEventAddedCallback(newEventSavedCallback2, nullptr);
    getStorage().saveEvent( eventToSave1 );
    ASSERT_EQ( callback1FireCounter, 1 );
    ASSERT_EQ( callback2FireCounter, 1 );

    getStorage().registerEventAddedCallback(nullptr, nullptr);
    getStorage().saveEvent( eventToSave2 );
    ASSERT_EQ( callback1FireCounter, 2 );
    ASSERT_EQ( callback2FireCounter, 1 );


    getStorage().registerEventAddedCallback(nullptr, this);
    getStorage().saveEvent( eventToSave3 );
    ASSERT_EQ( callback1FireCounter, 2 );
    ASSERT_EQ( callback2FireCounter, 1 );
}