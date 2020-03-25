#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace Challenge::Communication::Server {

    class ITransportConnection;

    class IHandshake {
    public:
        using Identifier = std::vector<std::byte>;

        virtual ~IHandshake() = default;

        //! Check validness of the handshake
        virtual bool isValid() const = 0;

        //! Returns handshake id
        virtual const Identifier& identifier() const = 0;


        //! Returns connection on which the handshake was made
        virtual ITransportConnection& connection() const = 0;

        //! Factory method, must be implemented in shared library together with handshake execution process
        /*!
         * Creates IHandshake - starts handshake process
         * @param _connection valid transport layer connection is required to start handshake
         * @return return pointer to IHandshake object, or nullptr in case of handshake process will fail
         */
        template<typename... _Args>
        static std::shared_ptr<IHandshake> start( std::shared_ptr<ITransportConnection> _connection, _Args...);
    };

} // namespace Challenge::Communication::Server

