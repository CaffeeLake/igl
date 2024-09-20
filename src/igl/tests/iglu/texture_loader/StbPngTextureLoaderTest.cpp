/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <IGLU/texture_loader/stb_png/Header.h>
#include <IGLU/texture_loader/stb_png/TextureLoaderFactory.h>
#include <cstring>
#include <igl/vulkan/util/TextureFormat.h>
#include <numeric>
#include <vector>

namespace igl::tests::stb::png {

namespace {
constexpr const std::array<uint8_t, 67> kSingleBlackPixelGrayscreenPNG{
    {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48,
     0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00,
     0x00, 0x3A, 0x7E, 0x9B, 0x55, 0x00, 0x00, 0x00, 0x0A, 0x49, 0x44, 0x41, 0x54, 0x08,
     0xD7, 0x63, 0x60, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0xE2, 0x21, 0xBC, 0x33, 0x00,
     0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82}};
constexpr const std::array<uint8_t, 160> kSingleWhitePixelGrayscreenJPG{
    {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x01, 0x00,
     0x48, 0x00, 0x48, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43, 0x00, 0x03, 0x02, 0x02, 0x03, 0x02,
     0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x03, 0x03, 0x04, 0x05, 0x08, 0x05, 0x05, 0x04, 0x04,
     0x05, 0x0A, 0x07, 0x07, 0x06, 0x08, 0x0C, 0x0A, 0x0C, 0x0C, 0x0B, 0x0A, 0x0B, 0x0B, 0x0D,
     0x0E, 0x12, 0x10, 0x0D, 0x0E, 0x11, 0x0E, 0x0B, 0x0B, 0x10, 0x16, 0x10, 0x11, 0x13, 0x14,
     0x15, 0x15, 0x15, 0x0C, 0x0F, 0x17, 0x18, 0x16, 0x14, 0x18, 0x12, 0x14, 0x15, 0x14, 0xFF,
     0xC0, 0x00, 0x0B, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x11, 0x00, 0xFF, 0xC4, 0x00,
     0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x09, 0xFF, 0xC4, 0x00, 0x14, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xDA, 0x00, 0x08,
     0x01, 0x01, 0x00, 0x00, 0x3F, 0x00, 0x54, 0xDF, 0xFF, 0xD9}};

std::vector<uint8_t> populateMinimalValidFile(uint32_t width, uint32_t height) {
  const uint8_t w1 = static_cast<uint8_t>((width & 0xFF000000) >> 24);
  const uint8_t w2 = static_cast<uint8_t>((width & 0xFF0000) >> 16);
  const uint8_t w3 = static_cast<uint8_t>((width & 0xFF00) >> 8);
  const uint8_t w4 = static_cast<uint8_t>(width & 0xFF);
  const uint8_t h1 = static_cast<uint8_t>((height & 0xFF000000) >> 24);
  const uint8_t h2 = static_cast<uint8_t>((height & 0xFF0000) >> 16);
  const uint8_t h3 = static_cast<uint8_t>((height & 0xFF00) >> 8);
  const uint8_t h4 = static_cast<uint8_t>(height & 0xFF);
  return {
      0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, // File Marker
      0x00, 0x00, 0x00, 0x0D, // Chunk Length
      'I',  'H',  'D',  'R', // IHDR chunk
      w1,   w2,   w3,   w4, // Width
      h1,   h2,   h3,   h4, // Height
      0x08, // Bit Depth
      0x02, // Color Type
      0x00, // Compression Method
      0x00, // Filter Method
      0x00, // Interlace Method
      0x00, 0x00, 0x00, 0x00, // Chunk CRC
      0x00, 0x00, 0x00, 0x00, // Chunk Length
      'I',  'D',  'A',  'T', // IHDR chunk
      0x00, 0x00, 0x00, 0x00, // Chunk CRC
  };
}

} // namespace

class StbPngTextureLoaderTest : public ::testing::Test {
 public:
  void SetUp() override {}

  void TearDown() override {}

  iglu::textureloader::stb::png::TextureLoaderFactory factory_;
};

TEST_F(StbPngTextureLoaderTest, EmptyBuffer_Fails) {
  std::vector<uint8_t> buffer;
  buffer.resize(iglu::textureloader::stb::png::kHeaderLength);

  Result ret;
  auto reader = *iglu::textureloader::DataReader::tryCreate(
      buffer.data(), static_cast<uint32_t>(buffer.size()), nullptr);
  auto loader = factory_.tryCreate(reader, &ret);
  EXPECT_EQ(loader, nullptr);
  EXPECT_FALSE(ret.isOk());
}

TEST_F(StbPngTextureLoaderTest, MinimumValidHeader_Succeeds) {
  const auto buffer = populateMinimalValidFile(32u, 64u);

  Result ret;
  auto reader = *iglu::textureloader::DataReader::tryCreate(
      buffer.data(), static_cast<uint32_t>(buffer.size()), nullptr);
  auto loader = factory_.tryCreate(reader, &ret);
  EXPECT_NE(loader, nullptr);
  EXPECT_TRUE(ret.isOk()) << ret.message;
}

TEST_F(StbPngTextureLoaderTest, ValidHeaderWithExtraData_Succeeds) {
  auto buffer = populateMinimalValidFile(32u, 64u);
  buffer.resize(buffer.size() + 1);

  Result ret;
  auto reader = *iglu::textureloader::DataReader::tryCreate(
      buffer.data(), static_cast<uint32_t>(buffer.size()), nullptr);

  auto loader = factory_.tryCreate(reader, &ret);
  EXPECT_NE(loader, nullptr);
  EXPECT_TRUE(ret.isOk()) << ret.message;
}

TEST_F(StbPngTextureLoaderTest, InsufficientData_Fails) {
  auto buffer = populateMinimalValidFile(32u, 64u);
  buffer.resize(buffer.size() - 8);

  Result ret;
  auto reader = *iglu::textureloader::DataReader::tryCreate(
      buffer.data(), static_cast<uint32_t>(buffer.size()), nullptr);
  auto loader = factory_.tryCreate(reader, &ret);
  EXPECT_EQ(loader, nullptr);
  EXPECT_FALSE(ret.isOk());
}

TEST_F(StbPngTextureLoaderTest, JpegData_Fails) {
  Result ret;
  auto reader = *iglu::textureloader::DataReader::tryCreate(
      kSingleWhitePixelGrayscreenJPG.data(),
      static_cast<uint32_t>(kSingleWhitePixelGrayscreenJPG.size()),
      nullptr);
  auto loader = factory_.tryCreate(reader, &ret);
  EXPECT_EQ(loader, nullptr);
  EXPECT_FALSE(ret.isOk());
}

TEST_F(StbPngTextureLoaderTest, PngData_Succeeds) {
  Result ret;
  auto reader = *iglu::textureloader::DataReader::tryCreate(
      kSingleBlackPixelGrayscreenPNG.data(),
      static_cast<uint32_t>(kSingleBlackPixelGrayscreenPNG.size()),
      nullptr);
  auto loader = factory_.tryCreate(reader, &ret);
  EXPECT_NE(loader, nullptr);
  EXPECT_TRUE(ret.isOk()) << ret.message;
}
} // namespace igl::tests::stb::png
