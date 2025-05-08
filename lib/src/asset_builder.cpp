#include "asset_builder.h"
#include "hr_exception.h"
#include "texture_asset.h"
#include <nlohmann/json.hpp>

using namespace nlohmann;
using namespace std;
using namespace DirectX;
namespace fs = filesystem;

static constexpr float opaque_alpha_threshold = 0.95;

#define ENUM_AND_STR(val) \
	{                     \
		#val, val         \
	}

static const unordered_map<string_view, DXGI_FORMAT> dxgi_to_str = {
	ENUM_AND_STR(DXGI_FORMAT_UNKNOWN),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32A32_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32A32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32A32_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32A32_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R32G32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G8X24_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_D32_FLOAT_S8X24_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_X32_TYPELESS_G8X24_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R10G10B10A2_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R10G10B10A2_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R10G10B10A2_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R11G11B10_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R32_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_D32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R32_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R32_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R24G8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_D24_UNORM_S8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R24_UNORM_X8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_X24_TYPELESS_G8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R16_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R16_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_D16_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R16_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R8_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8_SINT),
	ENUM_AND_STR(DXGI_FORMAT_A8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R1_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R9G9B9E5_SHAREDEXP),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_B8G8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_G8R8_G8B8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC1_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC1_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC1_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_BC2_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC2_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC2_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_BC3_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC3_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC3_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_BC4_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC4_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC4_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC5_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC5_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC5_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_B5G6R5_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_B5G5R5A1_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8A8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8X8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8A8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8X8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8X8_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_BC6H_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC6H_UF16),
	ENUM_AND_STR(DXGI_FORMAT_BC6H_SF16),
	ENUM_AND_STR(DXGI_FORMAT_BC7_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC7_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC7_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_AYUV),
	ENUM_AND_STR(DXGI_FORMAT_Y410),
	ENUM_AND_STR(DXGI_FORMAT_Y416),
	ENUM_AND_STR(DXGI_FORMAT_NV12),
	ENUM_AND_STR(DXGI_FORMAT_P010),
	ENUM_AND_STR(DXGI_FORMAT_P016),
	ENUM_AND_STR(DXGI_FORMAT_420_OPAQUE),
	ENUM_AND_STR(DXGI_FORMAT_YUY2),
	ENUM_AND_STR(DXGI_FORMAT_Y210),
	ENUM_AND_STR(DXGI_FORMAT_Y216),
	ENUM_AND_STR(DXGI_FORMAT_NV11),
	ENUM_AND_STR(DXGI_FORMAT_AI44),
	ENUM_AND_STR(DXGI_FORMAT_IA44),
	ENUM_AND_STR(DXGI_FORMAT_P8),
	ENUM_AND_STR(DXGI_FORMAT_A8P8),
	ENUM_AND_STR(DXGI_FORMAT_B4G4R4A4_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_P208),
	ENUM_AND_STR(DXGI_FORMAT_V208),
	ENUM_AND_STR(DXGI_FORMAT_V408),
	ENUM_AND_STR(DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE),
	ENUM_AND_STR(DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE),
	ENUM_AND_STR(DXGI_FORMAT_FORCE_UINT),
};

#define ENUM_AND_STR(val) \
	{                     \
		val, #val         \
	}

