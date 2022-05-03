find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Network WebSockets)

add_library(ClientLoadTestAppLibrary SHARED
            AsyncClientLoadAppWorker.cpp
            AsyncClientLoadAppWorker.h
            ClientLoadAppManager.cpp
            ClientLoadAppManager.h
            ClientLoadAppWorker.cpp
            ClientLoadAppWorker.h
            Main.cpp
            ThreadFactory.cpp
            ThreadFactory.h)
set_target_properties(ClientLoadTestAppLibrary PROPERTIES
                      INSTALL_RPATH "$ORIGIN"
                      BUILD_WITH_INSTALL_RPATH True)
target_include_directories(ClientLoadTestAppLibrary PRIVATE ${CUTE_CLIENT_SDK_DIR}/include)
target_link_libraries(ClientLoadTestAppLibrary
                      ${CUTE_CLIENT_SDK_DIR}/lib/libCuteClientSdk.so
                      Qt${QT_VERSION_MAJOR}::Core
                      Qt${QT_VERSION_MAJOR}::Concurrent
                      Qt${QT_VERSION_MAJOR}::Network
                      Qt${QT_VERSION_MAJOR}::WebSockets)
