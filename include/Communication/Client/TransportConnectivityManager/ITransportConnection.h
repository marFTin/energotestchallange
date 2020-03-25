#pragma once

#include <cstddef>
#include <cinttypes>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace Challenge::Communication::Client {

    //! Class represents once established connection
    class ITransportConnection {
    public:
        using ConnectionExpiredCallback = std::function<void(void)>;
        using NewDataReadyToReadCallback = std::function<void(void)>;
        using Payload = std::vector<std::byte>;
        virtual ~ITransportConnection() = default;

        //! Factory method
        /*!
         * Connection and ConnectivityManager are coupled so much, that may be implemented in the same library, but
         * this will reduce testability. I decided to move checking correctness of construction TransportConnection
         * to linker. The implementation must deliver the template definition together with its explicit instatiation.
         * @tparam _Args
         * @param ...
         * @return
         */
        template<typename... _Args>
        static std::shared_ptr<ITransportConnection> create(_Args...);

        //! After some time from its creation, connection may expired
        /*!
         *
         * @return true is connection is still  pending
         */
        virtual bool isValid() const = 0;

        //! When connection expired the object will fire a registered callback
        /*!
         *  Only one callback can be registered at time, new callback will overwrite old, already registered. The project
         *  does not required to connect to more than one server in the same time, moreover it is possible to implement some
         *  callback proxy to fire set of callbacks
         * @param _callback callback to be invoked
         * @return true if previous callback was overwritten
         */
        virtual bool registerConnectionExpiredCallback(ConnectionExpiredCallback _callback) = 0;


        //! Register callback for new data ready to get
        /*!
         *  Only one callback can be registered at time, new callback will overwrite old, already registered. The project
         *  does not required to connect to more than one server in the same time, moreover it is possible to implement some
         *  callback proxy to fire set of callbacks
         * @param _callback callback to be invoked
         * @return true if previous callback was overwritten
         */
        virtual bool registerNewDataReadyToReadCallback(NewDataReadyToReadCallback _callback) = 0;

        virtual std::optional<Payload> receive() = 0;
        virtual std::optional<uint32_t> send( const Payload& _payload ) = 0;
    };
} // namespace Challenge::Communication::Client

