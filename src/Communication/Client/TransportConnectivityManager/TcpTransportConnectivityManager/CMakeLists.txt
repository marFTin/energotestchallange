cmake_minimum_required(VERSION 3.10.2)

SET( SOURCES
        TcpConnectivityManager.cpp
        TcpConnectivityManager.h
)

SET( PROJECT_ID Client.TcpTransportConnectivityManager )

ADD_LIBRARY(${PROJECT_ID} SHARED ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT_ID} ${Qt5Network_LIBRARIES} stdc++fs)

INSTALL( TARGETS ${PROJECT_ID} LIBRARY DESTINATION /usr/lib)