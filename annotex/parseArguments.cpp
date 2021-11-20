
#define ANNOTEX_VERSION "1.0.0"
#define BC7ENC_VERSION "1.08"

#if _OPENMP
#include <omp.h>
#endif

#include <string>
#include <memory>
#include "../rdo_bc_encoder.h"
#include "./parseArguments.h"

using namespace std;

static void printUsage()
{
	printf("\nUsage: annotex.exe [options] input.png\n\n");
	printf("-f=<format> Specify encoding format. Default: auto\n");
	printf("   diff: Encode to BC7.\n");
	printf("   mask: Encode to BC1.\n");
	printf("   auto: Encode to BC1 if the file ends with '_mask.png', otherwise encode to BC7.\n");
	printf("-l=<0-9> Number of LOD to generate with '_<LOD>.png' ending. 0 will disable LODs. Default: 0\n");
	printf("-v verbose\n");
}

static bool invalidUsage(const char* pArg)
{
	fprintf(stderr, "Invalid argument: %s\n", pArg);
	printUsage();
	return false;
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

	if (annotexParameters.format == AnnotexFormat::Auto)
	{
		const string MASK_ENDING = "_mask.png";
		if (0 == annotexParameters.sourcePath.compare(annotexParameters.sourcePath.length() - MASK_ENDING.length(), MASK_ENDING.length(), MASK_ENDING))
		{
			rp.m_dxgi_format = DXGI_FORMAT_BC1_UNORM;
			annotexParameters.pixel_format_bpp = 4;
			annotexParameters.format = AnnotexFormat::Mask;
		}
		else
			annotexParameters.format = AnnotexFormat::Diff;
	}

	return true;
}
