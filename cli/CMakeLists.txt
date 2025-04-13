include(cmake/sourcelist.cmake)
include(cmake/headerlist.cmake)

add_executable(
	BGSAssetStudioCLI ${SOURCES} 
)

find_package(argparse)

target_link_libraries(BGSAssetStudioCLI PRIVATE BGSAssetStudioLIB argparse::argparse)
target_precompile_headers(BGSAssetStudioCLI PRIVATE PCH.h)
COPY_DLL_AND_PDB_TO_EXECUTABLE(BGSAssetStudioCLI BGSAssetStudioLIB)
COPY_ASSETS(BGSAssetStudioCLI)
target_compile_features(BGSAssetStudioCLI PRIVATE cxx_std_23)