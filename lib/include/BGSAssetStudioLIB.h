#pragma once


LIBRARY_API const char* foo();

LIBRARY_API void init_log();

LIBRARY_API void stop_log();

LIBRARY_API void register_assets(std::filesystem::path dir);
