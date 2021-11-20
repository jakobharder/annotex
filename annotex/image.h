
#include "../rdo_bc_encoder.h"

class CompressedImage;

class Image
{
public:
	static std::shared_ptr<Image> fromFile(const std::string& filePath);

	utils::image_u8 main;
	std::vector<utils::image_u8> mipmaps;

	void generateMipMaps();
	std::shared_ptr<CompressedImage> compress(rdo_bc::rdo_bc_params& rp);

private:
};
