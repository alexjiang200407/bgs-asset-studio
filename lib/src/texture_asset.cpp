#include "texture_asset.h"

using namespace std;
namespace fs = filesystem;

texture_asset::texture_asset(
	const fs::path& path,
	texture_size    size,
	texture_size    old_size,
	DXGI_FORMAT     format,
	DXGI_FORMAT     old_format) :
	asset(asset::type::TEXTURE, path),
	format(format), old_format(old_format), size(size), old_size(old_size)
{}

asset_ptr texture_asset::create(
	const fs::path& path,
	texture_size    size,
	texture_size    old_size,
	DXGI_FORMAT     format,
	DXGI_FORMAT     old_format)
{
	return make_shared<texture_asset>(texture_asset(path, size, old_size, format, old_format));
}

void texture_asset::process() const
{
	spdlog::info("Processing {}", path.string());
	const wstring  path_wstring = path.wstring();
	const wchar_t* path_wchar = path_wstring.c_str();



}
