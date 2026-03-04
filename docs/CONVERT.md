# GIF to ZMK Animation Converter

The [`gif-convert.py`](gif-convert.py) script converts GIF animations into LVGL-compatible C code for use with the ZMK nice_oled widget system. It handles image preprocessing, dithering, and automatic registration in the build system.

## Overview

This script performs the following operations:

1. **Preprocessing** (optional): Uses ImageMagick to coalesce GIF frames, convert to grayscale, and optionally crop
2. **Dithering**: Applies Atkinson dithering to convert grayscale images to 1-bit black and white
3. **Code Generation**: Creates `.c` and `.h` files containing LVGL image descriptors
4. **Registration**: Automatically registers the animation in the project build files

## Requirements

- Python 3.x
- Pillow (PIL) library: `pip install Pillow`
- ImageMagick (optional, for preprocessing): Install via your system package manager

## Usage

```bash
python3 gif-convert.py <input_gif> [options]
```

### Arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `input_gif` | - | positional | (required) | Path to the input GIF file |
| `--name` | `-n` | string | (from filename) | Base name for output files |
| `--output-dir` | `-o` | string | `boards/shields/nice_oled/assets` | Output directory for generated files |
| `--width` | `-W` | int | 64 | Target width in pixels |
| `--height` | `-H` | int | 64 | Target height in pixels |
| `--rotate` | - | int | 90 | Rotation angle in degrees |
| `--no-rotate` | - | flag | - | Disable rotation |
| `--center` | - | flag | - | Set image gravity to center |
| `--crop` | - | string | - | Crop geometry for ImageMagick |
| `--no-preprocess` | - | flag | - | Skip ImageMagick preprocessing |
| `--threshold` | `-t` | int | 128 | Dithering threshold (0-255) |
| `--animation-ms` | - | int | 5000 | Animation duration in milliseconds |
| `--no-register` | - | flag | - | Skip registering in project files |

### Examples

**Basic usage (name derived from filename):**
```bash
python3 gif-convert.py /path/to/myanimation.gif
```

**Specify custom name and dimensions:**
```bash
python3 gif-convert.py /path/to/animation.gif -n custom_anim -W 48 -H 48
```

**With cropping (48x48 from top-left):**
```bash
python3 gif-convert.py /path/to/animation.gif --crop 48x48
```

**With cropping (48x48 with offset):**
```bash
python3 gif-convert.py /path/to/animation.gif --crop 48x48+16+16
```

**Skip preprocessing and registration:**
```bash
python3 gif-convert.py /path/to/animation.gif --no-preprocess --no-register
```

**Custom animation duration:**
```bash
python3 gif-convert.py /path/to/animation.gif --animation-ms 3000
```

## Output Files

The script generates two files in the output directory:

### `<name>.c`
Contains:
- Frame data as byte arrays (`<name>_00_map[]`, `<name>_01_map[]`, etc.)
- LVGL image descriptors for each frame
- Color palette configuration (supports inverted mode via `CONFIG_NICE_OLED_WIDGET_INVERTED`)

### `<name>.h`
Contains:
- `LV_IMG_DECLARE()` declarations for each frame
- Array of image descriptor pointers (`<name>_images[]`)
- Frame count macro (`<NAME>_IMAGES_NUM_IMAGES`)

## Automatic Registration

By default, the script registers the animation in three project files. It uses existing animation entries (pokemon, spaceman, cat) as insertion anchors.

### 1. [`animation.c`](boards/shields/nice_oled/widgets/animation.c)

Adds:
- `LV_IMG_DECLARE()` declarations for all frames
- Image array definition (`<name>_imgs[]`)
- `lv_animimg_set_src()` call in `draw_animation()` function

### 2. [`CMakeLists.txt`](boards/shields/nice_oled/CMakeLists.txt)

Adds:
```cmake
target_sources_ifdef(CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_<NAME> app PRIVATE assets/<name>.c)
```

### 3. [`Kconfig.defconfig`](boards/shields/nice_oled/Kconfig.defconfig)

Adds:
```kconfig
config NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_<NAME>
    bool "Enable <Name> animation on peripheral"
    default n
```

And adds the animation duration default:
```kconfig
default <animation-ms> if NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_<NAME>
```

## Functions Reference

### Image Processing Functions

#### [`atkinson_dither(image, threshold=128)`](gif-convert.py:20)
Applies Atkinson dithering algorithm to a grayscale PIL Image.

**Parameters:**
- `image`: PIL Image in grayscale ('L' mode)
- `threshold`: Threshold value for binarization (0-255)

**Returns:** Dithered PIL Image with only black (0) and white (255) values

**Algorithm:** Distributes quantization error to neighboring pixels in the pattern:
```
      X   1/8  1/8
1/8  1/8  1/8
     1/8
```

#### [`process_frame(frame, target_width, target_height, threshold=128)`](gif-convert.py:50)
Processes a single animation frame with resizing, grayscale conversion, and dithering.

