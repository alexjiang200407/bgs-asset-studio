include(cmake/sourcelist.cmake)
include(cmake/headerlist.cmake)

add_executable(
	bgs-asset-studio-cli ${SOURCES} 
)

find_package(argparse)

target_link_libraries(bgs-asset-studio-cli PRIVATE bgs-asset-studio-lib argparse::argparse)
target_precompile_headers(bgs-asset-studio-cli PRIVATE PCH.h)
COPY_DLL_AND_PDB_TO_EXECUTABLE(bgs-asset-studio-cli bgs-asset-studio-lib)
COPY_ASSETS(bgs-asset-studio-cli)
target_compile_features(bgs-asset-studio-cli PRIVATE cxx_std_23)