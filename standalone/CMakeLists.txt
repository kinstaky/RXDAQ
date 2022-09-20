add_executable(daq daq.cpp)
target_compile_options(
	daq
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	daq
	PUBLIC frame
)

add_executable(batch_mode batch_mode.cpp)
target_compile_options(
	batch_mode
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(batch_mode PUBLIC frame)
