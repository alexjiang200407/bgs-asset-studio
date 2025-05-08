#include "asset.h"

using namespace std;
namespace fs = filesystem;

asset::asset(type asset_type, const fs::path& path) : asset_type(asset_type), path(path) {}

bool asset::operator<(const asset& rhs) const { return this->asset_type < rhs.asset_type; }

bool asset::operator==(const asset& rhs) const { return this->asset_type == rhs.asset_type; }

bool operator<(const asset_ptr& lhs, const asset_ptr& rhs) { return *lhs < *rhs; }
