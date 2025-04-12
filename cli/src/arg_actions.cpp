#include "arg_actions.h"

using namespace std;
using namespace argparse;

argparse_action<int> argparse::texture_sz(const std::string& arglabel)
{
	return [=](const string& value) {
		static constexpr int MAX_TEXTURE_SIZE = 16384;
		int                  num              = stoi(value);

		if (num < 0)
			throw std::runtime_error(arglabel + " must be non-negative");

		if (num > MAX_TEXTURE_SIZE)
			throw std::runtime_error(
				arglabel + " must be smaller than " + to_string(MAX_TEXTURE_SIZE));

		if ((num & (num - 1)) != 0)
			throw std::runtime_error(arglabel + " must be to the power of 2");

		return num;
	};
}

filesystem::path argparse::filepath(const string& str) { return filesystem::path(str); }
