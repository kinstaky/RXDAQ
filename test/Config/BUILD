load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
	name = "Completion",
	size = "small",
	srcs = ["completionTest.cpp"],
	deps = ["//main:config"],
	# data = ["//test/Config/data:completion/nothing.json"],
	data = ["//test/Config/data:completionData"]
)

cc_test(
	name = "Correction",
	size = "small",
	srcs = ["correctionTest.cpp"],
	deps = ["//main:config"],
	data = ["//test/Config/data:correctionData"]
)