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

source_group(
	TREE
		${CMAKE_CURRENT_BINARY_DIR}
	FILES
		${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
)

add_library(
	BGSAssetStudioLIB SHARED ${SOURCES} 
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	${CMAKE_CURRENT_BINARY_DIR}/version.rc 
	${CMAKE_SOURCE_DIR}/.clang-format
)

target_compile_definitions(BGSAssetStudioLIB PRIVATE LIBRARY_EXPORTS PUBLIC LIBRARY_EXPORTS)

target_link_libraries(BGSAssetStudioLIB PRIVATE Microsoft::DirectXTex spdlog::spdlog)

target_include_directories(BGSAssetStudioLIB PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
target_precompile_headers(BGSAssetStudioLIB PUBLIC ${CMAKE_SOURCE_DIR}/PCH.h PRIVATE ${CMAKE_SOURCE_DIR}/PCH.h PCH.h)