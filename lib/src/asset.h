#pragma once

class asset;
typedef std::unique_ptr<asset> asset_ptr;

class asset
{
public:
	enum class type
	{
		UNUSED = 0,
		TEXTURE,
		MESH,
		ANIMATION,
	};

	asset(type asset_type);

	bool operator<(const asset& rhs) const;
	bool operator==(const asset& rhs) const;

private:
	type asset_type;
};

bool operator<(const asset_ptr& lhs, const asset_ptr& rhs);
