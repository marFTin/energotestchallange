cmake_minimum_required(VERSION 3.10.2)

SET( SOURCES
        TcpConnection.h
        TcpConnection.cpp
)

SET( PROJECT_ID Client.TcpTransportConnection )

ADD_LIBRARY(${PROJECT_ID} SHARED ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT_ID} Lib.QtTcpConnectionHelper ${Qt5Network_LIBRARIES} stdc++fs)

INSTALL( TARGETS ${PROJECT_ID} LIBRARY DESTINATION /usr/lib)