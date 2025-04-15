#pragma once
#include "asset.h"

typedef std::pair<size_t, size_t> texture_size;

class texture_asset : public asset
{
private:
	texture_asset(
		texture_size size,
		texture_size old_size,
		DXGI_FORMAT  format,
		DXGI_FORMAT  old_format);

public:
	static asset_ptr create(
		texture_size size,
		texture_size old_size,
		DXGI_FORMAT  format,
		DXGI_FORMAT  old_format);

private:
	texture_size size;
	texture_size old_size;
	DXGI_FORMAT  format;
	DXGI_FORMAT  old_format;
};