static const unordered_map<DXGI_FORMAT, string_view> str_to_format = {
	ENUM_AND_STR(DXGI_FORMAT_UNKNOWN),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32A32_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32A32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32A32_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32A32_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32B32_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16G16B16A16_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R32G32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G32_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R32G8X24_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_D32_FLOAT_S8X24_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_X32_TYPELESS_G8X24_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R10G10B10A2_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R10G10B10A2_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R10G10B10A2_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R11G11B10_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16G16_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R32_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_D32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_R32_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R32_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R24G8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_D24_UNORM_S8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R24_UNORM_X8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_X24_TYPELESS_G8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R16_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R16_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_D16_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R16_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R16_SINT),
	ENUM_AND_STR(DXGI_FORMAT_R8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_R8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_R8_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_R8_SINT),
	ENUM_AND_STR(DXGI_FORMAT_A8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R1_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R9G9B9E5_SHAREDEXP),
	ENUM_AND_STR(DXGI_FORMAT_R8G8_B8G8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_G8R8_G8B8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC1_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC1_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC1_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_BC2_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC2_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC2_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_BC3_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC3_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC3_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_BC4_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC4_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC4_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC5_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC5_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC5_SNORM),
	ENUM_AND_STR(DXGI_FORMAT_B5G6R5_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_B5G5R5A1_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8A8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8X8_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8A8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8X8_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8X8_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_BC6H_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC6H_UF16),
	ENUM_AND_STR(DXGI_FORMAT_BC6H_SF16),
	ENUM_AND_STR(DXGI_FORMAT_BC7_TYPELESS),
	ENUM_AND_STR(DXGI_FORMAT_BC7_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_BC7_UNORM_SRGB),
	ENUM_AND_STR(DXGI_FORMAT_AYUV),
	ENUM_AND_STR(DXGI_FORMAT_Y410),
	ENUM_AND_STR(DXGI_FORMAT_Y416),
	ENUM_AND_STR(DXGI_FORMAT_NV12),
	ENUM_AND_STR(DXGI_FORMAT_P010),
	ENUM_AND_STR(DXGI_FORMAT_P016),
	ENUM_AND_STR(DXGI_FORMAT_420_OPAQUE),
	ENUM_AND_STR(DXGI_FORMAT_YUY2),
	ENUM_AND_STR(DXGI_FORMAT_Y210),
	ENUM_AND_STR(DXGI_FORMAT_Y216),
	ENUM_AND_STR(DXGI_FORMAT_NV11),
	ENUM_AND_STR(DXGI_FORMAT_AI44),
	ENUM_AND_STR(DXGI_FORMAT_IA44),
	ENUM_AND_STR(DXGI_FORMAT_P8),
	ENUM_AND_STR(DXGI_FORMAT_A8P8),
	ENUM_AND_STR(DXGI_FORMAT_B4G4R4A4_UNORM),
	ENUM_AND_STR(DXGI_FORMAT_P208),
	ENUM_AND_STR(DXGI_FORMAT_V208),
	ENUM_AND_STR(DXGI_FORMAT_V408),
	ENUM_AND_STR(DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE),
	ENUM_AND_STR(DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE),
	ENUM_AND_STR(DXGI_FORMAT_FORCE_UINT),
};

static const unordered_map<TEX_ALPHA_MODE, string_view> alpha_mode_str = {
	ENUM_AND_STR(TEX_ALPHA_MODE_UNKNOWN),       ENUM_AND_STR(TEX_ALPHA_MODE_STRAIGHT),
	ENUM_AND_STR(TEX_ALPHA_MODE_PREMULTIPLIED), ENUM_AND_STR(TEX_ALPHA_MODE_OPAQUE),
	ENUM_AND_STR(TEX_ALPHA_MODE_CUSTOM),
};

DXGI_FORMAT dxgi_format_from_str(const string& str) noexcept
{
	auto it = dxgi_to_str.find(str);
	if (it != dxgi_to_str.end())
	{
		return it->second;
	}
	else
	{
		return DXGI_FORMAT_UNKNOWN;
	}
}

string_view dxgi_format_to_str(DXGI_FORMAT format) noexcept
{
	auto it = str_to_format.find(format);
	if (it != str_to_format.end())
	{
		return it->second;
	}
	else
	{
		return "DXGI_FORMAT_UNKNOWN";
	}
}

string_view dxgi_alphamode_to_str(TEX_ALPHA_MODE format) noexcept
{
	auto it = alpha_mode_str.find(format);
	if (it != alpha_mode_str.end())
	{
		return it->second;
	}
	else
	{
		return "TEX_ALPHA_MODE_UNKNOWN";
	}
}

namespace ns
{
	void to_json(json& j, const tex_mapping& mapping)
	{
		j = json{ { "name", mapping.name },
			      { "regex", mapping.match_str },
			      { "dxgi-format-opaque", mapping.opaque_format_str },
			      { "dxgi-format-transparent", mapping.transparent_format_str } };
	}
	void from_json(const json& j, tex_mapping& tex_asset_type)
	{
		j.at("name").get_to(tex_asset_type.name);
		j.at("regex").get_to(tex_asset_type.match_str);

		tex_asset_type.match_regex = wregex(all(tex_asset_type.match_str));

		j.at("dxgi-format-opaque").get_to(tex_asset_type.opaque_format_str);
		tex_asset_type.opaque_format = dxgi_format_from_str(tex_asset_type.opaque_format_str);

		j.at("dxgi-format-transparent").get_to(tex_asset_type.transparent_format_str);
		tex_asset_type.transparent_format =
			dxgi_format_from_str(tex_asset_type.transparent_format_str);
	}

	void to_json(json& j, const asset_studio_meta& meta)
	{
		j = json{ { "name", meta.name }, { "tex-mappings", meta.mappings } };
	}

	void from_json(const json& j, asset_studio_meta& meta)
	{
		j.at("name").get_to(meta.name);
		j.at("tex-mappings").get_to(meta.mappings);
	}
}

