cmake_minimum_required(VERSION 3.10.2)

SET( SOURCES
        ProtocolExecutorV1.cpp
        ServerMessagesContainer.cpp
)

SET( PROJECT_ID Client.ProtocolExecutorV1 )

ADD_LIBRARY(${PROJECT_ID} SHARED ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT_ID} Lib.PacketCoderV1 stdc++fs)

INSTALL( TARGETS ${PROJECT_ID} LIBRARY DESTINATION /usr/lib)