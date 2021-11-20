
#include <string>
#include <memory>
#include "../rdo_bc_encoder.h"
#include "./dds.h"
#include "./parseArguments.h"

using namespace std;
using namespace utils;

shared_ptr<CompressedImage> CompressedImage::fromEncoder(const vector<shared_ptr<rdo_bc::rdo_bc_encoder>>& encoders)
{
	if (!encoders.size() || !encoders[0])
		return nullptr;
	return make_shared<CompressedImage>(encoders);
}

CompressedImage::CompressedImage(vector<shared_ptr<rdo_bc::rdo_bc_encoder>> encoders)
{
	_encoders = encoders;
}

bool CompressedImage::save(const string& filePath, const rdo_bc::rdo_bc_params& rp, const AnnotexParameters& annotexParameters)
{
	// TBD store lods

	//string filePathWithoutExt = filePath;
	//strip_extension(filePathWithoutExt);

	FILE* pFile = NULL;
#ifdef _MSC_VER
	fopen_s(&pFile, filePath.c_str(), "wb");
#else
	pFile = fopen(filePath.c_str(), "wb");
#endif
	if (!pFile)
	{
		fprintf(stderr, "Failed creating file %s!\n", filePath.c_str());
		return false;
	}

	fwrite("DDS ", 4, 1, pFile);

	DDSURFACEDESC2 desc;
	memset(&desc, 0, sizeof(desc));

	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;

	desc.dwWidth = this->_encoders[0]->get_orig_width();
	desc.dwHeight = this->_encoders[0]->get_orig_height();

	desc.dwMipMapCount = (uint32_t)this->_encoders.size();

	desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);

	desc.ddpfPixelFormat.dwFlags |= DDPF_FOURCC;

	desc.lPitch = (((desc.dwWidth + 3) & ~3) * ((desc.dwHeight + 3) & ~3) * annotexParameters.pixel_format_bpp) >> 3;
	desc.dwFlags |= DDSD_LINEARSIZE;

	desc.ddpfPixelFormat.dwRGBBitCount = 0;

	if (rp.m_dxgi_format == DXGI_FORMAT_BC1_UNORM)
	{
		desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('D', 'X', 'T', '1');
		fwrite(&desc, sizeof(desc), 1, pFile);
	}
	else
	{
		desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('D', 'X', '1', '0');
		fwrite(&desc, sizeof(desc), 1, pFile);

		DDS_HEADER_DXT10 hdr10;
		memset(&hdr10, 0, sizeof(hdr10));

		// Not all tools support DXGI_FORMAT_BC7_UNORM_SRGB (like NVTT), but ddsview in DirectXTex pays attention to it. So not sure what to do here.
		// For best compatibility just write DXGI_FORMAT_BC7_UNORM.
		//hdr10.dxgiFormat = srgb ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
		hdr10.dxgiFormat = rp.m_dxgi_format; // DXGI_FORMAT_BC7_UNORM;
		hdr10.resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
		hdr10.arraySize = 1;

		fwrite(&hdr10, sizeof(hdr10), 1, pFile);
	}

	for (auto& map : this->_encoders)
		fwrite(map->get_blocks(), map->get_total_blocks_size_in_bytes(), 1, pFile);

	if (fclose(pFile) == EOF)
	{
		fprintf(stderr, "Failed writing to DDS file %s!\n", filePath.c_str());
		return false;
	}

	return true;
}
