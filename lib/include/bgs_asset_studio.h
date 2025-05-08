#pragma once
#include "asset_registry.h"

LIBRARY_API const char* foo();

LIBRARY_API void init_log();

LIBRARY_API void stop_log();

LIBRARY_API asset_registry_handle register_assets(
	const std::filesystem::path& dir,
	const std::filesystem::path& preset_path,
	size_t                       num_threads,
	const size_t                 max_width_height);

LIBRARY_API asset_registry_handle register_assets(
	const std::filesystem::path& dir,
	size_t                       num_threads,
	const size_t                 max_width_height);
