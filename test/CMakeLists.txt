cmake_minimum_required(VERSION 3.10.3)

# gtest support
ADD_SUBDIRECTORY(googletest)
INCLUDE_DIRECTORIES(googletest/googletest/include)
INCLUDE_DIRECTORIES(googletest/googlemock/include)


#mocks
INCLUDE_DIRECTORIES(Mock)
ADD_SUBDIRECTORY(EventsStorage)
ADD_SUBDIRECTORY(Communication)
ADD_SUBDIRECTORY(Lib)

