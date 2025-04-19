#pragma once

#define THROW_HR_EXCEPTION(hr, msg)      \
	if (auto res = hr; FAILED(res)) \
	throw hr_exception(hr, msg)

class hr_exception : public std::runtime_error
{
public:
	hr_exception(HRESULT hr, const std::string& msg);
};