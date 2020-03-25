cmake_minimum_required(VERSION 3.10.2)



SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)

SET( SOURCES uic/MainWindow.ui Main.cpp MainWindow.cpp )

INCLUDE_DIRECTORIES(include)

SET( APPLICATION_TARGET challenge.application)
ADD_EXECUTABLE( ${APPLICATION_TARGET} ${SOURCES})

TARGET_LINK_LIBRARIES(${APPLICATION_TARGET}
        Client.TcpTransportConnectivityManager
        Client.TcpTransportConnection
        Client.HandshakeV1
        Client.ProtocolExecutorV1
        Lib.TableEventsModel
        ${Qt5Widgets_LIBRARIES}
)

INSTALL( TARGETS ${APPLICATION_TARGET} RUNTIME DESTINATION /usr/local/bin)
