#pragma once
#include "asset_registry.h"

LIBRARY_API const char* foo();

LIBRARY_API void init_log();

LIBRARY_API void stop_log();

LIBRARY_API asset_registry_handle
	register_assets(const std::filesystem::path& dir, const std::filesystem::path& preset_path);

LIBRARY_API asset_registry_handle register_assets(const std::filesystem::path& dir);

LIBRARY_API void process_assets(const asset_registry& assets);
