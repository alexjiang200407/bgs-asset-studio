include(cmake/sourcelist.cmake)
include(cmake/headerlist.cmake)

add_executable(
	bgs-asset-studio-gui ${SOURCES} 
)

set_target_properties(bgs-asset-studio-gui PROPERTIES 
    WIN32_EXECUTABLE YES
)

target_compile_definitions(bgs-asset-studio-gui PRIVATE UNICODE)

target_link_libraries(bgs-asset-studio-gui PRIVATE bgs-asset-studio-lib)
target_precompile_headers(bgs-asset-studio-gui PRIVATE PCH.h)
COPY_DLL_AND_PDB_TO_EXECUTABLE(bgs-asset-studio-gui bgs-asset-studio-lib)
COPY_ASSETS(bgs-asset-studio-gui)

target_compile_features(bgs-asset-studio-gui PRIVATE cxx_std_23)