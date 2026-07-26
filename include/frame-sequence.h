// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2026
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.

#ifndef RPI_FRAME_SEQUENCE_H
#define RPI_FRAME_SEQUENCE_H

#include <stddef.h>
#include <stdint.h>

#include <functional>
#include <vector>

namespace rgb_matrix {
class RGBMatrix;

// A lightweight in-memory sequence of RGB frames and hold-times.
//
// Frames are stored as packed RGB24 rows in display order (R,G,B per pixel).
// This allows simple playback without requiring stream files.
class FrameSequence {
public:
  typedef std::function<int(void)> BrightnessProvider;
  typedef std::function<bool(void)> StopProvider;

  FrameSequence(int width, int height);

  int width() const;
  int height() const;
  size_t frame_count() const;

  // Add one frame in packed RGB24 format.
  // byte_count must be width * height * 3.
  bool AddFrame(const uint8_t *rgb24, size_t byte_count, uint32_t hold_time_us);

  // Write the sequence to a .fseq file.
  bool WriteToFile(const char *path) const;

  // Read a sequence from a .fseq file.
  bool ReadFromFile(const char *path);

  void Clear();

  // Play the sequence in a loop until interrupt_received is set (if provided).
  // If brightness_provider is provided, it is polled per frame and applied
  // to the matrix (clamped to 1..100).
  void Play(RGBMatrix *matrix,
            volatile bool *interrupt_received = NULL,
            const BrightnessProvider &brightness_provider = BrightnessProvider()) const;

  // Callback-driven form of Play(), useful for language binding wrappers.
  // If stop_provider is set, it is checked between frames to stop playback.
  void Play(RGBMatrix *matrix,
            const StopProvider &stop_provider,
            const BrightnessProvider &brightness_provider = BrightnessProvider()) const;

private:
  struct Frame {
    std::vector<uint8_t> rgb24;
    uint32_t hold_time_us;
  };

  int width_;
  int height_;
  size_t frame_size_;
  std::vector<Frame> frames_;
};

}  // namespace rgb_matrix

#endif  // RPI_FRAME_SEQUENCE_H