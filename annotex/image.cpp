
#include <string>
#include <memory>
#include "../rdo_bc_encoder.h"
#include "./image.h"
#include "./dds.h"

using namespace std;
using namespace utils;

std::shared_ptr<Image> Image::fromFile(const string& filePath)
{
	auto img = make_shared<Image>();
	if (!load_png(filePath.c_str(), img->main))
		return nullptr;
	return img;
}

void Image::generateMipMaps()
{
	// super simple box downscale, optimal for pow2 textures or which are at least a sum of two pow2 numbers

	image_u8 const* current = &this->main;

	while (current->width() > 1 || current->height() > 1) {
		const int width = max<uint32_t>(1, current->width() / 2);
		const int height = max<uint32_t>(1, current->height() / 2);
		this->mipmaps.push_back(make_unique<image_u8>(width, height));
		image_u8& nextMip = *this->mipmaps.back();

		auto& sourcePixels = current->get_pixels();
		auto& targetPixels = nextMip.get_pixels();

		const int factorX = width == (int)current->width() ? 1 : 2;
		const int factorY = height == (int)current->height() ? 1 : 2;
#pragma omp parallel for
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				const auto& aa = sourcePixels[y * factorY * width * factorX + x * factorX];
				const auto& ab = sourcePixels[((y+1) * factorY - 1) * width * factorX + x * factorX];
				const auto& ba = sourcePixels[y * factorY * width * factorX + (x+1) * factorX - 1];
				const auto& bb = sourcePixels[((y+1) * factorY - 1) * width * factorX + (x+1) * factorX - 1];

				targetPixels[y * width + x].a = ((uint32_t)aa.a + ab.a + ba.a + bb.a + 2) / 4;
				targetPixels[y * width + x].r = ((uint32_t)aa.r + ab.r + ba.r + bb.r + 2) / 4;
				targetPixels[y * width + x].g = ((uint32_t)aa.g + ab.g + ba.g + bb.g + 2) / 4;
				targetPixels[y * width + x].b = ((uint32_t)aa.b + ab.b + ba.b + bb.b + 2) / 4;
			}
		}

		current = &nextMip;
	}
}

shared_ptr<CompressedImage> Image::compress(rdo_bc::rdo_bc_params& rp)
{
	vector<image_u8> maps{ main };
	for (auto& mipmap : this->mipmaps)
		maps.push_back(*mipmap);

	vector<shared_ptr<rdo_bc::rdo_bc_encoder>> encoders;
	for (auto& mipmap : maps)
	{
		auto encoder = make_shared<rdo_bc::rdo_bc_encoder>();
		if (!encoder->init(mipmap, rp))
		{
			fprintf(stderr, "rdo_bc_encoder::init() failed!\n");
			return nullptr;
		}

		if (!encoder->encode())
		{
			fprintf(stderr, "rdo_bc_encoder::encode() failed!\n");
			return nullptr;
		}

		encoders.push_back(move(encoder));
	}

	return CompressedImage::fromEncoder(encoders);
}
