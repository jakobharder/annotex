# annotex

A command line tool to generate DDS BC7/BC1 textures with LOD for Anno.
It is based on [bc7enc](https://github.com/richgel999/bc7enc_rdo).

The reason I made this is because I needed a CPU-based encoder to run
[Modding Tools for Anno](https://github.com/anno-mods/vscode-anno-modding-tools) in GitHub actions.
Otherwise I would have chosen [texconv](https://github.com/microsoft/DirectXTex).

## How to use

To generate a BC7 DDS texture with LOD from 0 to 2 simply call:

```shell
annotex.exe townhall_diff.png -l=3
```

The output will be 3 files including mipmaps:

- townhall_diff_0.dds
- townhall_diff_1.dds
- townhall_diff_2.dds


```
Usage: annotex.exe [options] input.png

-f=<format>  Specify encoding format. Default: auto
   auto: Select below formats based on file ending.
   diff: Encode to BC7 (default).
   mask: Encode to BC1.
         Extension for auto: '_mask.png'.
   rga:  Encode normal map to BC7. Converts blue to alpha (glossiness).
         Extension for auto: '_rga.png'.
         Optional: uses blue channel from '_r_a+b.png' for glow.
   r_a:  Encode metal map to BC7. Converts blue to alpha (ambient occlusion).
         Extension for auto: '_r_a.png' or '_r_a+b.png'
-l=<0-9>  Number of LOD to generate with '_<LOD>.png' ending.
          0 will disable LODs (default)
-v  verbose
```

### Downscale Algorithm

Mipmap and LOD generation are scaled down with a simple box downscale that relies on power of 2 textures (1024, 2048, ...).
A sum of two power of 2 sizes is also OK (e.g. 1536 = 1024 + 512).

Width and height can be different.

Other sizes will work, but ignore odd pixels when downscaling.

## Changes

### 1.2

- Added `_r_a.png`, `_r_a+b.png` to `_metal.dds` conversion
- Fixed non-square maps with `_rga`.

### 1.1

- Added `_rga.png` to `_norm.dds` conversion

## Notes & License

The original code in this repository is unmodified to enable me to rebase to newer bc7enc if I need to. Only `annotex.cpp`, `annotex/` has been added.

New code has been checked with VS 2019 only.

bc7enc: [MIT License](./LICENSE), Copyright(c) 2020-2021 Richard Geldreich, Jr.

annotex: MIT, Jakob Harder
