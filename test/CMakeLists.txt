# fetch googletest
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
 	GIT_TAG 5376968f6948923e2411081fd9372e71a59d8e77
)
FetchContent_MakeAvailable(googletest)

# test crate library
add_library(
	test_crate STATIC
	test_crate.cpp test_crate.h
)
target_compile_options(
	test_crate
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	test_crate
	PUBLIC crate
)


# test parser
add_executable(
	parser_test
	parser_test.cpp
)
target_compile_options(
	parser_test
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	parser_test
	PRIVATE gtest_main interactor parser
)

# test interactor
add_executable(
	interactor_test
	interactor_test.cpp
)
target_compile_options(
	interactor_test
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	interactor_test
	PRIVATE gtest_main interactor parser test_crate
)


# test config
add_executable(
	config_test
	config_test.cpp
)
target_compile_definitions(
	config_test
	PRIVATE TEST_DATA_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/data/config/"
)
target_compile_options(
	config_test
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	config_test
	PRIVATE gtest_main config
)


# test message
add_executable(
	message_test
	message_test.cpp
)
target_compile_options(
	message_test
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	message_test
	PRIVATE gtest_main message
)



# googletest discover
include(GoogleTest)
gtest_discover_tests(parser_test)
gtest_discover_tests(interactor_test)
gtest_discover_tests(config_test)
gtest_discover_tests(message_test)