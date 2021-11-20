
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
	return make_shared<CompressedImage>(encoders[0]);
}

CompressedImage::CompressedImage(shared_ptr<rdo_bc::rdo_bc_encoder> encoder)
{
	_encoder = encoder;
}

bool CompressedImage::save(const string& filePath, const rdo_bc::rdo_bc_params& rp, const AnnotexParameters& annotexParameters)
{
	// TBD store mipmaps
	// TBD store lods

	if (!save_dds(filePath.c_str(),
		this->_encoder->get_orig_width(),
		this->_encoder->get_orig_height(),
		this->_encoder->get_blocks(),
		annotexParameters.pixel_format_bpp,
		rp.m_dxgi_format, rp.m_perceptual,
		annotexParameters.force_dx10_dds))
	{
		fprintf(stderr, "Failed writing file \"%s\"\n", filePath.c_str());
		return false;
	}

	return true;
}
