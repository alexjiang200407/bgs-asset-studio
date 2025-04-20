#include "texture_asset.h"
#include "hr_exception.h"
#include <d3d11.h>
#include <dxgi.h>
#include <dxgiformat.h>
#include <wrl\client.h>

using namespace std;
using namespace DirectX;
namespace fs = filesystem;
using Microsoft::WRL::ComPtr;

bool is_srgb(DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return true;

	default:
		return false;
	}
}

bool is_typeless(DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC7_TYPELESS:
		return true;

	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return true;

	default:
		return false;
	}
}

bool has_alpha(DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
#ifdef DXGI_1_2_FORMATS
	case DXGI_FORMAT_B4G4R4A4_UNORM:
#endif
		return true;

	default:
		return false;
	}
}

bool is_depth_stencil(DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_D16_UNORM:
		return true;
	}
	return false;
}

constexpr size_t count_mips(size_t width, size_t height) noexcept
{
	size_t mipLevels = 1;

	while (height > 1 || width > 1)
	{
		if (height > 1)
			height >>= 1;

		if (width > 1)
			width >>= 1;

		++mipLevels;
	}

	return mipLevels;
}

constexpr static bool ispow2(size_t x) { return ((x != 0) && !(x & (x - 1))); }

constexpr size_t count_mips_3D(size_t width, size_t height, size_t depth) noexcept
{
	size_t mipLevels = 1;

	while (height > 1 || width > 1 || depth > 1)
	{
		if (height > 1)
			height >>= 1;

		if (width > 1)
			width >>= 1;

		if (depth > 1)
			depth >>= 1;

		++mipLevels;
	}

	return mipLevels;
}


bool get_dxgi_factory(IDXGIFactory1** pFactory)
{
	if (!pFactory)
		return false;

	*pFactory = nullptr;

	typedef HRESULT(WINAPI * pfn_CreateDXGIFactory1)(REFIID riid, _Out_ void** ppFactory);

	static pfn_CreateDXGIFactory1 s_CreateDXGIFactory1 = nullptr;

	if (!s_CreateDXGIFactory1)
	{
		HMODULE hModDXGI = LoadLibraryW(L"dxgi.dll");
		if (!hModDXGI)
			return false;

		s_CreateDXGIFactory1 = reinterpret_cast<pfn_CreateDXGIFactory1>(
			reinterpret_cast<void*>(GetProcAddress(hModDXGI, "CreateDXGIFactory1")));
		if (!s_CreateDXGIFactory1)
			return false;
	}

	return SUCCEEDED(s_CreateDXGIFactory1(IID_PPV_ARGS(pFactory)));
}

