cmake_minimum_required(VERSION 3.22)

project(
  compiler
  VERSION "0.0.1"
  DESCRIPTION "My compiler"
  LANGUAGES C CXX
)
find_path(READLINE_INCLUDE_DIR NAMES readline/readline.h REQUIRED)
find_library(READLINE_LIBRARY NAMES readline REQUIRED)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(compiler
  Environemnt.cpp
  # printer.cpp
  reader.cpp
  # step2_eval.cpp
  # step3_env.cpp
  # step4_if_fn_do.cpp
  main.cpp
  string.cpp
  type.cpp
)
target_compile_features(compiler PUBLIC cxx_std_23)
target_compile_options(compiler PUBLIC -Wall -Wextra -Wpedantic)

# Link to your target
target_include_directories(compiler PRIVATE ${READLINE_INCLUDE_DIR})
target_link_libraries(compiler PRIVATE ${READLINE_LIBRARY})
