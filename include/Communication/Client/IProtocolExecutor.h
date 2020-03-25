#pragma once

#include "Event/EventData.h"

#include <cinttypes>
#include <functional>
#include <future>
#include <optional>
#include <vector>

namespace Challenge::Communication::Client {

    class IHandshake;

    class IProtocolExecutor {
    public:
        //! callbac is fired when on server site new event is added
        /*!
         * @param number of events saved
         */
        using NewEventAddedCallback = std::function<void(uint64_t)>;
        using Events = std::vector<EventData>;
        static constexpr uint64_t FIRST_EVENT_NUMBER = 0;
        static constexpr uint64_t LAST_EVENT_NUMBER = std::numeric_limits<uint64_t>::max();

        virtual ~IProtocolExecutor() = default;

        //! Factory method
        /*!
         *
         * @tparam _Args additional parameters required by implementation
         * @param _handshake handshake
         * @param ... additional parameters
         * @return nullptr in case of error, otherwise pointer to object
         */
        template<typename... _Args>
        static std::shared_ptr<IProtocolExecutor> create(std::shared_ptr<IHandshake> _handshake, _Args... );

        //! Sends event to server
        /*!
         *  Sends events to server and returns information if event was delivered and saved
         * @param _eventText according to requirements events contains text
         * @return return future, its depends on implementation if function is synchronous or asynchronous, te results
         * carried by future is true when server confirmed hat event was saved
         */
        virtual bool sendEvent( const std::string& _eventText, uint32_t _priority )  = 0;

        //! Registered callback for new saved events
        /*!
         *
         * @param _callback parameter of callback is a number of already saved events
         * @return returns true if already register callback was overwritten
         */
        virtual bool registerNewEventAddedCallback(NewEventAddedCallback _callback) = 0;


        //! Gets range of saved event
        /*!
         *
         * @param _firstEvent
         * @param _lastEvent
         * @return list of events
         */
        virtual std::optional<Events> getSavedEvents( uint64_t _firstEvent, uint64_t _lastEvent)  = 0;

        //! Gets number of already stored event
        /*!
         *
         * @return future with number of saved events
         */
        virtual std::optional<uint64_t> getNumberOfSavedEvents()  = 0;
    };
} // namespace Challenge::Communication::Client
