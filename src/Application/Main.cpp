#include "MainWindow.h"

#include "Lib/Log/Logger.h"
#include "Lib/C++Tools/ScopedAction.h"

#include <QApplication>

#include <cinttypes>
#include <stdexcept>

#include<syslog.h>

int32_t  main( int32_t _argc, char** _argv) try {

    openlog( "CHALLENGE_APPLICATION", LOG_NDELAY | LOG_PID , LOG_DAEMON);

    Challenge::ScopedAction scopedAction( []{closelog();} );

    QApplication application(_argc, _argv);
    Challenge::Application::MainWindow mainWindow;
    mainWindow.show();

    return QApplication::exec();
} catch ( std::runtime_error _exception ) {
    LOG_ERROR(  _exception.what() );
}
