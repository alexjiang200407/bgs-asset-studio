#pragma once

class asset_registry;

typedef std::unique_ptr<asset_registry> asset_registry_handle;

class asset_registry
{
public:
	virtual ~asset_registry() = default;
};
