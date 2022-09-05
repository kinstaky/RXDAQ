# fetch xiallc/pixiesdk
FetchContent_Declare(
	pixie_sdk
	GIT_REPOSITORY https://github.com/xiallc/PixieSDK.git
	GIT_TAG eb66fbd4b701deb775bcc05888be7c0ba12c79fe
)
FetchContent_MakeAvailable(pixie_sdk)

# fetch nlohmann/json
FetchContent_Declare(
	json
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# fetch jarro2783/cxxopts
FetchContent_Declare(
	cxxopts
	GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
	GIT_TAG f7cf84bd56b47728e4674f316d274b3d82a1cfb3
)
FetchContent_MakeAvailable(cxxopts)


# include directories
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

# error library
add_library(
	error STATIC
	error.cpp ${PROJECT_INCLUDE_DIR}/error.h
)
target_include_directories(
	error
	PUBLIC ${PROJECT_SOURCE_DIR}
)
target_compile_options(
	error
	PRIVATE -Werror -Wall -Wextra
)

# message library
add_library(
	message STATIC
	message.cpp ${PROJECT_INCLUDE_DIR}/message.h
)
target_include_directories(
	message
	PUBLIC ${PROJECT_SOURCE_DIR}
)
target_compile_options(
	message
	PRIVATE -Werror -Wall -Wextra
)

# config library
add_library(
	config STATIC
	config.cpp ${PROJECT_INCLUDE_DIR}/config.h
)
target_include_directories(
	config
	PUBLIC ${PROJECT_SOURCE_DIR}
)
target_compile_options(
	config
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	config
	PUBLIC nlohmann_json::nlohmann_json
)

# crate library
add_library(
	crate STATIC
	crate.cpp ${PROJECT_INCLUDE_DIR}/crate.h
)
target_include_directories(
	crate
	PUBLIC ${PROJECT_SOURCE_DIR}
)
target_compile_options(
	crate
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	crate
	PUBLIC error config message PixieSDK
)

# interactor library
add_library(
	interactor STATIC
	interactor.cpp ${PROJECT_INCLUDE_DIR}/interactor.h
)
target_include_directories(
	interactor
	PUBLIC ${PROJECT_SOURCE_DIR}
)
target_compile_options(
	interactor
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	interactor
	PUBLIC crate error cxxopts::cxxopts
)

# parser library
add_library(
	parser STATIC
	parser.cpp ${PROJECT_INCLUDE_DIR}/parser.h
)
target_include_directories(
	parser
	PUBLIC ${PROJECT_SOURCE_DIR}
)
target_compile_options(
	parser
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	parser
	PUBLIC interactor error
)

# frame library
add_library(
	frame STATIC
	frame.cpp ${PROJECT_INCLUDE_DIR}/frame.h
)
target_include_directories(
	frame
	PUBLIC ${PROJECT_SOURCE_DIR}
)
target_compile_options(
	frame
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	frame
	PUBLIC interactor parser error crate
)