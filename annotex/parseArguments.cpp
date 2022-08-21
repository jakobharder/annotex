
#define ANNOTEX_VERSION "1.2.0"
#define BC7ENC_VERSION "1.08"

#if _OPENMP
#include <omp.h>
#endif

#include <string>
#include <memory>
#include "../rdo_bc_encoder.h"
#include "../utils.h"
#include "./parseArguments.h"

using namespace std;

static void printUsage()
{
	printf("\nUsage: annotex.exe [options] input.png\n\n");
	printf("-f=<format>  Specify encoding format. Default: auto\n");
	printf("   auto: Select below formats based on file ending.\n");
	printf("   diff: Encode to BC7 (default).\n");
	printf("   mask: Encode to BC1.\n");
	printf("         Extension for auto: '_mask.png'.\n");
	printf("   rga:  Encode normal map to BC7. Converts blue to alpha (glossiness).\n");
	printf("         Extension for auto: '_rga.png'.\n");
	printf("         Optional: uses blue channel from '_r_a+b.png' for glow.\n");
	printf("   r_a:  Encode metal map to BC7. Converts blue to alpha (ambient occlusion).\n");
	printf("         Extension for auto: '_r_a.png' or '_r_a+b.png'\n");
	printf("-l=<0-9>  Number of LOD to generate with '_<LOD>.png' ending.\n");
	printf("          0 will disable LODs (default)\n");
	printf("-v  verbose\n");
}

static bool invalidUsage(const char* pArg)
{
	fprintf(stderr, "Invalid argument: %s\n", pArg);
	printUsage();
	return false;
}

static bool checkFile(const std::string& filePath)
{
	struct stat buffer;
	return (stat(filePath.c_str(), &buffer) == 0);
}

static string swapEnding(const string& filePath, const string& oldEnding, const string& newEnding)
{
	return filePath.substr(0, filePath.length() - oldEnding.length()) + newEnding;
}

static bool endsWith(const std::string& filePath, const std::string& ending)
{
	return 0 == filePath.compare(filePath.length() - ending.length(), ending.length(), ending);
}

string swapExtension(const string& filePath, const string& newExtension, bool stripPath) {
	string result = filePath;
	utils::strip_extension(result);
	if (stripPath)
	{
		utils::strip_path(result);
	}
	result += newExtension;
	return result;
}

