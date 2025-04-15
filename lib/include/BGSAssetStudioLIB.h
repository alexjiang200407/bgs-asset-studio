#pragma once
#include "asset.h"

LIBRARY_API const char* foo();

LIBRARY_API void init_log();

LIBRARY_API void stop_log();

LIBRARY_API std::set<asset_ptr>
	register_assets(const std::filesystem::path& dir, const std::filesystem::path& preset_path);

LIBRARY_API std::set<asset_ptr> register_assets(const std::filesystem::path& dir);