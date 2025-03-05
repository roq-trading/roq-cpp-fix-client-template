set(TARGET_NAME ${PROJECT_NAME}-session)

set(SOURCES crypto.cpp manager.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

add_dependencies(${TARGET_NAME} ${PROJECT_NAME}-flags-autogen-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-io::roq-io roq-fix::roq-fix roq-utils::roq-utils roq-api::roq-api fmt::fmt)
