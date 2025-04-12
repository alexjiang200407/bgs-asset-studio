cmake_minimum_required(VERSION 3.10)
project(BGSAssetStudio VERSION 0.0.1)

function(copy_dll_and_pdb_to_executable target_name dll_target)
    add_custom_command(
        TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${dll_target}> $<TARGET_FILE_DIR:${target_name}>
        COMMENT "Copying ${dll_target} DLL to the executable directory"
    )
    
    add_custom_command(
        TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${dll_target}> $<TARGET_FILE_DIR:${target_name}>
        COMMENT "Copying ${dll_target} PDB to the executable directory"
    )
endfunction()

add_subdirectory(lib)
add_subdirectory(gui)
add_subdirectory(cli)
