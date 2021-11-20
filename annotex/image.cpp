
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
	// TBD
}

shared_ptr<CompressedImage> Image::compress(rdo_bc::rdo_bc_params& rp)
{
	vector<image_u8> maps{ main };
	maps.insert(maps.end(), make_move_iterator(this->mipmaps.begin()), make_move_iterator(this->mipmaps.end()));

	vector<shared_ptr<rdo_bc::rdo_bc_encoder>> encoders;
	for (auto mapItr = maps.begin(); mapItr != maps.end(); mapItr++)
	{
		auto encoder = make_shared<rdo_bc::rdo_bc_encoder>();
		if (!encoder->init(*mapItr, rp))
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
