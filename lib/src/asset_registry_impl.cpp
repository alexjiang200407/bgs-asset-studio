#include "asset_registry_impl.h"

using namespace std;

asset_registry_impl::asset_registry_impl(std::set<asset_ptr>&& assets) : assets(move(assets)) {}
