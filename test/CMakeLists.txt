# fetch googletest
FetchContent_Declare(
	googletest
	# GIT_REPOSITORY https://github.com/google/googletest.git
 	# GIT_TAG 5376968f6948923e2411081fd9372e71a59d8e77
	URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
FetchContent_MakeAvailable(googletest)


# test parser
add_executable(
	parser_test
	parser_test.cpp
)
target_link_libraries(
	parser_test
	PRIVATE gtest_main interactor parser
)


# googletest discover
include(GoogleTest)
gtest_discover_tests(parser_test)