bool create_device(int adapter, ID3D11Device** pDevice)
{
	if (!pDevice)
		return false;

	*pDevice = nullptr;

	static PFN_D3D11_CREATE_DEVICE s_DynamicD3D11CreateDevice = nullptr;

	if (!s_DynamicD3D11CreateDevice)
	{
		HMODULE hModD3D11 = LoadLibraryW(L"d3d11.dll");
		if (!hModD3D11)
			return false;

		s_DynamicD3D11CreateDevice = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(
			reinterpret_cast<void*>(GetProcAddress(hModD3D11, "D3D11CreateDevice")));
		if (!s_DynamicD3D11CreateDevice)
			return false;
	}

	const D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ComPtr<IDXGIAdapter> pAdapter;
	if (adapter >= 0)
	{
		ComPtr<IDXGIFactory1> dxgiFactory;
		if (get_dxgi_factory(dxgiFactory.GetAddressOf()))
		{
			if (FAILED(
					dxgiFactory->EnumAdapters(static_cast<UINT>(adapter), pAdapter.GetAddressOf())))
			{
				spdlog::error("Invalid GPU adapter index {}", adapter);
				return false;
			}
		}
	}

	D3D_FEATURE_LEVEL fl;
	HRESULT           hr = s_DynamicD3D11CreateDevice(
        pAdapter.Get(),
        (pAdapter) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevels,
        static_cast<UINT>(std::size(featureLevels)),
        D3D11_SDK_VERSION,
        pDevice,
        &fl,
        nullptr);
	if (SUCCEEDED(hr))
	{
		if (fl < D3D_FEATURE_LEVEL_11_0)
		{
			D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts;
			hr = (*pDevice)->CheckFeatureSupport(
				D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS,
				&hwopts,
				sizeof(hwopts));
			if (FAILED(hr))
				memset(&hwopts, 0, sizeof(hwopts));

			if (!hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
			{
				if (*pDevice)
				{
					(*pDevice)->Release();
					*pDevice = nullptr;
				}
				hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		ComPtr<IDXGIDevice> dxgiDevice;
		hr = (*pDevice)->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()));
		if (SUCCEEDED(hr))
		{
			hr = dxgiDevice->GetAdapter(pAdapter.ReleaseAndGetAddressOf());
			if (SUCCEEDED(hr))
			{
				DXGI_ADAPTER_DESC desc;
				hr = pAdapter->GetDesc(&desc);
				if (SUCCEEDED(hr))
				{
					spdlog::info(
						"\n[Using DirectCompute {} on \"{}\"]",
						(fl >= D3D_FEATURE_LEVEL_11_0) ? "5.0" : "4.0",
						(char*)desc.Description);
				}
			}
		}

		return true;
	}
	else
		return false;
}

void FitPowerOf2(
	size_t          origx,
	size_t          origy,
	_Inout_ size_t& targetx,
	_Inout_ size_t& targety,
	size_t          maxsize)
{
	const float origAR = float(origx) / float(origy);

	if (origx > origy)
	{
		size_t x;
		for (x = maxsize; x > 1; x >>= 1)
		{
			if (x <= targetx)
				break;
		}
		targetx = x;

		float bestScore = FLT_MAX;
		for (size_t y = maxsize; y > 0; y >>= 1)
		{
			const float score = fabsf((float(x) / float(y)) - origAR);
			if (score < bestScore)
			{
				bestScore = score;
				targety   = y;
			}
		}
	}
	else
	{
		size_t y;
		for (y = maxsize; y > 1; y >>= 1)
		{
			if (y <= targety)
				break;
		}
		targety = y;

		float bestScore = FLT_MAX;
		for (size_t x = maxsize; x > 0; x >>= 1)
		{
			const float score = fabsf((float(x) / float(y)) - origAR);
			if (score < bestScore)
			{
				bestScore = score;
				targetx   = x;
			}
		}
	}
}

texture_asset::texture_asset(
	const fs::path& path,
	texture_size    size,
	texture_size    old_size,
	DXGI_FORMAT     format,
	DXGI_FORMAT     old_format) :
	asset(asset::type::TEXTURE, path),
	format(format), old_format(old_format), size(size), old_size(old_size)
{}

asset_ptr texture_asset::create(
	const fs::path& path,
	texture_size    size,
	texture_size    old_size,
	DXGI_FORMAT     format,
	DXGI_FORMAT     old_format)
{
	return make_shared<texture_asset>(texture_asset(path, size, old_size, format, old_format));
}

void texture_asset::process() const
{
	spdlog::info("Processing {}", path.string());

	wstring              file_path_buf = path.wstring();
	const wchar_t*       file_path_raw = file_path_buf.c_str();
	size_t               mipLevels     = 0;  // TODO put this in struct
	HRESULT              hr;
	TEX_FILTER_FLAGS     dwFilter              = TEX_FILTER_DEFAULT;
	TEX_FILTER_FLAGS     dwSRGB                = TEX_FILTER_DEFAULT;
	TEX_FILTER_FLAGS     dwFilterOpts          = TEX_FILTER_DEFAULT;
	TEX_FILTER_FLAGS     dwConvert             = TEX_FILTER_DEFAULT;
	TEX_COMPRESS_FLAGS   dwCompress            = TEX_COMPRESS_DEFAULT;
	bool                 preserveAlphaCoverage = false;
	bool                 dxt5nm                = false;
	bool                 dxt5rxgb              = false;
	float                alphaThreshold        = TEX_THRESHOLD_DEFAULT;
	bool                 non4bc                = false;
	ComPtr<ID3D11Device> pDevice;
	int                  adapter     = -1;
	float                alphaWeight = 1.f;

	TexMetadata                   info;
	std::unique_ptr<ScratchImage> image(new (std::nothrow) ScratchImage);

	THROW_HR_EXCEPTION(
		LoadFromDDSFile(file_path_raw, DDS_FLAGS_NONE, &info, *image),
		"Couldn't load file at " + path.string());

	if (IsTypeless(info.format))
	{
		info.format = MakeTypelessUNORM(info.format);

		if (IsTypeless(info.format))
		{
			throw runtime_error("FAILED due to Typeless format");
		}

		image->OverrideFormat(info.format);
	}

	size_t tMips = (!mipLevels && info.mipLevels > 1) ? info.mipLevels : mipLevels;

	if (IsPlanar(info.format))
	{
		auto img = image->GetImage(0, 0, 0);
		assert(img);
		const size_t nimg = image->GetImageCount();

		std::unique_ptr<ScratchImage> timage(new (std::nothrow) ScratchImage);

		THROW_HR_EXCEPTION(
			ConvertToSinglePlane(img, nimg, info, *timage),
			"Failet to convert file to single plane " + path.string());

		auto& tinfo = timage->GetMetadata();

		info.format = tinfo.format;

		assert(info.width == tinfo.width);
		assert(info.height == tinfo.height);
		assert(info.depth == tinfo.depth);
		assert(info.arraySize == tinfo.arraySize);
		assert(info.mipLevels == tinfo.mipLevels);
		assert(info.miscFlags == tinfo.miscFlags);
		assert(info.dimension == tinfo.dimension);

		image.swap(timage);
	}
	DXGI_FORMAT tformat = (format == DXGI_FORMAT_UNKNOWN) ? info.format : format;

	std::unique_ptr<ScratchImage> cimage;
	if (IsCompressed(info.format))
	{
		// Direct3D can only create BC resources with multiple-of-4 top levels
		if ((info.width % 4) != 0 || (info.height % 4) != 0)
		{
			throw runtime_error(
				"Can only make BC resources with multiple-of-4 top levels " + path.string());
		}

		auto img = image->GetImage(0, 0, 0);
		assert(img);
		const size_t nimg = image->GetImageCount();

		std::unique_ptr<ScratchImage> timage(new (std::nothrow) ScratchImage);

		THROW_HR_EXCEPTION(
			Decompress(img, nimg, info, DXGI_FORMAT_UNKNOWN /* picks good default */, *timage),
			"Failed to decompress " + path.string());

		auto& tinfo = timage->GetMetadata();

		info.format = tinfo.format;

		assert(info.width == tinfo.width);
		assert(info.height == tinfo.height);
		assert(info.depth == tinfo.depth);
		assert(info.arraySize == tinfo.arraySize);
		assert(info.mipLevels == tinfo.mipLevels);
		assert(info.miscFlags == tinfo.miscFlags);
		assert(info.dimension == tinfo.dimension);

		// Keep the original compressed image in case we can reuse it
		cimage.reset(image.release());
		image.reset(timage.release());
	}

	// Undo Premultiplied Alpha?

	size_t width  = size.first;
	size_t height = size.second;

	size_t twidth  = (!width) ? info.width : width;
	size_t theight = (!height) ? info.height : height;

	if (info.width != twidth || info.height != theight)
	{
		std::unique_ptr<ScratchImage> timage(new (std::nothrow) ScratchImage);

		THROW_HR_EXCEPTION(
			Resize(
				image->GetImages(),
				image->GetImageCount(),
				image->GetMetadata(),
				twidth,
				theight,
				TEX_FILTER_DEFAULT,
				*timage),
			"Failed to resize " + path.string());

		auto& tinfo = timage->GetMetadata();

		assert(tinfo.width == twidth && tinfo.height == theight && tinfo.mipLevels == 1);
		info.width     = tinfo.width;
		info.height    = tinfo.height;
		info.mipLevels = 1;

		assert(info.depth == tinfo.depth);
		assert(info.arraySize == tinfo.arraySize);
		assert(info.miscFlags == tinfo.miscFlags);
		assert(info.format == tinfo.format);
		assert(info.dimension == tinfo.dimension);

		image.swap(timage);
		cimage.reset();

		if (tMips > 0)
		{
			const size_t maxMips = (info.depth > 1) ?
			                           count_mips_3D(info.width, info.height, info.depth) :
			                           count_mips(info.width, info.height);

			if (tMips > maxMips)
			{
				tMips = maxMips;
			}
		}
	}

	if (info.format != tformat && !IsCompressed(tformat))
	{
		std::unique_ptr<ScratchImage> timage(new (std::nothrow) ScratchImage);

		THROW_HR_EXCEPTION(
			Convert(
				image->GetImages(),
				image->GetImageCount(),
				image->GetMetadata(),
				tformat,
				TEX_FILTER_DEFAULT,
				TEX_THRESHOLD_DEFAULT,
				*timage),
			"Failed to convert " + path.string());

		auto& tinfo = timage->GetMetadata();

		assert(tinfo.format == tformat);
		info.format = tinfo.format;

		assert(info.width == tinfo.width);
		assert(info.height == tinfo.height);
		assert(info.depth == tinfo.depth);
		assert(info.arraySize == tinfo.arraySize);
		assert(info.mipLevels == tinfo.mipLevels);
		assert(info.miscFlags == tinfo.miscFlags);
		assert(info.dimension == tinfo.dimension);

		image.swap(timage);
		cimage.reset();
	}

	TEX_FILTER_FLAGS dwFilter3D = dwFilter;
	if (!ispow2(info.width) || !ispow2(info.height) || !ispow2(info.depth))
	{
		if (!tMips || info.mipLevels != 1)
		{
			throw runtime_error("Not to the power of 2");
		}

		if (info.dimension == TEX_DIMENSION_TEXTURE3D)
		{
			// Must force triangle filter for non-power-of-2 volume textures to get correct results
			dwFilter3D = TEX_FILTER_TRIANGLE;
		}
	}

	if ((!tMips || info.mipLevels != tMips || preserveAlphaCoverage) && (info.mipLevels != 1))
	{
		// Mips generation only works on a single base image, so strip off existing mip levels
		// Also required for preserve alpha coverage so that existing mips are regenerated

		std::unique_ptr<ScratchImage> timage(new (std::nothrow) ScratchImage);

		TexMetadata mdata = info;
		mdata.mipLevels   = 1;
		THROW_HR_EXCEPTION(
			timage->Initialize(mdata),
			"Failed to initialize metadata for file " + path.string());

		if (info.dimension == TEX_DIMENSION_TEXTURE3D)
		{
			for (size_t d = 0; d < info.depth; ++d)
			{
				hr = CopyRectangle(
					*image->GetImage(0, 0, d),
					Rect(0, 0, info.width, info.height),
					*timage->GetImage(0, 0, d),
					TEX_FILTER_DEFAULT,
					0,
					0);
			}
		}
		else
		{
			for (size_t i = 0; i < info.arraySize; ++i)
			{
				hr = CopyRectangle(
					*image->GetImage(0, i, 0),
					Rect(0, 0, info.width, info.height),
					*timage->GetImage(0, i, 0),
					TEX_FILTER_DEFAULT,
					0,
					0);
			}
		}
		THROW_HR_EXCEPTION(hr, "Failed to copy rectangle for file " + path.string());

		image.swap(timage);
		info.mipLevels = 1;

		if (cimage && (tMips == 1))
		{
			// Special case for trimming mips off compressed images and keeping the original compressed highest level mip
			mdata           = cimage->GetMetadata();
			mdata.mipLevels = 1;
			THROW_HR_EXCEPTION(
				timage->Initialize(mdata),
				"Failed to initialize metadata for file " + path.string());

			if (mdata.dimension == TEX_DIMENSION_TEXTURE3D)
			{
				for (size_t d = 0; d < mdata.depth; ++d)
				{
					auto simg = cimage->GetImage(0, 0, d);
					auto dimg = timage->GetImage(0, 0, d);

					memcpy_s(dimg->pixels, dimg->slicePitch, simg->pixels, simg->slicePitch);
				}
			}
			else
			{
				for (size_t i = 0; i < mdata.arraySize; ++i)
				{
					auto simg = cimage->GetImage(0, i, 0);
					auto dimg = timage->GetImage(0, i, 0);

					memcpy_s(dimg->pixels, dimg->slicePitch, simg->pixels, simg->slicePitch);
				}
			}

			cimage.swap(timage);
		}
		else
		{
			cimage.reset();
		}
	}

	if ((!tMips || info.mipLevels != tMips) &&
	    (info.width > 1 || info.height > 1 || info.depth > 1))
	{
		std::unique_ptr<ScratchImage> timage(new (std::nothrow) ScratchImage);

		if (info.dimension == TEX_DIMENSION_TEXTURE3D)
		{
			hr = GenerateMipMaps3D(
				image->GetImages(),
				image->GetImageCount(),
				image->GetMetadata(),
				dwFilter3D | dwFilterOpts | TEX_FILTER_FORCE_NON_WIC,
				tMips,
				*timage);
		}
		else
		{
			hr = GenerateMipMaps(
				image->GetImages(),
				image->GetImageCount(),
				image->GetMetadata(),
				dwFilter | dwFilterOpts | TEX_FILTER_FORCE_NON_WIC,
				tMips,
				*timage);
		}
		THROW_HR_EXCEPTION(hr, "Failed to generate mipmaps for image " + path.string());

		auto& tinfo    = timage->GetMetadata();
		info.mipLevels = tinfo.mipLevels;

		assert(info.width == tinfo.width);
		assert(info.height == tinfo.height);
		assert(info.depth == tinfo.depth);
		assert(info.arraySize == tinfo.arraySize);
		assert(info.miscFlags == tinfo.miscFlags);
		assert(info.format == tinfo.format);
		assert(info.dimension == tinfo.dimension);

		image.swap(timage);
		cimage.reset();
	}

	if (dxt5nm || dxt5rxgb)
	{
		// Prepare for DXT5nm/RXGB
		assert(tformat == DXGI_FORMAT_BC3_UNORM);

		std::unique_ptr<ScratchImage> timage(new (std::nothrow) ScratchImage);
		if (!timage)
		{
			wprintf(L"\nERROR: Memory allocation failed\n");
			return;
		}

		if (dxt5nm)
		{
			THROW_HR_EXCEPTION(
				TransformImage(
					image->GetImages(),
					image->GetImageCount(),
					image->GetMetadata(),
					[=](XMVECTOR* outPixels, const XMVECTOR* inPixels, size_t w, size_t y) {
						UNREFERENCED_PARAMETER(y);

						for (size_t j = 0; j < w; ++j)
						{
							outPixels[j] = XMVectorPermute<4, 1, 5, 0>(inPixels[j], g_XMIdentityR0);
						}
					},
					*timage),
				"Failed to transform DXT5nm for file " + path.string());
		}
		else
		{
			THROW_HR_EXCEPTION(
				TransformImage(
					image->GetImages(),
					image->GetImageCount(),
					image->GetMetadata(),
					[=](XMVECTOR* outPixels, const XMVECTOR* inPixels, size_t w, size_t y) {
						UNREFERENCED_PARAMETER(y);

						for (size_t j = 0; j < w; ++j)
						{
							outPixels[j] = XMVectorSwizzle<3, 1, 2, 0>(inPixels[j]);
						}
					},
					*timage),
				"Failed to transform DXT5 RXGB for file " + path.string());
		}

#ifndef NDEBUG
		auto& tinfo = timage->GetMetadata();
#endif

		assert(info.width == tinfo.width);
		assert(info.height == tinfo.height);
		assert(info.depth == tinfo.depth);
		assert(info.arraySize == tinfo.arraySize);
		assert(info.mipLevels == tinfo.mipLevels);
		assert(info.miscFlags == tinfo.miscFlags);
		assert(info.format == tinfo.format);
		assert(info.dimension == tinfo.dimension);

		image.swap(timage);
		cimage.reset();
	}

	if (IsCompressed(tformat))
	{
		if (cimage && (cimage->GetMetadata().format == tformat))
		{
			// We never changed the image and it was already compressed in our desired format, use original data
			image.reset(cimage.release());

			auto& tinfo = image->GetMetadata();

			if ((tinfo.width % 4) != 0 || (tinfo.height % 4) != 0)
			{
				non4bc = true;
			}

			info.format = tinfo.format;
			assert(info.width == tinfo.width);
			assert(info.height == tinfo.height);
			assert(info.depth == tinfo.depth);
			assert(info.arraySize == tinfo.arraySize);
			assert(info.mipLevels == tinfo.mipLevels);
			assert(info.miscFlags == tinfo.miscFlags);
			assert(info.dimension == tinfo.dimension);
		}
		else
		{
			cimage.reset();

			auto img = image->GetImage(0, 0, 0);
			assert(img);
			const size_t nimg = image->GetImageCount();

			std::unique_ptr<ScratchImage> timage(new (std::nothrow) ScratchImage);
			if (!timage)
			{
				wprintf(L"\nERROR: Memory allocation failed\n");
				return;
			}

			bool bc6hbc7 = false;
			switch (tformat)
			{
			case DXGI_FORMAT_BC6H_TYPELESS:
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
			case DXGI_FORMAT_BC7_TYPELESS:
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				bc6hbc7 = true;

				{
					static bool s_tryonce = false;

					if (!s_tryonce)
					{
						if (!create_device(adapter, pDevice.GetAddressOf()))
							spdlog::warn(
								"WARNING: DirectCompute is not available, using BC6H / BC7 "
								"CPU codec");
					}
				}
				break;

			default:
				break;
			}

			TEX_COMPRESS_FLAGS cflags = dwCompress;

			if ((img->width % 4) != 0 || (img->height % 4) != 0)
			{
				non4bc = true;
			}

			if (bc6hbc7 && pDevice)
			{
				hr = Compress(
					pDevice.Get(),
					img,
					nimg,
					info,
					tformat,
					dwCompress | dwSRGB,
					alphaWeight,
					*timage);
			}
			else
			{
				hr = Compress(img, nimg, info, tformat, cflags | dwSRGB, alphaThreshold, *timage);
			}
			THROW_HR_EXCEPTION(hr, "Compression failed for file " + path.string());

			auto& tinfo = timage->GetMetadata();

			info.format = tinfo.format;
			assert(info.width == tinfo.width);
			assert(info.height == tinfo.height);
			assert(info.depth == tinfo.depth);
			assert(info.arraySize == tinfo.arraySize);
			assert(info.mipLevels == tinfo.mipLevels);
			assert(info.miscFlags == tinfo.miscFlags);
			assert(info.dimension == tinfo.dimension);

			image.swap(timage);
		}
	}

	if (HasAlpha(info.format) && info.format != DXGI_FORMAT_A8_UNORM)
	{
		if (dxt5nm || dxt5rxgb)
		{
			info.SetAlphaMode(TEX_ALPHA_MODE_CUSTOM);
		}
		else if (image->IsAlphaAllOpaque())
		{
			info.SetAlphaMode(TEX_ALPHA_MODE_OPAQUE);
		}
		else if (info.IsPMAlpha())
		{
			// Aleady set TEX_ALPHA_MODE_PREMULTIPLIED
		}
		//else if (dwOptions & (UINT64_C(1) << OPT_SEPALPHA))
		//{
		//	info.SetAlphaMode(TEX_ALPHA_MODE_CUSTOM);
		//}
		else if (info.GetAlphaMode() == TEX_ALPHA_MODE_UNKNOWN)
		{
			info.SetAlphaMode(TEX_ALPHA_MODE_STRAIGHT);
		}
	}
	else
	{
		info.SetAlphaMode(TEX_ALPHA_MODE_UNKNOWN);
	}

	auto img = image->GetImage(0, 0, 0);
	assert(img);
	const size_t nimg     = image->GetImageCount();
	DDS_FLAGS    ddsFlags = DDS_FLAGS_NONE;
	THROW_HR_EXCEPTION(
		SaveToDDSFile(img, nimg, info, ddsFlags, file_path_raw),
		"Failed to save to DDS file at " + path.string());
}
