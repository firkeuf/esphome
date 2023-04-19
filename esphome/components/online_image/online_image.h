#pragma once
#ifdef USE_ARDUINO

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/display/image.h"
#include "esphome/core/helpers.h"

#include "image_decoder.h"

namespace esphome {
namespace online_image {
/**
 * @brief Format that the image is encoded with.
 */
enum ImageFormat {
  /** Automatically detect from MIME type. */
  AUTO,
  /** JPEG format. Not supported yet. */
  JPEG,
  /** PNG format. */
  PNG,
};

class OnlineImage: public PollingComponent,
                   public display::Image
{
 public:
  /**
   * @brief Construct a new Online Image object.
   *
   * @param url URL to download the image from.
   * @param width Desired width of the target image area.
   * @param height Desired height of the target image area.
   * @param format Format that the image is encoded in (@see ImageFormat).
   * @param buffer_size Size of the buffer used to download the image.
   */
  OnlineImage(const std::string &url, uint32_t width, uint32_t height, ImageFormat format, display::ImageType type, uint32_t buffer_size);

  bool get_pixel(int x, int y) const override;
  Color get_rgba_pixel(int x, int y) const override;
  Color get_color_pixel(int x, int y) const override;
  Color get_rgb565_pixel(int x, int y) const override;
  Color get_grayscale_pixel(int x, int y) const override;

  void update() override;

  void set_url(const std::string &url) { url_ = url; }
  void release();

 protected:
  using Allocator = ExternalRAMAllocator<uint8_t>;
  Allocator allocator_{  Allocator::Flags::ALLOW_FAILURE };

  uint32_t get_buffer_size() const { return get_buffer_size(width_, height_); }
  uint32_t get_buffer_size(uint32_t width, uint32_t height) const { return std::ceil(bits_per_pixel_ * width * height / 8.0); }

  uint32_t get_position(uint32_t x, uint32_t y) const {
    return ((x + y * width_) * bits_per_pixel_) / 8;
  }

  ALWAYS_INLINE bool auto_resize() const { return fixed_width_ == 0 || fixed_height_ == 0; }

  bool resize(uint32_t width, uint32_t height);
  void draw_pixel(uint32_t x, uint32_t y, Color color);

  uint8_t *buffer_;
  std::string url_;
  const uint32_t download_buffer_size_;
  const ImageFormat format_;
  const uint8_t bits_per_pixel_;
  const uint8_t fixed_width_;
  const uint8_t fixed_height_;

  friend void ImageDecoder::set_size(uint32_t width, uint32_t height);
  friend void ImageDecoder::draw(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const Color &color);
};

template<typename... Ts> class OnlineImageSetUrlAction : public Action<Ts...> {
 public:
  OnlineImageSetUrlAction(OnlineImage *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(const char *, url)
  void play(Ts... x) override {
    this->parent_->set_url(this->url_.value(x...));
    this->parent_->update();
  }
 protected:
  OnlineImage *parent_;
};

}  // namespace online_image
}  // namespace esphome

#endif  // USE_ARDUINO
