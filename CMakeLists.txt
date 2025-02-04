cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(custom_caf_vec VERSION 1.0.0)

add_subdirectory(${custom_caf_vec_SOURCE_DIR}/external/spdlog)

execute_process(
    COMMAND bash ${custom_caf_vec_SOURCE_DIR}/scripts/build_caf.sh
)

set(CMAKE_MODULE_PATH ${custom_caf_vec_SOURCE_DIR}/cmake)
set(CAF_ROOT_DIR ${custom_caf_vec_SOURCE_DIR}/external/actor-framework/build)
find_package(CAF COMPONENTS core io test)

string(APPEND CMAKE_CXX_FLAGS " -Werror -Wall -Wextra -Wcast-align -Wcast-qual -Wmissing-field-initializers -Wmissing-include-dirs -Wpointer-arith -Wstack-protector -Wstrict-aliasing=2 -Wunreachable-code -Wlogical-op -Wduplicated-cond -Wnull-dereference -Wrestrict -Wno-unknown-pragmas -Wno-unused-function")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -g -O0")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3 -march=native -funroll-loops -flto -fwhole-program -DNDEBUG")

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

find_package(Threads REQUIRED)

### APPLICATION
set(APP_NAME custom_caf_vec_app)

set(HEADERS
    include/thread_id.hpp
    include/trim.hpp
    include/io/skip_whitespaces.hpp
    include/io/skip_to_next_line.hpp
    include/io/skip_word.hpp
    include/io/line_reader.hpp
    include/io/istream_char_consumer.hpp
    include/merge.hpp
    include/log_level.hpp
    include/logger_id.hpp
    include/node_id.hpp
    include/mailbox_id.hpp
    include/entity.hpp
    include/entity_set.hpp
    include/entity_set_range.hpp
    include/node_cmp.hpp
    include/actor_cmp.hpp
    include/node_range.hpp
    include/thread_range.hpp
    include/log_entry.hpp
    include/enhanced_log_entry.hpp
    include/se_type.hpp
    include/se_event.hpp
    include/field_key_compare.hpp
    include/parse_event.hpp
    include/logger_id_meta_data.hpp
    include/verbosity_level.hpp
    include/first_pass.hpp
    include/second_pass.hpp
    include/config.hpp
    include/boolalpha.hpp
    include/initialize_spdlog.hpp
    include/io/read_until.hpp
    include/io/read_until_newline_and_digit.hpp)

set(SOURCES
    src/main.cpp
    src/trim.cpp
    src/io/skip_whitespaces.cpp
    src/io/skip_to_next_line.cpp
    src/io/skip_word.cpp
    src/io/line_reader.cpp
    src/io/istream_char_consumer.cpp
    src/merge.cpp
    src/log_level.cpp
    src/logger_id.cpp
    src/node_id.cpp
    src/mailbox_id.cpp
    src/entity.cpp
    src/entity_set_range.cpp
    src/node_cmp.cpp
    src/actor_cmp.cpp
    src/node_range.cpp
    src/thread_range.cpp
    src/log_entry.cpp
    src/enhanced_log_entry.cpp
    src/se_type.cpp
    src/se_event.cpp
    src/field_key_compare.cpp
    src/parse_event.cpp
    src/first_pass.cpp
    src/second_pass.cpp
    src/config.cpp
    src/initialize_spdlog.cpp)

add_executable(
    ${APP_NAME}
    ${HEADERS}
    ${SOURCES}
)

target_link_libraries(
    ${APP_NAME}
    PRIVATE
    Threads::Threads
)

target_link_libraries(
    ${APP_NAME}
    PRIVATE
    spdlog::spdlog
)

target_link_libraries(
    ${APP_NAME}
    PRIVATE
    ${CMAKE_DL_LIBS} ${CAF_LIBRARY_CORE} ${CAF_LIBRARY_IO}
)

target_include_directories(
    ${APP_NAME}
    PRIVATE
    ${CAF_INCLUDE_DIRS}
)

target_include_directories(
    ${APP_NAME}
    PRIVATE
    ${custom_caf_vec_SOURCE_DIR}/include
)

target_compile_definitions(
    ${APP_NAME}
    PRIVATE
    SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
)

target_compile_definitions(
    ${APP_NAME}
    PRIVATE
    CAF_ENABLE_ACTOR_PROFILER
)

