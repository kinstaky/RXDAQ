load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "args",
  hdrs = ["args.hxx"],
  visibility = ["//main:__pkg__"],
)

cc_library(
  name = "json",
  hdrs = ["nlohmann/json.hpp"],
  visibility = ["//main:__pkg__", "//lib/pixie_sdk:__pkg__"],
)

