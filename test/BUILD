load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "testCrate",
  srcs = ["TestCrate.cpp"],
  hdrs = ["TestCrate.h"],
  deps = ["//main:crate"],
  visibility = ["//test/Interactor:__pkg__"]
)