bool parseArguments(int argc, char* argv[], rdo_bc::rdo_bc_params& rp, AnnotexParameters& annotexParameters)
{
#if _OPENMP
	const int MAX_THREADS = min(max(1, omp_get_max_threads()), 128);
#else
	const int MAX_THREADS = 1;
#endif
	rp.m_rdo_max_threads = MAX_THREADS;
	rp.m_use_bc1_3color_mode_for_black = false;
	annotexParameters.verbose = false;
	annotexParameters.outputToCurrentDir = true;
	annotexParameters.format = AnnotexFormat::Auto;
	annotexParameters.lods = 0;


	for (int i = 0; i < argc; i++)
		if (strcmp(argv[i], "-v") == 0)
			annotexParameters.verbose = true;

	if (annotexParameters.verbose || argc < 2)
		printf("annotex %s - Anno DDS texture encoder (based on bc7enc v%s)\n", ANNOTEX_VERSION, BC7ENC_VERSION);
	if (argc < 2)
	{
		printUsage();
		return false;
	}

	for (int i = 1; i < argc; i++)
	{
		const char* pArg = argv[i];
		if (pArg[0] == '-')
		{
			switch (pArg[1])
			{
			case 'v':
			{
				break;
			}
			case 'f':
			{
				if (0 == strcmp(pArg + 2, "=auto")) {
					annotexParameters.format = AnnotexFormat::Auto;
					rp.m_dxgi_format = DXGI_FORMAT_BC7_UNORM;
					annotexParameters.pixel_format_bpp = 8;
				}
				else if (0 == strcmp(pArg + 2, "=diff"))
				{
					annotexParameters.format = AnnotexFormat::Diff;
					rp.m_dxgi_format = DXGI_FORMAT_BC7_UNORM;
					annotexParameters.pixel_format_bpp = 8;
				}
				else if (0 == strcmp(pArg + 2, "=rga"))
				{
					annotexParameters.format = AnnotexFormat::NormRga;
					rp.m_dxgi_format = DXGI_FORMAT_BC7_UNORM;
					annotexParameters.pixel_format_bpp = 8;
				}
				else if (0 == strcmp(pArg + 2, "=r_a"))
				{
					annotexParameters.format = AnnotexFormat::MetalR_a;
					rp.m_dxgi_format = DXGI_FORMAT_BC7_UNORM;
					annotexParameters.pixel_format_bpp = 8;
				}
				else if (0 == strcmp(pArg + 2, "=mask"))
				{
					annotexParameters.format = AnnotexFormat::Mask;
					rp.m_dxgi_format = DXGI_FORMAT_BC1_UNORM;
					annotexParameters.pixel_format_bpp = 4;
				}
				else
					return invalidUsage(pArg);
				break;
			}
			case 'l':
			{
				if (0 != strncmp(pArg + 2, "=", 1))
					return invalidUsage(pArg);
				const int lods = atoi(pArg + 3);
				if (lods < 0 || lods > 9)
					return invalidUsage(pArg);
				annotexParameters.lods = (uint32_t)lods;
				break;
			}
			default:
			{
				return invalidUsage(pArg);
			}
			}
		}
		else
		{
			if (!annotexParameters.sourcePath.size())
			{
				annotexParameters.sourcePath = pArg;
			}
			else
			{
				fprintf(stderr, "Invalid argument: %s\n", pArg);
				return false;
			}
		}
	}

	rp.m_status_output = annotexParameters.verbose;

	if (!annotexParameters.sourcePath.size())
	{
		fprintf(stderr, "No source filename specified!\n");
		return false;
	}

	const string RGA_ENDING = "_rga.png";
	const string R_A_B_ENDING = "_r_a+b.png";
	if (annotexParameters.format == AnnotexFormat::Auto)
	{
		if (endsWith(annotexParameters.sourcePath, "_mask.png"))
		{
			rp.m_dxgi_format = DXGI_FORMAT_BC1_UNORM;
			annotexParameters.pixel_format_bpp = 4;
			annotexParameters.format = AnnotexFormat::Mask;
		}
		else if (endsWith(annotexParameters.sourcePath, RGA_ENDING))
		{
			annotexParameters.format = AnnotexFormat::NormRga;
		}
		else if (endsWith(annotexParameters.sourcePath, "_r_a.png"))
		{
			annotexParameters.format = AnnotexFormat::MetalR_a;
		}
		else if (endsWith(annotexParameters.sourcePath, R_A_B_ENDING))
		{
			annotexParameters.format = AnnotexFormat::MetalR_a;
		}
		else
			annotexParameters.format = AnnotexFormat::Diff;
	}

	annotexParameters.targetPath = annotexParameters.sourcePath;
	if (annotexParameters.format == AnnotexFormat::NormRga)
	{
		annotexParameters.targetPath = swapEnding(annotexParameters.targetPath, RGA_ENDING, "_norm.png");
	}
	else if (annotexParameters.format == AnnotexFormat::MetalR_a)
	{
		annotexParameters.targetPath = swapEnding(
			annotexParameters.targetPath, 
			endsWith(annotexParameters.sourcePath, R_A_B_ENDING) ? R_A_B_ENDING : RGA_ENDING, 
			"_metal.png");
	}
	annotexParameters.targetPath = swapExtension(annotexParameters.targetPath, ".dds", annotexParameters.outputToCurrentDir);

	// check r_a+b
	if (annotexParameters.format == AnnotexFormat::NormRga)
	{
		const std::string glowMapPath = swapEnding(annotexParameters.sourcePath, RGA_ENDING, R_A_B_ENDING);
		if (checkFile(glowMapPath))
		{
			annotexParameters.glowMapPath = glowMapPath;
		}
	}

	return true;
}
