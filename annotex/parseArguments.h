
#pragma once

enum AnnotexFormat {
	Auto,
	Diff,
	Mask
};

struct AnnotexParameters {
	bool verbose;
	bool outputToCurrentDir;
	AnnotexFormat format;
	uint32_t pixel_format_bpp;
	std::string sourcePath;
};

bool parseArguments(int argc, char* argv[], rdo_bc::rdo_bc_params& rp, AnnotexParameters& annotexParameters);
