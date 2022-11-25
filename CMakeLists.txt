cmake_minimum_required(VERSION 3.22)
project(git-recent)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
find_package(absl REQUIRED)
find_package(libgit2 REQUIRED)

add_executable(git-recent
        main.cpp)
target_link_libraries(git-recent
        libgit2::libgit2
        absl::flags_parse
        absl::strings
        absl::cleanup)
set_property(TARGET git-recent PROPERTY CXX_STANDARD 20)

install(TARGETS git-recent)
