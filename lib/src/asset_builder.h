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
		DXGI_FORMAT opaque_format;
		DXGI_FORMAT transparent_format;
	};
}

class asset_builder
{
public:
	void                         push(const std::filesystem::path& p);
	void                         pop();
	const std::filesystem::path& top();
	const std::string            asset_type(const std::filesystem::path& p);
	bool                         empty();

private:
	std::stack<std::filesystem::path>        path_stack;
	std::stack<std::vector<ns::tex_mapping>> tex_mapping_context_stack;
};
