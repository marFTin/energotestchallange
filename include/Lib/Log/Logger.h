#pragma once

#include <iostream>
#include <syslog.h>
#include <type_traits>

#define LOG_ERROR( _text ) syslog( LOG_ERR, "%s", _text );

#define LOG_INFORMATION( _text )syslog( LOG_INFO, "%s", _text );
