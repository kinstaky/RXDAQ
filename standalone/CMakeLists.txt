add_executable(daq daq.cpp)
target_compile_options(
	daq
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	daq
	PUBLIC frame
)