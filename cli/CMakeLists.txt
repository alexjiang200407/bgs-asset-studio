
include(cmake/sourcelist.cmake)
include(cmake/headerlist.cmake)

add_executable(
	BGSAssetStudioCLI ${SOURCES} 
)

target_link_libraries(BGSAssetStudioCLI PRIVATE BGSAssetStudioLIB)
target_precompile_headers(BGSAssetStudioCLI PRIVATE PCH.h)
COPY_DLL_AND_PDB_TO_EXECUTABLE(BGSAssetStudioCLI BGSAssetStudioLIB)