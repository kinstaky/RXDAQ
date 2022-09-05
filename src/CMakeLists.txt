# include directories
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

# interactor library
add_library(
	interactor STATIC
	interactor.cpp ${PROJECT_INCLUDE_DIR}/interactor.h
)
target_include_directories(
	interactor
	PUBLIC ${PROJECT_SOURCE_DIR}
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
target_link_libraries(
	parser
	PUBLIC interactor
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
target_link_libraries(
	frame
	PUBLIC interactor parser
)