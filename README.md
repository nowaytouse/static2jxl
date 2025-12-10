# static2jxl

High-performance C tool for batch static image → JXL conversion with complete metadata preservation.

## Features

### Intelligent Format Detection
- **Magic bytes detection** - Identifies file types by content, not extension
- **TIFF compression analysis** - Detects JPEG-compressed TIFFs and skips them (already lossy)
- **RAW format preservation** - Automatically skips RAW files (DNG, CR2, CR3, NEF, ARW, etc.)

### Conversion Modes
| Input Format | Output Mode | Description |
|--------------|-------------|-------------|
| JPEG | `--lossless_jpeg=1` | **Reversible transcode** - preserves DCT coefficients, can be converted back to identical JPEG |
| PNG/BMP/TGA/PPM | `-d 0` | Mathematical lossless (only files >2MB) |
| TIFF (uncompressed/LZW) | `-d 0` | Mathematical lossless (only files >2MB) |
| RAW formats | SKIP | Preserves RAW flexibility |
| JPEG-compressed TIFF | SKIP | Already lossy, avoid generation loss |

### Complete Metadata Preservation (5 Layers)
1. **Internal metadata** - EXIF, IPTC, XMP, ICC Profile via exiftool
2. **System timestamps** - mtime, atime preserved
3. **macOS extended attributes** - xattr (WhereFroms, quarantine, etc.)
4. **macOS creation time** - birthtime preserved
5. **Verification** - Optional metadata preservation check

### Safety Features
- **Smart rollback** - Automatically skips if JXL output is larger than original
- **Health check** - Validates JXL output via djxl before deleting original
- **Dangerous directory detection** - Prevents accidental conversion in system directories
- **Size threshold** - Lossless sources must be >2MB to convert

### Performance
- **Multi-threaded** - Configurable parallel processing (`-j N`)
- **Progress visualization** - Real-time progress bar with ETA
- **Interrupt handling** - Graceful shutdown on Ctrl+C

## Usage

```bash
# Build
make

# Basic usage - convert to separate .jxl files
./static2jxl /path/to/images

# In-place replacement (delete originals after successful conversion)
./static2jxl --in-place /path/to/images

# 8 parallel threads
./static2jxl -j 8 /path/to/images

# Verbose output with metadata verification
./static2jxl -v /path/to/images

# Dry run (preview without converting)
./static2jxl --dry-run /path/to/images

# Skip health check (faster but less safe)
./static2jxl --skip-health-check /path/to/images
```

## Options

| Option | Description |
|--------|-------------|
| `--in-place`, `-i` | Replace original files after successful conversion |
| `--skip-health-check` | Skip JXL validation (faster) |
| `--no-recursive` | Don't process subdirectories |
| `--force-lossless` | Force lossless mode for all formats |
| `--verbose`, `-v` | Show detailed output including metadata verification |
| `--dry-run` | Preview without converting |
| `-j <N>` | Parallel threads (default: 4) |
| `-d <distance>` | Override JXL distance parameter |
| `-e <effort>` | JXL effort 1-9 (default: 7) |

## Dependencies

```bash
# macOS
brew install jpeg-xl exiftool

# Linux (Debian/Ubuntu)
apt install libjxl-tools libimage-exiftool-perl
```

## Why This Tool?

- **JPEG reversibility** - Unlike other converters, uses `--lossless_jpeg=1` which preserves DCT coefficients. The JXL can be converted back to a bit-identical JPEG.
- **Complete metadata** - Most converters lose xattr, creation time, or ICC profiles. This tool preserves everything.
- **Smart filtering** - Automatically skips files that shouldn't be converted (RAW, small files, already-lossy TIFFs).
- **Safe by default** - Health checks, size comparisons, and dangerous directory detection prevent data loss.

## License

MIT
