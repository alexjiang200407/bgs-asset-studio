#pragma once

namespace argparse
{
	template <typename T>
	using argparse_action = std::function<T(const std::string&)>;

	argparse_action<int>  texture_sz(const std::string& arglabel);
	std::filesystem::path filepath(const std::string& str);
}
