#pragma once

#include <functional>
#include <memory>

namespace Challenge::EventsStorage {
    class IEventsStorage;
} // namespace Challenge::EventsStorage::IEventsStorage

namespace Challenge::Communication::Server {

    class IHandshake;

    class IProtocolExecutor {
    public:
        virtual bool isValid() const = 0;

        //! Factory method
        /*!
         *
         * @tparam _Args additional parameters required by implementation
         * @param _handshake handshake
         * @param _storage storage
         * @param ... additional parameters
         * @return nullptr in case of error, otherwise pointer to object
         */
        template<typename... _Args>
        static std::shared_ptr<IProtocolExecutor> create(std::shared_ptr<IHandshake> _handshake, std::shared_ptr<Challenge::EventsStorage::IEventsStorage> _storage, _Args... );

    };

} // namespace Challenge::Communication::Server