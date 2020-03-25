cmake_minimum_required(VERSION 3.10.2)

SET( SOURCES
        ${CMAKE_SOURCE_DIR}/include/Lib/QtTcpConnectionHelper/QtTcpConnectionHelper.h
        TcpConnection.cpp
)

SET( PROJECT_ID Lib.QtTcpConnectionHelper )

ADD_LIBRARY(${PROJECT_ID} STATIC ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT_ID} ${Qt5Network_LIBRARIES} stdc++fs)