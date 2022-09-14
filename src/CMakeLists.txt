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

# fetch grpc/grpc
FetchContent_Declare(
	grpc
	GIT_REPOSITORY https://github.com/grpc/grpc
	GIT_TAG v1.48.1
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(grpc)

set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_GRPCPP grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)

add_subdirectory(proto)


# include directories
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

# error library
add_library(
	error
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
	message
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
	config
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
	crate
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

# remote crate
add_library(
	remote_crate
	remote_crate.cpp ${PROJECT_INCLUDE_DIR}/remote_crate.h
)
target_include_directories(
	remote_crate
	PUBLIC ${PROJECT_SOURCE_DIR}
)
target_compile_options(
	remote_crate
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	remote_crate
	PUBLIC crate control_crate_grpc_proto
)

# control crate service
add_library(
	control_crate_service
	control_crate_service.cpp ${PROJECT_INCLUDE_DIR}/control_crate_service.h
)
target_include_directories(
	control_crate_service
	PUBLIC ${PROJECT_SOURCE_DIR}
)
target_compile_options(
	control_crate_service
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	control_crate_service
	PUBLIC crate control_crate_grpc_proto
)

# interactor library
add_library(
	interactor
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
	PUBLIC crate error control_crate_service cxxopts::cxxopts 
)

# parser library
add_library(
	parser
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
	frame
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
	PUBLIC interactor parser error crate remote_crate
)