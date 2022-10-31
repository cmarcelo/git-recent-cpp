cmake_minimum_required(VERSION 3.22)
project(git-recent)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)

set(ABSL_BUILD_TESTING OFF)
set(ABSL_PROPAGATE_CXX_STD ON)
add_subdirectory(thirdparty/abseil-cpp)

find_package(PkgConfig)
pkg_check_modules(libgit2 REQUIRED libgit2)

add_executable(git-recent
        main.cpp)
target_link_libraries(git-recent
        ${libgit2_LIBRARIES}
        absl::flags_parse
        absl::strings
        absl::cleanup)

install(TARGETS git-recent)
