#include "texture_asset.h"

using namespace std;

texture_asset::texture_asset(
	texture_size size,
	texture_size old_size,
	DXGI_FORMAT  format,
	DXGI_FORMAT  old_format) :
	asset(asset::type::TEXTURE),
	format(format), old_format(old_format), size(size), old_size(old_size)
{}

asset_ptr texture_asset::create(
	texture_size size,
	texture_size old_size,
	DXGI_FORMAT  format,
	DXGI_FORMAT  old_format)
{
	return make_unique<asset>(texture_asset(size, old_size, format, old_format));
}
