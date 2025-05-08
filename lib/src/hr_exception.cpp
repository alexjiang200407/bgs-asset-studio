#include "hr_exception.h"

using namespace std;

string get_hr_error(HRESULT hr)
{
	char* msg_buffer = nullptr;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&msg_buffer,
		0,
		nullptr);
	string result = msg_buffer ? msg_buffer : "Unknown error";
	LocalFree(msg_buffer);
	return result;
}

hr_exception::hr_exception(HRESULT hr, const string& msg) :
	runtime_error(msg + " Reason: " + get_hr_error(hr))
{}