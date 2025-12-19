# static2jxl

🎨 High-Quality JPEG/Image to JXL Lossless Converter

[English](#features) | [中文](#功能特性)

---

## 🎯 Positioning: High-Quality Image Optimization Tool

**Target Users**: Photographers, designers, archivists who need lossless quality preservation

**Core Philosophy**: Quality First, Size Second

| Priority | Description |
|----------|-------------|
| 🥇 Quality | Lossless JPEG transcoding preserves DCT coefficients |
| 🥈 Metadata | Complete EXIF/XMP/ICC preservation |
| 🥉 Size | Only convert if output is smaller |

---

## Features

### Intelligent Format Detection
- **Magic bytes detection** - Identifies file types by content, not extension
- **TIFF compression analysis** - Detects JPEG-compressed TIFFs and skips them
- **RAW format preservation** - Automatically skips RAW files (DNG, CR2, NEF, etc.)

### Conversion Strategy
| Input Format | Output Mode | Description |
|--------------|-------------|-------------|
| JPEG | `--lossless_jpeg=1` | **Reversible transcode** - can convert back to identical JPEG |
| PNG/BMP/TGA | `-d 0` | Mathematical lossless (files ≥1.25MB only) |
| TIFF (uncompressed) | `-d 0` | Mathematical lossless (files ≥1.25MB only) |
| RAW formats | SKIP | Preserves RAW flexibility |

### Complete Metadata Preservation (5 Layers)
1. **Internal metadata** - EXIF, IPTC, XMP, ICC Profile via exiftool
2. **System timestamps** - mtime, atime preserved
3. **macOS extended attributes** - xattr (WhereFroms, quarantine, etc.)
4. **macOS creation time** - birthtime preserved
5. **Verification** - Optional metadata preservation check

### Safety Features
- **Smart rollback** - Skips if JXL output is larger than original
- **Health check** - Validates JXL output via djxl
- **Size threshold** - Lossless sources must be ≥1.25MB

## Usage

```bash
make
./static2jxl /path/to/images
./static2jxl --in-place /path/to/images  # Replace originals
./static2jxl -j 8 /path/to/images        # 8 threads
```

## Options

| Option | Description |
|--------|-------------|
| `--in-place`, `-i` | Replace original files |
| `--verbose`, `-v` | Show detailed output |
| `-j <N>` | Parallel threads (default: 4) |
| `-e <effort>` | JXL effort 1-9 (default: 7) |

## Dependencies

```bash
brew install jpeg-xl exiftool  # macOS
```

## Test Coverage / 测试覆盖

**Total: 47 precision tests ✅**

| Category | Tests | Description |
|----------|-------|-------------|
| Size Reduction | 6 | Formula: `(1 - output/input) * 100%` |
| Size Threshold | 5 | 2MB threshold for lossless sources |
| JXL Distance | 3 | Distance parameter validation |
| Magic Bytes | 9 | JPEG/PNG/BMP/TIFF/JXL/PPM detection |
| TIFF Compression | 5 | Compression type suitability |
| Lossless Source | 5 | PNG/BMP/PPM classification |
| **Consistency** | **14** | **5-level verification system** |

### 🔄 Consistency Verification System / 一致性验证系统

Following `shared_utils` pattern from modern_format_boost:

遵循 modern_format_boost 的 `shared_utils` 模式：

| Level | Tests | Description |
|-------|-------|-------------|
| L1: Deterministic | 4 | Same input → same output (100 iterations) |
| L2: Cross-Function | 2 | Related functions agree |
| L3: Boundary | 3 | Edge cases handled uniformly |
| L4: Pipeline | 2 | Input → process → output chain |
| L5: Data Integrity | 3 | Mathematical relationships hold |

### Run Tests / 运行测试

```bash
cd tests
make test
# Or manually:
cc -o test_precision test_precision.c -lm
./test_precision
```

### Quality Principles / 质量原则

- ✅ **Precision Validated** - All calculations verified by "裁判" tests
- ✅ **Content-Based Detection** - Magic bytes, not extensions
- ✅ **Fail Loudly** - No silent fallback
- ✅ **Consistency Guaranteed** - Same input → same output

---

## 功能特性

### 🎯 定位：高质量图像优化工具

**目标用户**：摄影师、设计师、需要无损质量保留的存档用户

**核心理念**：质量优先，大小其次

| 优先级 | 说明 |
|--------|------|
| 🥇 质量 | JPEG 无损转码保留 DCT 系数 |
| 🥈 元数据 | 完整 EXIF/XMP/ICC 保留 |
| 🥉 大小 | 仅在输出更小时转换 |

### 转换策略
| 输入格式 | 输出模式 | 说明 |
|----------|----------|------|
| JPEG | `--lossless_jpeg=1` | **可逆转码** - 可完美还原为相同 JPEG |
| PNG/BMP/TGA | `-d 0` | 数学无损（仅 ≥1.25MB 文件） |
| RAW 格式 | 跳过 | 保留 RAW 灵活性 |

### 完整元数据保留（5 层）
1. 内部元数据 - EXIF、IPTC、XMP、ICC Profile
2. 系统时间戳 - mtime、atime
3. macOS 扩展属性 - xattr
4. macOS 创建时间 - birthtime
5. 验证 - 可选的元数据保留检查

## 使用方法

```bash
make
./static2jxl /path/to/images
./static2jxl --in-place /path/to/images  # 替换原文件
```

---

MIT License
