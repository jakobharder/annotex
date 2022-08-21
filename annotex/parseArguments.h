
#pragma once

enum AnnotexFormat {
	Auto,
	Diff,
	Mask,
	NormRga,
	MetalR_a
};

struct AnnotexParameters {
	bool verbose;
	bool outputToCurrentDir;
	AnnotexFormat format;
	uint32_t pixel_format_bpp;
	std::string sourcePath;
	std::string targetPath;
	std::string glowMapPath;
	uint32_t lods;
};

bool parseArguments(int argc, char* argv[], rdo_bc::rdo_bc_params& rp, AnnotexParameters& annotexParameters);
std::string swapExtension(const std::string& filePath, const std::string& newExtension, bool stripPath);
