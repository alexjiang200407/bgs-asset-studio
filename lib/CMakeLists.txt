configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/Version.rc
	@ONLY
)

include(cmake/sourcelist.cmake)
include(cmake/headerlist.cmake)

find_package(directxtex CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

source_group(
	TREE
		${CMAKE_CURRENT_BINARY_DIR}
	FILES
		${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
)
add_library(
	bgs-asset-studio-lib SHARED ${SOURCES} 
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	${CMAKE_CURRENT_BINARY_DIR}/version.rc 
	${CMAKE_SOURCE_DIR}/.clang-format
)
target_compile_features(bgs-asset-studio-lib PRIVATE cxx_std_23)

target_compile_definitions(bgs-asset-studio-lib PRIVATE LIBRARY_EXPORTS PUBLIC LIBRARY_EXPORTS)

target_link_libraries(
	bgs-asset-studio-lib
	PRIVATE
		Microsoft::DirectXTex
		spdlog::spdlog
		nlohmann_json
)

target_include_directories(bgs-asset-studio-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
target_precompile_headers(bgs-asset-studio-lib PUBLIC ${CMAKE_SOURCE_DIR}/PCH.h PRIVATE ${CMAKE_SOURCE_DIR}/PCH.h PCH.h)


