#pragma once
#include "asset.h"
#include "asset_registry.h"
#include <concurrent_queue.h>

class asset_registry_impl : public asset_registry
{
public:
	asset_registry_impl() = default;

public:
	void insert(asset_ptr asset);

	void process_all(size_t num_threads) override;

private:
	concurrency::concurrent_queue<asset_ptr> assets;
};