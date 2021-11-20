
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
	// TBD store mipmaps
	// TBD store lods

	string filePathWithoutExt = filePath;
	strip_extension(filePathWithoutExt);

	int idx = 0;
	for (auto mipItr = this->_encoders.begin(); mipItr != this->_encoders.end(); mipItr++, idx++)
	{
		const shared_ptr<rdo_bc::rdo_bc_encoder>& mipmap = this->_encoders[idx];
		const auto mipPath = filePathWithoutExt + "_mip" + to_string(idx) + ".dds";
		if (!save_dds(mipPath.c_str(),
			mipmap->get_orig_width(),
			mipmap->get_orig_height(),
			mipmap->get_blocks(),
			annotexParameters.pixel_format_bpp,
			rp.m_dxgi_format, rp.m_perceptual,
			annotexParameters.force_dx10_dds))
		{
			fprintf(stderr, "Failed writing file \"%s\"\n", filePath.c_str());
			return false;
		}
	}

	return true;
}
