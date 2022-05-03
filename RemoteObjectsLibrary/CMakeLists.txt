find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Network WebSockets)

add_library(LoadTestLibrary SHARED
            LoadTestLibrary.cpp
            LoadTestLibrary.h)
            
target_include_directories(LoadTestLibrary PRIVATE ${CUTE_SERVER_SDK_DIR}/include)
            
target_link_libraries(LoadTestLibrary
                      ${CUTE_SERVER_SDK_DIR}/lib/libCuteServerSdk.so
                      Qt${QT_VERSION_MAJOR}::Core
                      Qt${QT_VERSION_MAJOR}::Concurrent
                      Qt${QT_VERSION_MAJOR}::Network
                      Qt${QT_VERSION_MAJOR}::WebSockets)
