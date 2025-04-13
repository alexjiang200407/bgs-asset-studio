#pragma once
#include "asset.h"

class texture_asset :
	public asset
{


private:
	DXGI_FORMAT format;
	DXGI_FORMAT old_format;
};


