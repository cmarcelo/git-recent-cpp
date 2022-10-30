// Copyright (c) 2022 Caio Oliveira
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <boost/outcome.hpp>
#include <boost/program_options.hpp>
#include <git2.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <span>
#include <string>
#include <vector>

// TODO: Should (also) look at "ref" file date?
// TODO: Colored output?

namespace {

struct entry {
  entry(git_reference *ref, git_commit *commit)
      : ref(ref), commit(commit), name(nullptr) {
    git_branch_name(&name, ref);
    commit_time = std::chrono::system_clock::time_point{
        std::chrono::seconds(git_commit_time(commit))};
  }

  git_reference *ref;
  git_commit *commit;

  const char *name;
  std::chrono::system_clock::time_point commit_time;
};

struct error {
  std::string msg;
};

error make_git_error() {
  if (const git_error *ge = git_error_last(); ge)
    return {ge->message};
  else
    return {"error"};
}

std::string format_duration(std::chrono::system_clock::duration duration) {
  namespace c = std::chrono;

  const auto d = c::duration_cast<c::days>(duration);
  const auto h = c::duration_cast<c::hours>(duration - d);
  const auto m = c::duration_cast<c::minutes>(duration - d - h);

  std::ostringstream oss;
  if (d.count() > 0)
    oss << std::setw(5) << d.count() << "d ago";
  else if (h.count() > 0)
    oss << std::setw(5) << h.count() << "h ago";
  else if (d.count() > 0)
    oss << std::setw(5) << m.count() << "m ago";
  else
    oss << std::setw(10) << "now";

  return oss.str();
}

struct options {
  unsigned n;
  bool remote;
};

options parse_options(int argc, char *argv[]) {
  namespace po = boost::program_options;

  po::options_description desc("Allowed options");

  // clang-format off
  desc.add_options()
    ("help,h", "produce help message")
    ("count,n", po::value<unsigned>()->default_value(7u),
     "show at most N branches, zero means all branches")
    ("remote",
     "show remote branches instead of local branches");
  // clang-format on

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    exit(0);
  }

  return {
      .n = vm["count"].as<unsigned>(),
      .remote = vm.count("remote") > 0,
  };
}

std::tuple<std::vector<entry>, std::optional<error>>
collect_branches(git_repository *repo, git_branch_t branch_type) {
  std::vector<entry> branches;

  git_branch_iterator *branch_it = nullptr;
  if (int err = git_branch_iterator_new(&branch_it, repo, branch_type); err)
    return {branches, make_git_error()};

  while (true) {
    git_reference *ref = nullptr;
    git_branch_t type;
    if (int err = git_branch_next(&ref, &type, branch_it); err) {
      if (err == GIT_ITEROVER)
        break;
      return {branches, make_git_error()};
    }

    git_object *obj = nullptr;
    if (int err = git_reference_peel(&obj, ref, GIT_OBJECT_COMMIT); err)
      return {branches, make_git_error()};

    branches.emplace_back(ref, reinterpret_cast<git_commit *>(obj));
  }

  git_branch_iterator_free(branch_it);

  return {branches, {}};
}

// TODO: Is there a better way to do this?
template <typename T>
std::unique_ptr<T, void (*)(T *)> make_unique_with_deleter(auto *t, auto *d) {
  return std::unique_ptr<T, void (*)(T *)>(t, d);
}

std::optional<error> run(options opts) {
  git_repository *repo_ = nullptr;
  if (int err = git_repository_open_ext(&repo_, ".", 0, nullptr); err)
    return make_git_error();

  auto repo =
      make_unique_with_deleter<git_repository>(repo_, git_repository_free);

  auto [branches, err] = collect_branches(
      repo.get(), opts.remote ? GIT_BRANCH_REMOTE : GIT_BRANCH_LOCAL);
  if (err)
    return err;

  if (opts.n == 0 || opts.n > branches.size())
    opts.n = branches.size();

  std::ranges::partial_sort(
      branches, branches.begin() + opts.n,
      [](auto &a, auto &b) { return a.commit_time > b.commit_time; });

  auto recent = std::span(branches.begin(), opts.n);

  const size_t min_padding = 10;
  auto max_branch_size = std::transform_reduce(
      recent.begin(), recent.end(), min_padding,
      [](auto a, auto b) { return std::max(a, b); },
      [](auto &e) { return strlen(e.name); });

  auto now = std::chrono::system_clock::now();

  for (const auto &e : recent) {
    const auto duration = now - e.commit_time;

    // clang-format off
    std::cout << (git_branch_is_head(e.ref) ? "* " : "  ")
              << std::left << std::setw(int(max_branch_size)) << e.name << "  "
              << std::right << format_duration(duration) << "  "
              << std::left << git_commit_summary(e.commit) << "\n";
    // clang-format on
  }

  for (auto &e : branches) {
    git_commit_free(e.commit);
    git_reference_free(e.ref);
  }

  return {};
}

} // namespace

int main(int argc, char *argv[]) {
  auto opts = parse_options(argc, argv);

  git_libgit2_init();

  if (auto err = run(opts); err) {
    std::cerr << "error: " << err->msg << "\n";
    return EXIT_FAILURE;
  }

  git_libgit2_shutdown();
  return 0;
}
