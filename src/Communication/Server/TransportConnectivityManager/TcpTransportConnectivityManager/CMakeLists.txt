cmake_minimum_required(VERSION 3.10.2)

SET( SOURCES
        TcpTransportConnectivityManager.cpp
        TcpTransportConnectivityManager.h
)

SET( PROJECT_ID Server.TcpTransportConnectivityManager )

ADD_LIBRARY(${PROJECT_ID} SHARED ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT_ID} ${Qt5Network_LIBRARIES} stdc++fs)

INSTALL( TARGETS ${PROJECT_ID} LIBRARY DESTINATION /usr/lib)