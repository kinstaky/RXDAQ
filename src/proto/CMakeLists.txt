# Proto file
get_filename_component(control_crate_proto "control_crate.proto" ABSOLUTE)
get_filename_component(control_crate_proto_path "${control_crate_proto}" PATH)

# Generated sources
set(control_crate_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/control_crate.pb.cc")
set(control_crate_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/control_crate.pb.h")
set(control_crate_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/control_crate.grpc.pb.cc")
set(control_crate_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/control_crate.grpc.pb.h")
add_custom_command(
	OUTPUT "${control_crate_proto_srcs}" "${control_crate_proto_hdrs}"
		"${control_crate_grpc_srcs}" "${control_crate_grpc_hdrs}"
	COMMAND ${_PROTOBUF_PROTOC}
		--grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
    	--cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
    	-I "${control_crate_proto_path}"
    	--plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    	"${control_crate_proto}"
    DEPENDS "${control_crate_proto}"
)


# hw_grpc_proto
add_library(
	control_crate_grpc_proto
	${control_crate_grpc_srcs} ${control_crate_grpc_hdrs}
	${control_crate_proto_srcs} ${control_crate_proto_hdrs}
)
target_include_directories(
	control_crate_grpc_proto
	PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/../../"
)
target_link_libraries(
	control_crate_grpc_proto
	PUBLIC ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF}
)