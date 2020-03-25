#pragma once

#include <functional>
#include <memory>

namespace Challenge {
namespace Communication {
namespace Server {

            class ITransportConnection;

            class ITransportConnectivityManager {
            public:
                using NewConnectionCallback = std::function<void(std::shared_ptr<ITransportConnection>)>;

                //! Factory method to implement in the shared library
                static std::unique_ptr<ITransportConnectivityManager> create();

                virtual ~ITransportConnectivityManager() = default;

                //! When new transport connection being established the object will fire a registered callback
                /*!
                 *  Only one callback can be registered at time, new callback will overwrite old, already registered. The project
                 *  does not required to connect to more than one server in the same time, moreover it is possible to implement some
                 *  callback proxy to fire set of callbacks
                 * @param _callback callback to be invoked
                 * @return true if previous callback was overwritten
                 */
                virtual bool registerNewConnectionCallback(NewConnectionCallback _callback) = 0;
            };

} // namespace Communication
} // namespace Server
} // namespace Challenge
