set(TARGET_NAME ${PROJECT_NAME})

add_subdirectory(flags)
add_subdirectory(session)
add_subdirectory(service)

set(SOURCES application.cpp controller.cpp settings.cpp shared.cpp main.cpp)

add_executable(${TARGET_NAME} ${SOURCES})

add_dependencies(${TARGET_NAME} ${TARGET_NAME}-flags-autogen-headers)

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${TARGET_NAME}-flags
          ${TARGET_NAME}-session
          ${TARGET_NAME}-service
          roq-api::roq-api
          roq-fix::roq-fix
          roq-fix-client::roq-fix-client
          roq-fix-client::roq-fix-client-flags
          roq-flags::roq-flags
          roq-io::roq-io
          roq-logging::roq-logging
          roq-logging::roq-logging-flags
          roq-utils::roq-utils
          fmt::fmt)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

target_compile_definitions(${TARGET_NAME} PRIVATE ROQ_PACKAGE_NAME="${TARGET_NAME}" ROQ_BUILD_VERSION="${CMAKE_PROJECT_VERSION}")

install(TARGETS ${TARGET_NAME})
