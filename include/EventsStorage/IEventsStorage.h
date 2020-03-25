#pragma once

#include "Event/EventData.h"

#include <cinttypes>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

namespace Challenge::EventsStorage {

    class IEventsStorage {
        public:
            using Events = std::vector<EventData>;
            using EventSavedCallback = std::function<void()>;
            static constexpr uint64_t FIRST_EVENT_NUMBER = 0;
            static constexpr uint64_t LAST_EVENT_NUMBER = std::numeric_limits<uint64_t>::max();

            virtual ~IEventsStorage() = default;

            //! Factory method, must be implemented in shared library
            /*!
             *
             * @return nullptr in case if fail
             */
             template<typename... _Args>
            static std::shared_ptr<IEventsStorage> create(_Args...);

            //! Saves events to storage
            /*!
             *
             * @param _event event to save
             * @return true when event was saved
             */
            virtual bool saveEvent( const EventData& _event ) = 0;

            //! Saves events to storage
            /*!
             *
             * @param _firstEvent start range of events
             * @param _lastEvent eend range of events
             * @return if is some error then return std::nullopt, otherwise list of events
             */
            virtual std::optional<Events> getSavedEvents(uint64_t _firstEvent, uint64_t _lastEvent) const = 0;

            //! Get total naumber of saved events
            /*!
             *
             * @return nullopt in case of error, or number of events
             */
            virtual std::optional<uint64_t> getNumberOfEvents() const = 0;

            //! Register callback for new event saved
            /*!
             *  Callback will be fired when new event is saved
             * @param _callback function to invoke, or nullptr when callback for given key has to be removed
             * @param _key index of callback, it is used to distinguish betwwen subsribed callbacks, the best to use this
             * of registed class
             * @return true in case when callback override alreade registered callback
             */
            virtual bool registerEventAddedCallback( EventSavedCallback _callback, void* _key ) = 0;
    };

} // namespace Challenge::EventsStorage
