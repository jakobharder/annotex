
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

static void print_usage()
{
	printf("\nUsage: annotex.exe [options] input_filename.png\n\n");
	printf("-v verbose (print more status output)\n");
	printf("-o Write output files to the source file's directory, instead of the current directory.\n");
	printf("-1 Encode to BC1.\n");
	printf("-f Force writing DX10-style DDS files (otherwise for BC1-5 it uses DX9-style DDS files)\n");
	printf("\n");
	printf("-uX BC7: Set the BC7 base encoder quality level. X ranges from [0,4] for bc7enc.cpp or [0,6] for bc7e.ispc. Default is 6 (highest quality).\n");
	printf("\nRDO encoding options (compatible with all formats/encoders):\n");
	printf("-e BC7: Quantize/weight BC7 output for lower entropy (no slowdown but only 5-10%% gains, can be combined with -z# for more gains)\n");
	printf("-z# BC1-7: Set RDO lambda factor (quality), lower=higher quality/larger LZ compressed files, try .1-4, combine with -e for BC7 for more gains\n");
	printf("\n");
	printf("BC1/BC3 RGB specific options:\n");
	printf("-b BC1: Don't use 3-color mode transparent texels on blocks containing black or very dark pixels. By default this mode is now enabled.\n");
	printf("-c BC1: Disable 3-color mode\n");
	printf("\nBy default, this tool encodes to BC1 *without rounding* 4-color block colors 2,3, which may not match the output of some software decoders.\n");
	printf("\nFor BC1, the engine/shader must ignore decoded texture alpha because the encoder utilizes transparent texel to get black/dark texels. Use -b to disable.\n");
}

bool parseArguments(int argc, char* argv[], rdo_bc::rdo_bc_params& rp, AnnotexParameters& annotexParameters)
{
#if _OPENMP
	const int MAX_THREADS = min(max(1, omp_get_max_threads()), 128);
#else
	const int MAX_THREADS = 1;
#endif
	rp.m_rdo_max_threads = MAX_THREADS;
	annotexParameters.verbose = false;
	annotexParameters.outputToCurrentDir = true;
	annotexParameters.force_dx10_dds = false;
	annotexParameters.pixel_format_bpp = 8;

	for (int i = 0; i < argc; i++)
		if (strcmp(argv[i], "-v") == 0)
			annotexParameters.verbose = true;

	if (annotexParameters.verbose || argc < 2)
		printf("annotex %s - Anno DDS texture encoder (based on bc7enc v%s)\n", ANNOTEX_VERSION, BC7ENC_VERSION);
	if (argc < 2)
	{
		print_usage();
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
			case 'e':
			{
				rp.m_bc7enc_reduce_entropy = true;
				break;
			}
			case '1':
			{
				rp.m_dxgi_format = DXGI_FORMAT_BC1_UNORM;
				annotexParameters.pixel_format_bpp = 4;
				printf("Compressing to BC1\n");
				break;
			}
			case 'f':
			{
				annotexParameters.force_dx10_dds = true;
				break;
			}
			case 'u':
			{
				rp.m_bc7_uber_level = atoi(pArg + 2);
				if ((rp.m_bc7_uber_level < 0) || (rp.m_bc7_uber_level > 6)) //BC7ENC_MAX_UBER_LEVEL))
				{
					fprintf(stderr, "Invalid argument: %s\n", pArg);
					return false;
				}
				break;

			}
			case 'z':
			{
				rp.m_rdo_lambda = (float)atof(pArg + 2);
				rp.m_rdo_lambda = min<float>(max<float>(rp.m_rdo_lambda, 0.0f), 500.0f);
				break;
			}
			case 'o':
			{
				annotexParameters.outputToCurrentDir = false;
				break;
			}
			case 'b':
			{
				rp.m_use_bc1_3color_mode_for_black = false;
				break;
			}
			case 'c':
			{
				rp.m_use_bc1_3color_mode = false;
				break;
			}
			default:
			{
				fprintf(stderr, "Invalid argument: %s\n", pArg);
				return false;
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

	return true;
}
