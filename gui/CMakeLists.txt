
include(cmake/sourcelist.cmake)
include(cmake/headerlist.cmake)

add_executable(
	BGSAssetStudioGUI ${SOURCES} 
)

set_target_properties(BGSAssetStudioGUI PROPERTIES 
    WIN32_EXECUTABLE YES
)

target_link_libraries(BGSAssetStudioGUI PRIVATE BGSAssetStudioLIB)
target_precompile_headers(BGSAssetStudioGUI PRIVATE PCH.h)
COPY_DLL_AND_PDB_TO_EXECUTABLE(BGSAssetStudioGUI BGSAssetStudioLIB)