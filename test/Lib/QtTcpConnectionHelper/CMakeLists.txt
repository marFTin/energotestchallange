cmake_minimum_required(VERSION 3.10.2)

SET ( TEST_ID Test.Lib.QtTcpConnectionHelper )

SET( SOURCES
        Main.cpp
        TestCases.cpp
)

ADD_EXECUTABLE( ${TEST_ID} ${SOURCES})

# includes to unit under test
TARGET_INCLUDE_DIRECTORIES( ${TEST_ID} PRIVATE "${CMAKE_SOURCE_DIR}/src/Lib/QtTcpConnectionHelper" )

TARGET_LINK_LIBRARIES( ${TEST_ID} PRIVATE Lib.QtTcpConnectionHelper )
TARGET_LINK_LIBRARIES( ${TEST_ID} PRIVATE gtest gmock)

ADD_TEST( NAME Unit.${TEST_ID} COMMAND ${TEST_ID}  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )