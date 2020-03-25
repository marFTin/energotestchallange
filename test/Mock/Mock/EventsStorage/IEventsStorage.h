#pragma once

#include "EventsStorage/IEventsStorage.h"

#include <gmock/gmock.h>

namespace Challenge::EventsStorage::Mock {

    class StorageFactoryMethodMock {
    public:
        MOCK_METHOD0(create, std::shared_ptr<EventsStorage::IEventsStorage>() );
    };


    class IEventsStorage: public EventsStorage::IEventsStorage {
    public:
        MOCK_METHOD1(saveEvent, bool(const EventData&));
        MOCK_CONST_METHOD2(getSavedEvents, std::optional<Events>(uint64_t, uint64_t));
        MOCK_CONST_METHOD0(getNumberOfEvents, std::optional<uint64_t>() );
        MOCK_METHOD2(registerEventAddedCallback, bool(EventSavedCallback, void*));

        static std::shared_ptr<StorageFactoryMethodMock> getFactoryMock();

    private:
        static std::weak_ptr<StorageFactoryMethodMock> ms_factoryMock;
    };

    inline std::weak_ptr<StorageFactoryMethodMock> IEventsStorage::ms_factoryMock{};

    inline std::shared_ptr<StorageFactoryMethodMock> IEventsStorage::getFactoryMock() {
        auto mock = ms_factoryMock.lock();
        if ( mock ) {
            return mock;
        }

        mock.reset( new  StorageFactoryMethodMock );
        ms_factoryMock = mock;
        return mock;
    }
} // namespace Challenge::EventsStorage::Mock

namespace Challenge::EventsStorage {
    template<typename... _Args>
    inline std::shared_ptr<IEventsStorage> IEventsStorage::create(_Args...) {
        return Mock::IEventsStorage::getFactoryMock()->create();
    }
} // namespace Challenge::EventsStorage
