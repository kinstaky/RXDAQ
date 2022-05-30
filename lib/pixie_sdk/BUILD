load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "PixieSDK",
  srcs = glob(["src/pixie/*.cpp", "src/pixie/pixie16/*.cpp"]),
  hdrs = glob(["include/pixie/*.hpp", "include/pixie/pixie16/*.hpp"]),
  copts = ["-Ilib/pixie_sdk/include", "-Ilib", "-Ilib/PlxApi", "-std=c++14"],
  includes = ["include"],
  local_defines = ["PLX_LINUX", "PCI_CODE", "PLX_LITTLE_ENDIAN"],
  linkopts = ["-pthread"],
  deps = ["//lib:json", "@PlxApi//:PlxApi"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "Pixie16Api",
  srcs = glob(["src/pixie16/*.cpp"]),
  hdrs = glob(["include/pixie16/*.h"]),
  copts = ["-Ilib/pixie_sdk/include"],
  deps = ["PixieSDK"],
  visibility = ["//main:__pkg__"]
)

