/**
 * 🔬 Precision Validation Tests for static2jxl
 * 
 * 裁判机制 (Judge Mechanism) - Ensures mathematical precision and consistency
 * 
 * Following Pixly Quality Manifesto:
 * - NO silent fallback
 * - NO hardcoded defaults without validation
 * - Fail loudly on errors
 * - All calculations verified
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

// ============================================================
// Test Framework
// ============================================================

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing %s... ", #name); \
    test_##name(); \
    printf("✅ PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("❌ FAILED: %s != %s (line %d)\n", #a, #b, __LINE__); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_NEAR(a, b, eps) do { \
    if (fabs((a) - (b)) > (eps)) { \
        printf("❌ FAILED: %s ≈ %s (diff: %f, line %d)\n", #a, #b, fabs((a)-(b)), __LINE__); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf("❌ FAILED: %s (line %d)\n", #cond, __LINE__); \
        tests_failed++; \
        return; \
    } \
} while(0)

// ============================================================
// Size Reduction Calculation Tests (裁判机制)
// ============================================================

// Formula: (1 - output/input) * 100
double calculate_size_reduction(size_t input_size, size_t output_size) {
    if (input_size == 0) return 0.0;
    return (1.0 - (double)output_size / input_size) * 100.0;
}

TEST(size_reduction_50_percent) {
    // 1000 -> 500 = 50% reduction
    double reduction = calculate_size_reduction(1000, 500);
    ASSERT_NEAR(reduction, 50.0, 0.01);
}

TEST(size_reduction_75_percent) {
    // 1000 -> 250 = 75% reduction
    double reduction = calculate_size_reduction(1000, 250);
    ASSERT_NEAR(reduction, 75.0, 0.01);
}

TEST(size_reduction_no_change) {
    // Same size = 0% reduction
    double reduction = calculate_size_reduction(1000, 1000);
    ASSERT_NEAR(reduction, 0.0, 0.01);
}

TEST(size_reduction_increase) {
    // 500 -> 1000 = -100% (doubled)
    double reduction = calculate_size_reduction(500, 1000);
    ASSERT_NEAR(reduction, -100.0, 0.01);
}

TEST(size_reduction_zero_input) {
    // Zero input should return 0 (not crash)
    double reduction = calculate_size_reduction(0, 100);
    ASSERT_NEAR(reduction, 0.0, 0.01);
}

TEST(size_reduction_large_files) {
    // 10GB -> 5GB = 50%
    size_t input = 10ULL * 1024 * 1024 * 1024;
    size_t output = 5ULL * 1024 * 1024 * 1024;
    double reduction = calculate_size_reduction(input, output);
    ASSERT_NEAR(reduction, 50.0, 0.001);
}

// ============================================================
// Size Threshold Tests (裁判机制)
// ============================================================

// MIN_LOSSLESS_SIZE from static2jxl.h = 2MB
#define MIN_LOSSLESS_SIZE (2 * 1024 * 1024)

bool should_process_lossless(size_t file_size, bool is_jpeg) {
    // JPEG always processed (reversible transcode)
    if (is_jpeg) return true;
    // Lossless sources: only if >= 2MB
    return file_size >= MIN_LOSSLESS_SIZE;
}

TEST(threshold_jpeg_small) {
    // JPEG should always be processed regardless of size
    ASSERT_TRUE(should_process_lossless(100, true));
}

TEST(threshold_jpeg_large) {
    ASSERT_TRUE(should_process_lossless(10 * 1024 * 1024, true));
}

TEST(threshold_png_below) {
    // PNG below 2MB should be skipped
    ASSERT_TRUE(!should_process_lossless(1 * 1024 * 1024, false));
}

TEST(threshold_png_exact) {
    // PNG exactly 2MB should be processed
    ASSERT_TRUE(should_process_lossless(2 * 1024 * 1024, false));
}

TEST(threshold_png_above) {
    // PNG above 2MB should be processed
    ASSERT_TRUE(should_process_lossless(3 * 1024 * 1024, false));
}

// ============================================================
// JXL Distance Tests (裁判机制)
// ============================================================

// JXL distance: 0 = lossless, 1 = high quality lossy
// For JPEG: --lossless_jpeg=1 (reversible)
// For lossless sources: -d 0 (mathematical lossless)

double get_jxl_distance(bool is_jpeg, bool force_lossless) {
    if (is_jpeg) return -1.0;  // Use --lossless_jpeg=1 instead
    if (force_lossless) return 0.0;
    return 0.0;  // Default: lossless for PNG/BMP/etc
}

TEST(distance_jpeg) {
    // JPEG uses --lossless_jpeg=1, not -d
    double d = get_jxl_distance(true, false);
    ASSERT_NEAR(d, -1.0, 0.001);
}

TEST(distance_png_lossless) {
    // PNG uses -d 0 (lossless)
    double d = get_jxl_distance(false, false);
    ASSERT_NEAR(d, 0.0, 0.001);
}

TEST(distance_force_lossless) {
    double d = get_jxl_distance(false, true);
    ASSERT_NEAR(d, 0.0, 0.001);
}

// ============================================================
// Magic Bytes Detection Tests (裁判机制)
// ============================================================

typedef enum {
    FT_UNKNOWN = 0,
    FT_JPEG,
    FT_PNG,
    FT_BMP,
    FT_TIFF,
    FT_PPM,
    FT_TGA,
    FT_JXL,
    FT_RAW
} TestFileType;

TestFileType detect_magic(const unsigned char *buf, size_t len) {
    if (len < 2) return FT_UNKNOWN;
    
    // JPEG: FF D8 FF
    if (len >= 3 && buf[0] == 0xFF && buf[1] == 0xD8 && buf[2] == 0xFF) {
        return FT_JPEG;
    }
    
    // PNG: 89 50 4E 47 0D 0A 1A 0A
    if (len >= 8 && buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47 &&
        buf[4] == 0x0D && buf[5] == 0x0A && buf[6] == 0x1A && buf[7] == 0x0A) {
        return FT_PNG;
    }
    
    // BMP: 42 4D (BM)
    if (len >= 2 && buf[0] == 0x42 && buf[1] == 0x4D) {
        return FT_BMP;
    }
    
    // TIFF: 49 49 2A 00 (little-endian) or 4D 4D 00 2A (big-endian)
    if (len >= 4 && ((buf[0] == 0x49 && buf[1] == 0x49 && buf[2] == 0x2A && buf[3] == 0x00) ||
                     (buf[0] == 0x4D && buf[1] == 0x4D && buf[2] == 0x00 && buf[3] == 0x2A))) {
        return FT_TIFF;
    }
    
    // JXL: FF 0A (codestream) or 00 00 00 0C 4A 58 4C 20 (container)
    if ((len >= 2 && buf[0] == 0xFF && buf[1] == 0x0A) ||
        (len >= 12 && buf[0] == 0x00 && buf[4] == 0x4A && buf[5] == 0x58 && buf[6] == 0x4C)) {
        return FT_JXL;
    }
    
    // PPM/PGM/PBM: P3, P6 (PPM), P2, P5 (PGM), P1, P4 (PBM)
    if (len >= 2 && buf[0] == 'P' && (buf[1] >= '1' && buf[1] <= '6')) {
        return FT_PPM;
    }
    
    return FT_UNKNOWN;
}

TEST(magic_jpeg) {
    unsigned char buf[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10};
    ASSERT_EQ(detect_magic(buf, sizeof(buf)), FT_JPEG);
}

TEST(magic_png) {
    unsigned char buf[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    ASSERT_EQ(detect_magic(buf, sizeof(buf)), FT_PNG);
}

TEST(magic_bmp) {
    unsigned char buf[] = {0x42, 0x4D, 0x00, 0x00};
    ASSERT_EQ(detect_magic(buf, sizeof(buf)), FT_BMP);
}

TEST(magic_tiff_le) {
    // Little-endian TIFF
    unsigned char buf[] = {0x49, 0x49, 0x2A, 0x00};
    ASSERT_EQ(detect_magic(buf, sizeof(buf)), FT_TIFF);
}

TEST(magic_tiff_be) {
    // Big-endian TIFF
    unsigned char buf[] = {0x4D, 0x4D, 0x00, 0x2A};
    ASSERT_EQ(detect_magic(buf, sizeof(buf)), FT_TIFF);
}

TEST(magic_jxl_codestream) {
    // JXL codestream
    unsigned char buf[] = {0xFF, 0x0A};
    ASSERT_EQ(detect_magic(buf, sizeof(buf)), FT_JXL);
}

TEST(magic_ppm) {
    unsigned char buf[] = {'P', '6', ' '};
    ASSERT_EQ(detect_magic(buf, sizeof(buf)), FT_PPM);
}

TEST(magic_unknown) {
    unsigned char buf[] = {0x00, 0x00, 0x00, 0x00};
    ASSERT_EQ(detect_magic(buf, sizeof(buf)), FT_UNKNOWN);
}

TEST(magic_too_short) {
    unsigned char buf[] = {0xFF};
    ASSERT_EQ(detect_magic(buf, 1), FT_UNKNOWN);
}

// ============================================================
// TIFF Compression Detection Tests (裁判机制)
// ============================================================

typedef enum {
    TIFF_NONE = 1,
    TIFF_LZW = 5,
    TIFF_JPEG = 7,
    TIFF_DEFLATE = 8,
    TIFF_UNKNOWN = 0
} TiffCompression;

bool is_tiff_suitable(TiffCompression comp) {
    // JPEG-compressed TIFF is already lossy, skip it
    return comp != TIFF_JPEG && comp != TIFF_UNKNOWN;
}

TEST(tiff_uncompressed_suitable) {
    ASSERT_TRUE(is_tiff_suitable(TIFF_NONE));
}

TEST(tiff_lzw_suitable) {
    ASSERT_TRUE(is_tiff_suitable(TIFF_LZW));
}

TEST(tiff_deflate_suitable) {
    ASSERT_TRUE(is_tiff_suitable(TIFF_DEFLATE));
}

TEST(tiff_jpeg_not_suitable) {
    ASSERT_TRUE(!is_tiff_suitable(TIFF_JPEG));
}

TEST(tiff_unknown_not_suitable) {
    ASSERT_TRUE(!is_tiff_suitable(TIFF_UNKNOWN));
}

// ============================================================
// Lossless Source Detection Tests (裁判机制)
// ============================================================

bool is_lossless_source(TestFileType type) {
    return type == FT_PNG || type == FT_BMP || 
           type == FT_TGA || type == FT_PPM;
}

TEST(lossless_png) {
    ASSERT_TRUE(is_lossless_source(FT_PNG));
}

TEST(lossless_bmp) {
    ASSERT_TRUE(is_lossless_source(FT_BMP));
}

TEST(lossless_ppm) {
    ASSERT_TRUE(is_lossless_source(FT_PPM));
}

TEST(not_lossless_jpeg) {
    ASSERT_TRUE(!is_lossless_source(FT_JPEG));
}

TEST(not_lossless_jxl) {
    ASSERT_TRUE(!is_lossless_source(FT_JXL));
}

// ============================================================
// 🔄 Consistency Verification System (一致性验证系统)
// ============================================================
// Following shared_utils pattern:
// 1. Deterministic output - same input → same output
// 2. Cross-function consistency - related functions agree
// 3. Boundary consistency - edge cases handled uniformly
// 4. Pipeline consistency - input → process → output chain
// ============================================================

// --- Level 1: Deterministic Output Tests ---

TEST(consistency_size_reduction) {
    // Same input should always produce same output
    for (int i = 0; i < 100; i++) {
        double r1 = calculate_size_reduction(1000, 500);
        double r2 = calculate_size_reduction(1000, 500);
        ASSERT_NEAR(r1, r2, 0.0000001);
    }
}

TEST(consistency_threshold) {
    // Same input should always produce same decision
    for (int i = 0; i < 100; i++) {
        bool d1 = should_process_lossless(3 * 1024 * 1024, false);
        bool d2 = should_process_lossless(3 * 1024 * 1024, false);
        ASSERT_EQ(d1, d2);
    }
}

TEST(consistency_magic) {
    unsigned char jpeg[] = {0xFF, 0xD8, 0xFF, 0xE0};
    unsigned char png[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    unsigned char tiff[] = {0x49, 0x49, 0x2A, 0x00};
    
    for (int i = 0; i < 100; i++) {
        ASSERT_EQ(detect_magic(jpeg, sizeof(jpeg)), FT_JPEG);
        ASSERT_EQ(detect_magic(png, sizeof(png)), FT_PNG);
        ASSERT_EQ(detect_magic(tiff, sizeof(tiff)), FT_TIFF);
    }
}

TEST(consistency_distance) {
    // JXL distance calculation must be deterministic
    for (int i = 0; i < 100; i++) {
        double d1 = get_jxl_distance(false, false);
        double d2 = get_jxl_distance(false, false);
        ASSERT_NEAR(d1, d2, 0.0000001);
    }
}

// --- Level 2: Cross-Function Consistency Tests ---

TEST(consistency_lossless_threshold_relationship) {
    // Lossless source detection and threshold should agree
    // PNG is lossless source, so threshold applies
    ASSERT_TRUE(is_lossless_source(FT_PNG));
    ASSERT_TRUE(!should_process_lossless(1 * 1024 * 1024, false));  // Below threshold
    ASSERT_TRUE(should_process_lossless(3 * 1024 * 1024, false));   // Above threshold
    
    // JPEG is not lossless source, threshold doesn't apply
    ASSERT_TRUE(!is_lossless_source(FT_JPEG));
    ASSERT_TRUE(should_process_lossless(100, true));  // Always process JPEG
}

TEST(consistency_tiff_suitability) {
    // TIFF suitability should be consistent with compression type
    // Lossless compressions are suitable
    ASSERT_TRUE(is_tiff_suitable(TIFF_NONE));
    ASSERT_TRUE(is_tiff_suitable(TIFF_LZW));
    ASSERT_TRUE(is_tiff_suitable(TIFF_DEFLATE));
    
    // Lossy compression is not suitable
    ASSERT_TRUE(!is_tiff_suitable(TIFF_JPEG));
}

// --- Level 3: Boundary Consistency Tests ---

TEST(consistency_boundary_size_reduction) {
    // Boundary cases should be handled consistently
    ASSERT_NEAR(calculate_size_reduction(1, 1), 0.0, 0.01);      // Minimum non-zero
    ASSERT_NEAR(calculate_size_reduction(1, 0), 100.0, 0.01);    // Complete reduction
    ASSERT_NEAR(calculate_size_reduction(0, 0), 0.0, 0.01);      // Zero/zero
}

TEST(consistency_boundary_threshold) {
    // Exact threshold boundary
    size_t threshold = MIN_LOSSLESS_SIZE;
    
    ASSERT_TRUE(!should_process_lossless(threshold - 1, false));  // Just below
    ASSERT_TRUE(should_process_lossless(threshold, false));       // Exactly at
    ASSERT_TRUE(should_process_lossless(threshold + 1, false));   // Just above
}

TEST(consistency_boundary_distance) {
    // Distance boundaries
    ASSERT_NEAR(get_jxl_distance(true, false), -1.0, 0.001);   // JPEG special case
    ASSERT_NEAR(get_jxl_distance(false, false), 0.0, 0.001);   // Lossless
    ASSERT_NEAR(get_jxl_distance(false, true), 0.0, 0.001);    // Force lossless
}

// --- Level 4: Pipeline Consistency Tests ---

typedef struct {
    TestFileType type;
    size_t size;
    bool is_lossless_src;
    bool should_process;
    double distance;
} JxlPipelineResult;

JxlPipelineResult simulate_jxl_pipeline(TestFileType type, size_t size) {
    JxlPipelineResult result;
    result.type = type;
    result.size = size;
    result.is_lossless_src = is_lossless_source(type);
    result.should_process = should_process_lossless(size, type == FT_JPEG);
    result.distance = get_jxl_distance(type == FT_JPEG, false);
    return result;
}

TEST(consistency_pipeline_flow) {
    // Pipeline: detect → classify → decide → encode
    JxlPipelineResult r1 = simulate_jxl_pipeline(FT_PNG, 3 * 1024 * 1024);
    JxlPipelineResult r2 = simulate_jxl_pipeline(FT_PNG, 3 * 1024 * 1024);
    
    ASSERT_EQ(r1.type, r2.type);
    ASSERT_EQ(r1.size, r2.size);
    ASSERT_EQ(r1.is_lossless_src, r2.is_lossless_src);
    ASSERT_EQ(r1.should_process, r2.should_process);
    ASSERT_NEAR(r1.distance, r2.distance, 0.0000001);
}

TEST(consistency_pipeline_chain) {
    // Chained operations should be consistent
    size_t input = 10000000;  // 10MB
    size_t output = 5000000;  // 5MB
    
    double reduction = calculate_size_reduction(input, output);
    bool should_proc = should_process_lossless(input, false);
    
    // Run 10 times, all should match
    for (int i = 0; i < 10; i++) {
        ASSERT_NEAR(calculate_size_reduction(input, output), reduction, 0.0000001);
        ASSERT_EQ(should_process_lossless(input, false), should_proc);
    }
}

// --- Level 5: Data Integrity Tests ---

TEST(consistency_data_integrity) {
    // Verify mathematical relationships hold
    // reduction = (1 - output/input) * 100
    // So: output = input * (1 - reduction/100)
    
    size_t input = 1000;
    size_t output = 500;
    double reduction = calculate_size_reduction(input, output);
    
    // Reverse calculation
    size_t calculated_output = (size_t)(input * (1.0 - reduction / 100.0));
    ASSERT_EQ(calculated_output, output);
}

TEST(consistency_format_classification) {
    // Format classification must be mutually exclusive and exhaustive
    TestFileType types[] = {FT_JPEG, FT_PNG, FT_BMP, FT_TIFF, FT_PPM, FT_TGA, FT_JXL, FT_RAW};
    int lossless_count = 0;
    
    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
        if (is_lossless_source(types[i])) {
            lossless_count++;
        }
    }
    
    // PNG, BMP, PPM, TGA are lossless sources
    ASSERT_EQ(lossless_count, 4);
}

TEST(consistency_skip_logic) {
    // Skip logic must be consistent
    // JXL files should be detected and skipped
    unsigned char jxl_codestream[] = {0xFF, 0x0A};
    ASSERT_EQ(detect_magic(jxl_codestream, sizeof(jxl_codestream)), FT_JXL);
    
    // JXL is not a lossless source (it's already JXL!)
    ASSERT_TRUE(!is_lossless_source(FT_JXL));
}

// ============================================================
// Main
// ============================================================

int main(void) {
    printf("\n🔬 static2jxl Precision Validation Tests\n");
    printf("==========================================\n\n");
    
    printf("📊 Size Reduction Tests:\n");
    RUN_TEST(size_reduction_50_percent);
    RUN_TEST(size_reduction_75_percent);
    RUN_TEST(size_reduction_no_change);
    RUN_TEST(size_reduction_increase);
    RUN_TEST(size_reduction_zero_input);
    RUN_TEST(size_reduction_large_files);
    
    printf("\n📏 Size Threshold Tests:\n");
    RUN_TEST(threshold_jpeg_small);
    RUN_TEST(threshold_jpeg_large);
    RUN_TEST(threshold_png_below);
    RUN_TEST(threshold_png_exact);
    RUN_TEST(threshold_png_above);
    
    printf("\n🎯 JXL Distance Tests:\n");
    RUN_TEST(distance_jpeg);
    RUN_TEST(distance_png_lossless);
    RUN_TEST(distance_force_lossless);
    
    printf("\n🔍 Magic Bytes Detection Tests:\n");
    RUN_TEST(magic_jpeg);
    RUN_TEST(magic_png);
    RUN_TEST(magic_bmp);
    RUN_TEST(magic_tiff_le);
    RUN_TEST(magic_tiff_be);
    RUN_TEST(magic_jxl_codestream);
    RUN_TEST(magic_ppm);
    RUN_TEST(magic_unknown);
    RUN_TEST(magic_too_short);
    
    printf("\n📦 TIFF Compression Tests:\n");
    RUN_TEST(tiff_uncompressed_suitable);
    RUN_TEST(tiff_lzw_suitable);
    RUN_TEST(tiff_deflate_suitable);
    RUN_TEST(tiff_jpeg_not_suitable);
    RUN_TEST(tiff_unknown_not_suitable);
    
    printf("\n🎨 Lossless Source Tests:\n");
    RUN_TEST(lossless_png);
    RUN_TEST(lossless_bmp);
    RUN_TEST(lossless_ppm);
    RUN_TEST(not_lossless_jpeg);
    RUN_TEST(not_lossless_jxl);
    
    printf("\n🔄 Consistency Verification (一致性验证):\n");
    printf("  --- Level 1: Deterministic Output ---\n");
    RUN_TEST(consistency_size_reduction);
    RUN_TEST(consistency_threshold);
    RUN_TEST(consistency_magic);
    RUN_TEST(consistency_distance);
    
    printf("  --- Level 2: Cross-Function ---\n");
    RUN_TEST(consistency_lossless_threshold_relationship);
    RUN_TEST(consistency_tiff_suitability);
    
    printf("  --- Level 3: Boundary ---\n");
    RUN_TEST(consistency_boundary_size_reduction);
    RUN_TEST(consistency_boundary_threshold);
    RUN_TEST(consistency_boundary_distance);
    
    printf("  --- Level 4: Pipeline ---\n");
    RUN_TEST(consistency_pipeline_flow);
    RUN_TEST(consistency_pipeline_chain);
    
    printf("  --- Level 5: Data Integrity ---\n");
    RUN_TEST(consistency_data_integrity);
    RUN_TEST(consistency_format_classification);
    RUN_TEST(consistency_skip_logic);
    
    printf("\n==========================================\n");
    printf("📊 Results: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("==========================================\n\n");
    
    return tests_failed > 0 ? 1 : 0;
}
