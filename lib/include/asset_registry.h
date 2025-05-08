#pragma once

class asset_registry;

typedef std::unique_ptr<asset_registry> asset_registry_handle;

class asset_registry
{
public:
	virtual ~asset_registry()                    = default;
	virtual void process_all(size_t num_threads) = 0;
};
