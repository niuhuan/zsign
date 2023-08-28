cmake_minimum_required(VERSION 3.14)

project(zsign)
set(CMAKE_CXX_STANDARD 11)

find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")
include_directories(${OPENSSL_INCLUDE_DIR})
list(APPEND LIB_LIST ${OPENSSL_LIBRARIES})

find_package(ZLIB REQUIRED)
message("zlib include dir: ${ZLIB_INCLUDE_DIR}")
message("zlib libraries: ${ZLIB_LIBRARIES}")
include_directories(${ZLIB_INCLUDE_DIR})
list(APPEND LIB_LIST ${ZLIB_LIBRARIES})

find_package(libzip REQUIRED)
list(APPEND LIB_LIST "libzip::zip")

# Main sources

file(GLOB SRC common/*.cpp ./**.cpp)

add_executable(zsign ${SRC})
target_link_libraries(zsign ${LIB_LIST})

#find_package(OpenSSL REQUIRED)
#target_link_libraries(main PRIVATE OpenSSL::SSL OpenSSL::Crypto)

