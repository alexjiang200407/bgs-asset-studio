#pragma once

class asset;
typedef std::shared_ptr<asset> asset_ptr;

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

	asset(type asset_type, const std::filesystem::path& path);

	bool operator<(const asset& rhs) const;
	bool operator==(const asset& rhs) const;

	virtual void process() const = 0;

protected:
	std::filesystem::path path;
	type                  asset_type;
};

bool operator<(const asset_ptr& lhs, const asset_ptr& rhs);
