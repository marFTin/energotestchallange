cmake_minimum_required(VERSION 3.10.2)

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)

SET( SOURCES Main.cpp Server.cpp )

INCLUDE_DIRECTORIES(include)

SET( APPLICATION_TARGET challenge.server)
ADD_EXECUTABLE( ${APPLICATION_TARGET} ${SOURCES})

TARGET_LINK_LIBRARIES(${APPLICATION_TARGET}
        Server.TcpTransportConnectivityManager
        Server.TcpTransportConnection
        Server.ProtocolExecutorV1
        Storage.SqliteStorage
        Server.HandshakeV1
        ${Qt5Widgets_LIBRARIES}
)

INSTALL( TARGETS ${APPLICATION_TARGET} RUNTIME DESTINATION /usr/local/bin )
INSTALL( FILES systemd/ChallengeServer.service DESTINATION /lib/systemd/system )
