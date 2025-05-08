#pragma once
#include "asset.h"

namespace ns
{
	struct tex_mapping
	{
		std::string name;
		std::string match_str;
		std::wregex match_regex;
		std::string opaque_format_str;
		std::string transparent_format_str;
		DXGI_FORMAT opaque_format      = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT transparent_format = DXGI_FORMAT_UNKNOWN;
	};

	struct asset_studio_meta
	{
		std::string                  name;
		std::vector<ns::tex_mapping> mappings;
	};

}

class asset_builder
{
public:
	using task = std::function<asset_ptr()>;

public:
	class exception : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};

public:
	void               push(const std::filesystem::path& path);
	void               pop() noexcept;
	const std::string& preset_name() noexcept;
	const std::string  asset_type(const std::filesystem::path& path) noexcept;
	bool               empty() noexcept;
	task               build(const std::filesystem::path& path, const size_t max_width_height);

private:
	std::stack<ns::asset_studio_meta> tex_mapping_context_stack;
};
