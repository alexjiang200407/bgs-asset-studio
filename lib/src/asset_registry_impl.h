#pragma once
#include "asset.h"
#include "asset_registry.h"

class asset_registry_impl : public asset_registry
{
public:
	asset_registry_impl(std::set<asset_ptr>&& assets);

private:
	std::set<asset_ptr> assets;
};