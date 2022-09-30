project('git-recent', 'cpp', default_options: ['cpp_std=c++20'])
dep_boost = dependency('boost', modules: ['program_options'])
dep_libgit2 = dependency('libgit2')
executable('git-recent', 'main.cpp', install: true, dependencies: [dep_boost, dep_libgit2])
