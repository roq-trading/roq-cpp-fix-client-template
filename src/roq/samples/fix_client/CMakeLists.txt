set(TARGET_NAME ${PROJECT_NAME})

add_subdirectory(flags)

set(SOURCES application.cpp controller.cpp crypto.cpp session.cpp settings.cpp main.cpp)

add_executable(${TARGET_NAME} ${SOURCES})

add_dependencies(${TARGET_NAME} ${TARGET_NAME}-flags-autogen-headers)

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${TARGET_NAME}-flags
          roq-io::roq-io
          roq-codec::roq-codec
          roq-fix::roq-fix
          roq-utils::roq-utils
          roq-logging::roq-logging
          roq-logging::roq-logging-flags
          roq-flags::roq-flags
          roq-api::roq-api
          fmt::fmt)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

target_compile_definitions(${TARGET_NAME} PRIVATE ROQ_PACKAGE_NAME="${PROJECT_NAME}")

install(TARGETS ${TARGET_NAME})
