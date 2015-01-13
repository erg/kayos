cmake_minimum_required(VERSION 2.6)
project(kayos)

set(CMAKE_C_FLAGS "-std=gnu99 -Wall -Wextra")
set(kayos_VERSION_MAJOR 0)
set(kayos_VERSION_MINOR 1)

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/forestdb/include/libforestdb)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/jansson/src)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/http-parser)

#message("CMAKE_INCLUDE_PATH: ${CMAKE_INCLUDE_PATH}")

add_subdirectory(forestdb)
find_path(FORESTDB_INCLUDES
  NAMES forestdb.h
  PATHS ${PROJECT_SOURCE_DIR}/forestdb/include/libforestdb/forestdb.h)

add_subdirectory(jansson)
find_path(JANSSON_INCLUDES
  NAMES jansson.h
  PATHS ${PROJECT_SOURCE_DIR}/jansson/src/jansson.h)

find_path(HTTP-PARSER-INCLUDES
  NAMES http_parser.h
  PATHS ${PROJECT_SOURCE_DIR}/http-parser)

set(common_files
  src/both.c
  src/buffer.c
  src/errors.c
  src/hexdump.c
  src/json_utils.c
  src/paths.c
  src/io.c
  http-parser/http_parser.c
  http-parser/http_parser.h
)

add_executable(kayos-producer
  ${common_files}
  src/kayos_producer_main.c
  src/producer.c
  src/producer_http.c
  src/producer_json.c
)
target_link_libraries(kayos-producer forestdb jansson)

add_executable(kayos-consumer
  ${common_files}
  src/kayos_consumer_main.c
  src/consumer.c
  src/consumer_http.c
  src/consumer_json.c
)
target_link_libraries(kayos-consumer forestdb jansson)

add_executable(kayos-server
  ${common_files}
  src/kayos_server_main.c
)
target_link_libraries(kayos-server forestdb jansson)
