load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
	name = "dstring",
	srcs = ["dstring.cpp"],
	hdrs = ["dstring.h"],
	visibility = ["//test/dstring:__pkg__"]
)

cc_library(
  name = "config",
  srcs = ["Config.cpp"],
  hdrs = ["Config.h"],
  visibility = ["//test/Config:__pkg__"],
  deps = [
  	"//lib:json",
  ]
)


cc_library(
	name = "error",
	srcs = ["Error.cpp"],
	hdrs = ["Error.h"],
)

cc_library(
	name = "firmware",
	srcs = ["Firmware.cpp"],
	hdrs = ["Firmware.h"],
	deps = [
		"//lib/pixie_sdk:PixieSDK",
	]
)


cc_library(
	name = "crate",
	srcs = ["Crate.cpp"],
	hdrs = ["Crate.h", "defs.h"],
	# copts = ["-Ilib/pixie_sdk/include"],
	deps = [
		"config",
		"dstring",
		"error",
		"firmware",
		"//lib/pixie_sdk:PixieSDK",
		"//lib:json"
	],
	visibility = ["//visibility:public"]
)



cc_library(
	name = "interactor",
	srcs = ["Interactor.cpp"],
	hdrs = ["Interactor.h", "defs.h"],
	# copts = ["-Ilib/pixie_sdk/include"],
	deps = [
		"dstring",
		"crate",
		"//lib:args",
	],
	visibility = ["//visibility:public"]
)

cc_library(
	name = "rxdaq",
	srcs = ["RXDAQ.cpp"],
	hdrs = ["RXDAQ.h"],
	deps = [
		"dstring",
		"interactor",
		"crate",
	],
	visibility = ["//visibility:public"]
)


cc_binary(
	name = "daq",
	srcs = ["main.cpp"],
	deps = [
		"rxdaq",
	],
)
