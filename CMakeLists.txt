cmake_minimum_required(VERSION 3.22)
project(git-recent)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig)
pkg_check_modules(libgit2 REQUIRED libgit2)
find_package(Boost 1.74 REQUIRED COMPONENTS program_options)

add_executable(git-recent
        main.cpp)
target_link_libraries(git-recent
        ${libgit2_LIBRARIES}
        Boost::program_options)

install(TARGETS git-recent)