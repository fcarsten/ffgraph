# PNG icons and masks

The 85 PNG files replace every bundled Xpm (37) and XBM (48) image, including
the hidden `.xpm` asset. Color images live in `png/`; monochrome images and masks
live in `png/xbm/`, keeping identically named color/monochrome images distinct.

Pixels and dimensions are unchanged. Monochrome set bits are opaque black;
unset bits and Xpm `None` colors are transparent. Mask PNGs use their alpha
channel: opaque pixels keep the source image, transparent pixels clip it.
Applying a mask never changes the shared original icon.

New graph references can use `redball.png`, `procedureN.png`/`procedureS.png`,
or `xbm/default.png`. `legacy-icons.tcl` maps historical names such as
`redball.xpm` and `ballmask.xbm` to PNG. Extensionless historical N/S pairs
also work. Graph metadata retains the user's original strings when saved.

`scripts/convert-legacy-icons.py` performs the deterministic conversion using
only Python's standard library. Supply the original `icons/` tree from Git
history as `--source`, this directory as `--output`, and
`tests/image_assets_manifest.tsv` as `--manifest`. Original-file SHA256 hashes,
dimensions and decoded RGBA CRC32 values are recorded in that manifest.
The GUI regression decodes the PNGs with Tk and checks all pixels against those
frozen source checksums. The migration also independently checked the source
rasters using libXpm and Xlib.

Normal builds require no Xpm library. `FFGRAPH_XPM=ON` optionally enables the
existing Linux handler for external Xpm files. That handler is not used for
any bundled image; Windows does not build it. External XBM, PNG, GIF and PPM
files remain usable without Xpm.
