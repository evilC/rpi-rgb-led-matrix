// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#include "frame-sequence.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>

#include "led-matrix.h"

namespace rgb_matrix {

namespace {
static inline uint8_t ClampBrightness(int value) {
  if (value < 1) return 1;
  if (value > 100) return 100;
  return value;
}

static bool ShouldStop(const volatile bool *interrupt_received) {
  return interrupt_received != NULL && *interrupt_received;
}
}  // namespace

FrameSequence::FrameSequence(int width, int height)
    : width_(width), height_(height), frame_size_(0) {
  if (width_ > 0 && height_ > 0) {
    frame_size_ = static_cast<size_t>(width_) * static_cast<size_t>(height_) * 3;
  }
}

int FrameSequence::width() const { return width_; }
int FrameSequence::height() const { return height_; }
size_t FrameSequence::frame_count() const { return frames_.size(); }

bool FrameSequence::AddFrame(const uint8_t *rgb24,
                             size_t byte_count,
                             uint32_t hold_time_us) {
  if (rgb24 == NULL || frame_size_ == 0 || byte_count != frame_size_) {
    return false;
  }

  Frame frame;
  frame.rgb24.resize(frame_size_);
  memcpy(frame.rgb24.data(), rgb24, frame_size_);
  frame.hold_time_us = hold_time_us;
  frames_.push_back(frame);
  return true;
}

void FrameSequence::Clear() {
  frames_.clear();
}

void FrameSequence::Play(
    RGBMatrix *matrix,
    volatile bool *interrupt_received,
    const BrightnessProvider &brightness_provider) const {
  if (matrix == NULL || frames_.empty()) {
    return;
  }

  if (matrix->width() != width_ || matrix->height() != height_) {
    fprintf(stderr, "FrameSequence is %dx%d but matrix is %dx%d\n",
            width_, height_, matrix->width(), matrix->height());
    return;
  }

  FrameCanvas *offscreen = matrix->CreateFrameCanvas();
  if (offscreen == NULL) {
    return;
  }

  int last_brightness = -1;
  while (!ShouldStop(interrupt_received)) {
    for (std::vector<Frame>::const_iterator it = frames_.begin();
         it != frames_.end(); ++it) {
      if (ShouldStop(interrupt_received)) break;

      if (brightness_provider) {
        const int wanted = ClampBrightness(brightness_provider());
        if (wanted != last_brightness) {
          matrix->SetBrightness(wanted);
          last_brightness = wanted;
        }
      }

      const uint8_t *p = it->rgb24.data();
      for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
          offscreen->SetPixel(x, y, p[0], p[1], p[2]);
          p += 3;
        }
      }

      offscreen = matrix->SwapOnVSync(offscreen);
      if (it->hold_time_us > 0) {
        usleep(it->hold_time_us);
      }
    }
  }
}

}  // namespace rgb_matrix