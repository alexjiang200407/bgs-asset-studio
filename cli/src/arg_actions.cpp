#include "arg_actions.h"

using namespace std;
using namespace argparse;

argparse_action<int> argparse::range(const std::string& arglabel, int mn, int mx)
{
	return [=](const string& value) {
		int num = stoi(value);
		if (num < mn)
		{
			if (mn == 0)
				throw runtime_error(arglabel + " must be non-negative");
			else
				throw runtime_error(
					arglabel + " must be greater than or equal to " + to_string(mn));
		}

		if (num > mx)
			throw runtime_error(arglabel + " must be smaller than or equal to " + to_string(mx));

		return num;
	};
}

argparse_action<int> argparse::texture_sz(const string& arglabel)
{
	return [=](const string& value) {
		static constexpr int MAX_TEXTURE_SIZE = 16384;
		int                  num              = range(value, 1, MAX_TEXTURE_SIZE)(value);

		if ((num & (num - 1)) != 0)
			throw runtime_error(arglabel + " must be to the power of 2");

		return num;
	};
}

argparse_action<std::regex> argparse::regex_action(const std::string& arglabel)
{
	return [=](const string& arg) {
		try
		{
			return regex(arg);
		}
		catch (const exception e)
		{
			throw runtime_error("'" + arglabel + "' is invalid: " + e.what());
		}
	};
}

filesystem::path argparse::filepath(const string& str) { return filesystem::path(str); }