**Parameters:**
- `frame`: PIL Image (RGB)
- `target_width`: Target canvas width
- `target_height`: Target canvas height
- `threshold`: Dithering threshold

**Returns:** 1-bit PIL Image centered on target canvas

### Code Generation Functions

#### [`create_animation_files(input_gif, output_c, output_h, name, target_width, target_height, threshold)`](gif-convert.py:78)
Generates C and H files from a GIF animation.

**Parameters:**
- `input_gif`: Path to source GIF file
- `output_c`: Path for output .c file
- `output_h`: Path for output .h file
- `name`: Base name for generated identifiers
- `target_width`: Target canvas width
- `target_height`: Target canvas height
- `threshold`: Dithering threshold

**Returns:** Number of frames processed

### Preprocessing Functions

#### [`preprocess_gif_with_imagemagick(input_gif, output_gif, crop=None, rotate=None, center=False)`](gif-convert.py:195)
Preprocesses a GIF using ImageMagick.

**Parameters:**
- `input_gif`: Path to input GIF
- `output_gif`: Path for preprocessed output
- `crop`: Optional crop geometry (e.g., "48x48" or "48x48+16+16")
- `rotate`: Optional rotation angle in degrees (default: 90, use `--no-rotate` to disable)
- `center`: Whether to center the crop using -gravity center (default: False)

**ImageMagick Command Order:**
```bash
magick input.gif -coalesce [-rotate DEGREES] -colorspace Gray [-gravity center] [-crop WxH+X+Y +repage] output.gif
```

**Order of operations:**
1. `-coalesce` - Flatten frames
2. `-rotate` - Rotate image (if specified)
3. `-colorspace Gray` - Convert to grayscale
4. `-gravity center` - Set gravity for centered cropping (if `--center`)
5. `-crop` + `+repage` - Crop and reset page geometry

**Returns:** `True` on success, `False` on failure

### Registration Functions

#### [`register_in_animation_c(name, frame_count, animation_path)`](gif-convert.py:235)
Registers the animation in the animation.c widget file.

**Parameters:**
- `name`: Animation base name
- `frame_count`: Number of frames
- `animation_path`: Path to animation.c

**Modifies:** Adds declarations and array definitions to animation.c

#### [`register_in_cmake(name, cmake_path)`](gif-convert.py:312)
Registers the animation in CMakeLists.txt.

**Parameters:**
- `name`: Animation base name
- `cmake_path`: Path to CMakeLists.txt

**Modifies:** Adds `target_sources_ifdef` line

#### [`register_in_kconfig(name, default_ms, kconfig_path)`](gif-convert.py:349)
Registers the animation in Kconfig.defconfig.

**Parameters:**
- `name`: Animation base name
- `default_ms`: Default animation duration in milliseconds
- `kconfig_path`: Path to Kconfig.defconfig

**Modifies:** Adds config option and duration default

## Configuration Constants

Defined at the top of [`gif-convert.py`](gif-convert.py:14-18):

```python
DEFAULT_ASSETS_DIR = "boards/shields/nice_oled/assets"
DEFAULT_CMAKE_PATH = "boards/shields/nice_oled/CMakeLists.txt"
DEFAULT_KCONFIG_PATH = "boards/shields/nice_oled/Kconfig.defconfig"
DEFAULT_ANIMATION_PATH = "boards/shields/nice_oled/widgets/animation.c"
```

## Generated Image Format

The generated C code produces LVGL 1-bit indexed images with the following characteristics:

- **Color Format:** `LV_IMG_CF_INDEXED_1BIT`
- **Palette:** 2 colors (black and white, with inversion support)
- **Data Structure:** `lv_img_dsc_t` descriptors pointing to byte arrays

### Palette Definition
```c
#if CONFIG_NICE_OLED_WIDGET_INVERTED
  0xff, 0xff, 0xff, 0xff, /*Color of index 0*/
  0x00, 0x00, 0x00, 0xff, /*Color of index 1*/
#else
  0x00, 0x00, 0x00, 0xff, /*Color of index 0*/
  0xff, 0xff, 0xff, 0xff, /*Color of index 1*/
#endif
```

## Troubleshooting

### ImageMagick Not Found
If you see "ImageMagick 'magick' command not found":
- Install ImageMagick: `sudo apt install imagemagick` (Debian/Ubuntu)
- Or use `--no-preprocess` to skip preprocessing

### Registration Fails
If registration cannot find insertion points:
- Ensure the default animation files exist at the expected paths
- Check that the animation entries are present (used as insertion anchors)
- Use `--no-register` and manually add the entries

### Large Frame Counts
For GIFs with many frames, the generated C file can become very large. Consider:
- Reducing frame count in the source GIF
- Using a smaller canvas size
- Skipping registration if not needed

## See Also

- [LVGL Image Documentation](https://docs.lvgl.io/latest/en/html/overview/image.html)
- [ZMK Display Documentation](https://zmk.dev/docs/features/displays)
- [ImageMagick Command-line Options](https://imagemagick.org/script/command-line-options.php)
