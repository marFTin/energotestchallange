cmake_minimum_required(VERSION 3.10.3)

SET ( TEST_ID Test.Server.TcpTransportConnection )

SET( SOURCES
        Main.cpp
        TestCases.cpp
)

ADD_EXECUTABLE( ${TEST_ID} ${SOURCES})

# includes to unit under test
TARGET_INCLUDE_DIRECTORIES( ${TEST_ID} PRIVATE "${CMAKE_SOURCE_DIR}/src/Communication/Server/TransportConnectivityManager/TcpTransportConnection" )

TARGET_LINK_LIBRARIES( ${TEST_ID} PRIVATE Server.TcpTransportConnection )
TARGET_LINK_LIBRARIES( ${TEST_ID} PRIVATE gtest gmock)

ADD_TEST( NAME Unit.${TEST_ID} COMMAND ${TEST_ID}  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )