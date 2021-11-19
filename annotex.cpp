// test.cpp - Command line example/test app
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define ANNOTEX_VERSION "1.0.0"
#define BC7ENC_VERSION "1.08"

#define COMPUTE_SSIM (0)

#if _OPENMP
#include <omp.h>
#endif

#include "rdo_bc_encoder.h"

using namespace utils;

static int print_usage()
{
	fprintf(stderr, "\nUsage: annotex.exe [options] input_filename.png\n\n");
	fprintf(stderr, "-v verbose (print more status output)\n");
	fprintf(stderr, "-o Write output files to the source file's directory, instead of the current directory.\n");
	fprintf(stderr, "-1 Encode to BC1.\n");
	fprintf(stderr, "-f Force writing DX10-style DDS files (otherwise for BC1-5 it uses DX9-style DDS files)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "-uX BC7: Set the BC7 base encoder quality level. X ranges from [0,4] for bc7enc.cpp or [0,6] for bc7e.ispc. Default is 6 (highest quality).\n");
	fprintf(stderr, "\nRDO encoding options (compatible with all formats/encoders):\n");
	fprintf(stderr, "-e BC7: Quantize/weight BC7 output for lower entropy (no slowdown but only 5-10%% gains, can be combined with -z# for more gains)\n");
	fprintf(stderr, "-z# BC1-7: Set RDO lambda factor (quality), lower=higher quality/larger LZ compressed files, try .1-4, combine with -e for BC7 for more gains\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "BC1/BC3 RGB specific options:\n");
	fprintf(stderr, "-b BC1: Don't use 3-color mode transparent texels on blocks containing black or very dark pixels. By default this mode is now enabled.\n");
	fprintf(stderr, "-c BC1: Disable 3-color mode\n");
	fprintf(stderr, "\nBy default, this tool encodes to BC1 *without rounding* 4-color block colors 2,3, which may not match the output of some software decoders.\n");
	fprintf(stderr, "\nFor BC1, the engine/shader must ignore decoded texture alpha because the encoder utilizes transparent texel to get black/dark texels. Use -b to disable.\n");

	return EXIT_FAILURE;
}

int main(int argc, char* argv[])
{
	bool quiet_mode = true;

	for (int i = 0; i < argc; i++)
		if (strcmp(argv[i], "-v") == 0)
			quiet_mode = false;

	if (!quiet_mode || argc < 2)
		printf("annotex %s - Anno DDS texture encoder (based on bc7enc v%s)\n", ANNOTEX_VERSION, BC7ENC_VERSION);

	int max_threads = 1;
#if _OPENMP
	max_threads = std::min(std::max(1, omp_get_max_threads()), 128);
#endif
		
	if (argc < 2)
		return print_usage();
		
	std::string src_filename, dds_output_filename;

	bool no_output_png = true;
	bool out_cur_dir = true;
	bool force_dx10_dds = false;

	uint32_t pixel_format_bpp = 8;

	rdo_bc::rdo_bc_params rp;
	rp.m_rdo_max_threads = max_threads;
	rp.m_status_output = !quiet_mode;

	for (int i = 1; i < argc; i++)
	{
		const char *pArg = argv[i];
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
					pixel_format_bpp = 4;
					printf("Compressing to BC1\n");
					break;
				}
				case 'f':
				{
					force_dx10_dds = true;
					break;
				}
				case 'u':
				{
					rp.m_bc7_uber_level = atoi(pArg + 2);
					if ((rp.m_bc7_uber_level < 0) || (rp.m_bc7_uber_level > 6)) //BC7ENC_MAX_UBER_LEVEL))
					{
						fprintf(stderr, "Invalid argument: %s\n", pArg);
						return EXIT_FAILURE;
					}
					break;

				}
				case 'z':
				{
					rp.m_rdo_lambda = (float)atof(pArg + 2);
					rp.m_rdo_lambda = std::min<float>(std::max<float>(rp.m_rdo_lambda, 0.0f), 500.0f);
					break;
				}
				case 'o':
				{
					out_cur_dir = false;
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
					return EXIT_FAILURE;
				}
			}
		}
		else
		{
			if (!src_filename.size())
			{
				src_filename = pArg;
			}
			else
			{
				fprintf(stderr, "Invalid argument: %s\n", pArg);
				return EXIT_FAILURE;
			}
		}
	}
		
	if (!src_filename.size())
	{
		fprintf(stderr, "No source filename specified!\n");
		return EXIT_FAILURE;
	}

	if (!dds_output_filename.size())
	{
		dds_output_filename = src_filename;
		strip_extension(dds_output_filename);
		if (out_cur_dir)
			strip_path(dds_output_filename);
		dds_output_filename += ".dds";
	}

	image_u8 source_image;
	if (!load_png(src_filename.c_str(), source_image))
		return EXIT_FAILURE;

	clock_t overall_start_t = clock();

	rdo_bc::rdo_bc_encoder encoder;
	if (!encoder.init(source_image, rp))
	{
		fprintf(stderr, "rdo_bc_encoder::init() failed!\n");
		return EXIT_FAILURE;
	}

	if (rp.m_status_output)
	{
		printf("Source image: %s %ux%u %s\n",
			src_filename.c_str(),
			source_image.width(),
			source_image.height(),
			encoder.get_has_alpha() ? "with alpha" : "without alpha");
	}

	if (!encoder.encode())
	{
		fprintf(stderr, "rdo_bc_encoder::encode() failed!\n");
		return EXIT_FAILURE;
	}
			
	if (!save_dds(dds_output_filename.c_str(), encoder.get_orig_width(), encoder.get_orig_height(),
		encoder.get_blocks(), pixel_format_bpp, rp.m_dxgi_format, rp.m_perceptual, force_dx10_dds))
	{
		fprintf(stderr, "Failed writing file \"%s\"\n", dds_output_filename.c_str());
		return EXIT_FAILURE;
	}

	clock_t overall_end_t = clock();
	if (rp.m_status_output)
		printf("Total processing time: %f secs\n", (double)(overall_end_t - overall_start_t) / CLOCKS_PER_SEC);

	if (rp.m_status_output)
		printf("Wrote DDS file %s\n", dds_output_filename.c_str());

	return EXIT_SUCCESS;
}
