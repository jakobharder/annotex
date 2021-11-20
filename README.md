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

To get an overview of all parameters call without any arguments:

```
annotex.exe
```

## License

Original license: [MIT License](./LICENSE), Copyright(c) 2020-2021 Richard Geldreich, Jr.
Added code in annotex.cpp is also MIT.
