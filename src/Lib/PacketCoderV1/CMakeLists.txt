cmake_minimum_required(VERSION 3.10.2)

SET( SOURCES PacketFactory.cpp PacketDecoder.cpp BytesStream.cpp)

SET( PROJECT_ID Lib.PacketCoderV1 )

ADD_LIBRARY(${PROJECT_ID} STATIC ${SOURCES})
