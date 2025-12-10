# static2jxl

High-performance C tool for batch static image → JXL conversion with complete metadata preservation.

[English](#features) | [中文](#功能特性)

---

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

---

## 功能特性

### 智能格式检测
- **魔数检测** - 通过文件内容识别类型，而非扩展名
- **TIFF 压缩分析** - 检测 JPEG 压缩的 TIFF 并跳过（已有损）
- **RAW 格式保留** - 自动跳过 RAW 文件（DNG、CR2、CR3、NEF、ARW 等）

### 转换模式
| 输入格式 | 输出模式 | 说明 |
|----------|----------|------|
| JPEG | `--lossless_jpeg=1` | **可逆转码** - 保留 DCT 系数，可完美还原为相同的 JPEG |
| PNG/BMP/TGA/PPM | `-d 0` | 数学无损（仅 >2MB 文件） |
| TIFF (未压缩/LZW) | `-d 0` | 数学无损（仅 >2MB 文件） |
| RAW 格式 | 跳过 | 保留 RAW 灵活性 |
| JPEG 压缩的 TIFF | 跳过 | 已有损，避免代际损失 |

### 完整元数据保留（5 层）
1. **内部元数据** - 通过 exiftool 保留 EXIF、IPTC、XMP、ICC Profile
2. **系统时间戳** - 保留 mtime、atime
3. **macOS 扩展属性** - xattr（WhereFroms、quarantine 等）
4. **macOS 创建时间** - 保留 birthtime
5. **验证** - 可选的元数据保留检查

### 安全特性
- **智能回退** - 如果 JXL 输出大于原文件则自动跳过
- **健康检查** - 删除原文件前通过 djxl 验证 JXL 输出
- **危险目录检测** - 防止在系统目录中意外转换
- **大小阈值** - 无损源文件必须 >2MB 才转换

### 性能
- **多线程** - 可配置的并行处理（`-j N`）
- **进度可视化** - 实时进度条和预计剩余时间
- **中断处理** - Ctrl+C 优雅关闭

## 使用方法

```bash
# 编译
make

# 基本用法 - 转换为单独的 .jxl 文件
./static2jxl /path/to/images

# 原地替换（成功转换后删除原文件）
./static2jxl --in-place /path/to/images

# 8 个并行线程
./static2jxl -j 8 /path/to/images

# 详细输出含元数据验证
./static2jxl -v /path/to/images

# 预览模式（不实际转换）
./static2jxl --dry-run /path/to/images

# 跳过健康检查（更快但不太安全）
./static2jxl --skip-health-check /path/to/images
```

## 选项

| 选项 | 说明 |
|------|------|
| `--in-place`, `-i` | 成功转换后替换原文件 |
| `--skip-health-check` | 跳过 JXL 验证（更快） |
| `--no-recursive` | 不处理子目录 |
| `--force-lossless` | 对所有格式强制无损模式 |
| `--verbose`, `-v` | 显示详细输出含元数据验证 |
| `--dry-run` | 预览不转换 |
| `-j <N>` | 并行线程数（默认：4） |
| `-d <distance>` | 覆盖 JXL distance 参数 |
| `-e <effort>` | JXL effort 1-9（默认：7） |

## 依赖

```bash
# macOS
brew install jpeg-xl exiftool

# Linux (Debian/Ubuntu)
apt install libjxl-tools libimage-exiftool-perl
```

## 为什么选择这个工具？

- **JPEG 可逆性** - 与其他转换器不同，使用 `--lossless_jpeg=1` 保留 DCT 系数。JXL 可以转换回完全相同的 JPEG。
- **完整元数据** - 大多数转换器会丢失 xattr、创建时间或 ICC 配置文件。此工具保留一切。
- **智能过滤** - 自动跳过不应转换的文件（RAW、小文件、已有损的 TIFF）。
- **默认安全** - 健康检查、大小比较和危险目录检测防止数据丢失。

---

MIT License
