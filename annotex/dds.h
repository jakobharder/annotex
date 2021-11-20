
#include "../rdo_bc_encoder.h"
#include "./parseArguments.h"

class CompressedImage
{
public:
	static std::shared_ptr<CompressedImage> fromEncoder(const std::vector < std::shared_ptr<rdo_bc::rdo_bc_encoder >> &encoders);

	CompressedImage::CompressedImage(std::shared_ptr<rdo_bc::rdo_bc_encoder> encoder);

	bool save(const std::string& filePath, const rdo_bc::rdo_bc_params& rp, const AnnotexParameters& annotexParameters);

private:
	std::shared_ptr<rdo_bc::rdo_bc_encoder> _encoder;
};
