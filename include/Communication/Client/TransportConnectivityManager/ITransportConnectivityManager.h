#pragma once

#include <memory>

namespace Challenge::Communication::Client {

    class ITransportConnection;

    class ITransportConnectivityManager {
    public:
        virtual ~ITransportConnectivityManager() = default;

        //! Factory method, must be implemented in shared library together with class implementation
        static std::shared_ptr<ITransportConnectivityManager> create();

        //! Starts connection with server
        /*!
         *
         * @return pointer to ITransportConnection object if connection was established, or nullptr in case
         * of connection cannot be done.
         */
        virtual std::shared_ptr<ITransportConnection> connectToServer() const = 0;
    };
} // namespace Challenge::Communication::Client