void asset_builder::push(const fs::path& p)
{
	try
	{
		ifstream ifs(p);
		json     js = json::parse(ifs);

		ns::asset_studio_meta meta;
		js.get_to(meta);

		tex_mapping_context_stack.push(std::move(meta));
	}
	catch (const json::exception e)
	{
		throw asset_builder::exception(
			"File at '"s + p.string() + "' has invalid json content "s + e.what());
	}
}

void asset_builder::pop() noexcept { tex_mapping_context_stack.pop(); }

const string& asset_builder::preset_name() noexcept { return tex_mapping_context_stack.top().name; }

const string asset_builder::asset_type(const fs::path& path) noexcept
{
	if (empty())
		return "unknown";

	const auto& current_level = tex_mapping_context_stack.top();
	auto        name_wide     = path.filename().wstring();
	for_e(entry, current_level.mappings)
	{
		if (regex_match(name_wide, entry.match_regex))
		{
			return entry.name;
		}
	}

	if (path.extension() == ".nif")
	{
		return "mesh";
	}

	return "unknown";
}

bool asset_builder::empty() noexcept { return tex_mapping_context_stack.size() == 0; }

asset_ptr create_texture_asset(const fs::path& path, const ns::tex_mapping& entry);

asset_builder::task asset_builder::build(const fs::path& path)
{
	if (empty())
		return nullptr;

	const auto& current_level = tex_mapping_context_stack.top();
	auto        name_wide     = path.filename().wstring();

	for_e(entry, current_level.mappings)
	{
		if (regex_match(name_wide, entry.match_regex))
		{
			return [path, entry]() { return create_texture_asset(path, entry); };
		}
	}

	if (path.extension() == ".nif")
	{
		return nullptr;
	}

	return nullptr;
}

HRESULT analyze(const Image& image, XMFLOAT4& result)
{
	XMVECTOR minv      = g_XMFltMax;
	XMVECTOR maxv      = XMVectorNegate(g_XMFltMax);
	XMVECTOR acc       = g_XMZero;
	XMVECTOR luminance = g_XMZero;

	size_t totalPixels = 0;

	HRESULT hr = EvaluateImage(image, [&](const XMVECTOR* pixels, size_t width, size_t y) {
		static const XMVECTORF32 s_luminance = { { { 0.3f, 0.59f, 0.11f, 0.f } } };

		UNREFERENCED_PARAMETER(y);

		for (size_t x = 0; x < width; ++x)
		{
			const XMVECTOR v = *pixels++;
			minv             = XMVectorMin(minv, v);
			++totalPixels;
		}
	});
	if (FAILED(hr))
		return hr;

	if (!totalPixels)
		return S_FALSE;

	XMStoreFloat4(&result, minv);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

asset_ptr create_texture_asset(const fs::path& path, const ns::tex_mapping& entry)
{
	DXGI_FORMAT    new_format;
	const string   path_str      = path.string();
	wstring        file_path_buf = path.wstring();
	const wchar_t* file_path_raw = file_path_buf.c_str();

	TexMetadata metadata;
	if (FAILED(GetMetadataFromDDSFile(file_path_raw, DDS_FLAGS::DDS_FLAGS_NONE, metadata)))
		throw asset_builder::exception("Couldn't get the metadata from DDS file " + path_str);

	auto alphamode = metadata.GetAlphaMode();

	spdlog::info("Old format {}: {}", dxgi_format_to_str(metadata.format), path_str);

	if (alphamode == TEX_ALPHA_MODE_OPAQUE)
	{
		new_format = entry.opaque_format;
		spdlog::info("Is opaque: new format is {}", entry.opaque_format_str);
	}
	else
	{
		auto img_buf = make_unique<ScratchImage>();
		if (FAILED(LoadFromDDSFile(file_path_raw, DDS_FLAGS::DDS_FLAGS_NONE, &metadata, *img_buf)))
			throw asset_builder::exception("Couldn't load the DDS file " + path_str);

		XMFLOAT4 analysis;
		if (FAILED(analyze(*img_buf->GetImage(0, 0, 0), analysis)))
			throw asset_builder::exception("Analysis of DDS file failed " + path_str);

		if (analysis.w >= opaque_alpha_threshold)
		{
			new_format = entry.opaque_format;
			spdlog::info(
				"Minimum alpha of {} which is opaque: new format is {} for {}",
				analysis.w,
				entry.opaque_format_str,
				path_str);
		}
		else
		{
			new_format = entry.transparent_format;
			spdlog::info(
				"Transparency detected: new format is {} for {}",
				entry.transparent_format_str,
				path_str);
		}
	}

	return texture_asset::create(
		path,
		{ metadata.width, metadata.height },
		{ metadata.width, metadata.height },
		new_format,
		metadata.format);
}