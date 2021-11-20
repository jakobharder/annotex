#include <memory>
#include "rdo_bc_encoder.h"
#include "./annotex/parseArguments.h"
#include "./annotex/image.h"
#include "./annotex/dds.h"

using namespace std;
using namespace utils;

string swapExtension(const string& filePath, const string& newExtension, bool stripPath) {
	string result = filePath;
	strip_extension(result);
	if (stripPath)
	{
		strip_path(result);
	}
	result += newExtension;
	return result;
}

int main(int argc, char* argv[])
{
	AnnotexParameters annotexParameters;
	rdo_bc::rdo_bc_params rp;
	if (!parseArguments(argc, argv, rp, annotexParameters))
		return EXIT_FAILURE;

	auto measureTimeStart = clock();

	// read
	auto sourceImage = Image::fromFile(annotexParameters.sourcePath);
	if (!sourceImage)
		return EXIT_FAILURE;
	printf("=> %s %ux%u\n", annotexParameters.sourcePath.c_str(), sourceImage->main.width(), sourceImage->main.height());

	// process
	sourceImage->generateMipMaps();
	auto compressedImage = sourceImage->compress(rp);
	if (!compressedImage)
		return EXIT_FAILURE;

	// write
	if (annotexParameters.lods == 0)
	{
		auto targetPath = swapExtension(annotexParameters.sourcePath, ".dds", annotexParameters.outputToCurrentDir);
		if (!compressedImage->save(targetPath, rp, annotexParameters, 0))
			return EXIT_FAILURE;
		printf("<= %s\n", targetPath.c_str());
	}
	else
	{
		for (uint32_t lod = 0; lod < min(annotexParameters.lods, compressedImage->getMipMapCount()); lod++)
		{
			auto targetPath = swapExtension(annotexParameters.sourcePath, "_" + to_string(lod) + ".dds", annotexParameters.outputToCurrentDir);
			if (!compressedImage->save(targetPath, rp, annotexParameters, lod))
				return EXIT_FAILURE;
			printf("<= LOD %u: %s\n", lod, targetPath.c_str());
		}
	}

	auto measureTimeEnd = clock();
	if (annotexParameters.verbose)
		printf("Total processing time: %f secs\n", (double)(measureTimeEnd - measureTimeStart) / CLOCKS_PER_SEC);

	return EXIT_SUCCESS;
}
