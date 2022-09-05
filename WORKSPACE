load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# googletest
http_archive(
  name = "com_google_googletest",
  urls = ["https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip"],
  strip_prefix = "googletest-5376968f6948923e2411081fd9372e71a59d8e77",
)

# PlxApi
new_local_repository(
	name = "PlxApi",
	path = "/home/test/Dependencies/broadcom/pcisdk",
	build_file = "lib/PlxApi/BUILD"
)