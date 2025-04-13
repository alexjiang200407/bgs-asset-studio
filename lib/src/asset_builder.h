#pragma once

namespace ns
{
	struct tex_mapping
	{
		std::string name;
		std::string match_str;
		std::regex  match_regex;
		std::string opaque_format_str;
		std::string transparent_format_str;
		DXGI_FORMAT opaque_format      = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT transparent_format = DXGI_FORMAT_UNKNOWN;
	};
}

class asset_builder
{
public:
	class exception : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};

public:
	void                         push(const std::filesystem::path& path);
	void                         pop();
	const std::filesystem::path& top();
	const std::string            asset_type(const std::filesystem::path& path);
	bool                         empty();
	DXGI_FORMAT                  tex_analysis(const std::filesystem::path& path);

private:
	std::stack<std::filesystem::path>        path_stack;
	std::stack<std::vector<ns::tex_mapping>> tex_mapping_context_stack;
};
