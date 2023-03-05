cmake_minimum_required(VERSION 3.22)
project(git-recent)

find_package(absl CONFIG REQUIRED)
find_package(unofficial-git2 CONFIG REQUIRED)

add_executable(git-recent
        main.cpp)
target_link_libraries(git-recent
         PRIVATE
         unofficial::git2::git2
         absl::flags_parse
         absl::strings
         absl::cleanup)
set_property(TARGET git-recent PROPERTY CXX_STANDARD 20)

install(TARGETS git-recent)
