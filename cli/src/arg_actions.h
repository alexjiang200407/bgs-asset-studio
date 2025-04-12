#pragma once

namespace argparse
{
	template <typename T>
	using argparse_action = std::function<T(const std::string&)>;

	argparse_action<int>        range(const std::string& arglabel, int mn, int mx);
	argparse_action<int>        texture_sz(const std::string& arglabel);
	argparse_action<std::regex> regex_action(const std::string& arglabel);
	std::filesystem::path       filepath(const std::string& str);
}
