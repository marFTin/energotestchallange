#include <QCoreApplication>

#include "Server.h"

#include "Lib/Log/Logger.h"
#include "Lib/C++Tools/ScopedAction.h"

int32_t  main( int32_t _argc, char** _argv) try {

    openlog( "CHALLENGE_SERVER", LOG_NDELAY | LOG_PID, LOG_DAEMON);

    Challenge::ScopedAction scopedAction( []{closelog();} );

    QCoreApplication application(_argc, _argv);

    Challenge::Communication::Server::Server server;

    return QCoreApplication::exec();
} catch ( std::exception& _exception ) {
    LOG_ERROR( _exception.what() );
    return -1;
} catch (...) {
    LOG_ERROR( "Unhandled unknown exception" );
    return -1;
}