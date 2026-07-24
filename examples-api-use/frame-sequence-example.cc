// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// Example showing how to decode an image/animation and play it from an
// in-memory frame sequence.

#include "frame-sequence.h"
#include "led-matrix.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <exception>
#include <Magick++.h>

using rgb_matrix::FrameSequence;
using rgb_matrix::RGBMatrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

using ImageVector = std::vector<Magick::Image>;

static bool HasSuffix(const char *value, const char *suffix) {
  if (value == NULL || suffix == NULL) return false;
  const size_t value_len = strlen(value);
  const size_t suffix_len = strlen(suffix);
  if (suffix_len > value_len) return false;
  return strcmp(value + value_len - suffix_len, suffix) == 0;
}

static ImageVector LoadAndScaleFrames(const char *filename,
                                      int target_width,
                                      int target_height) {
  ImageVector result;
  ImageVector frames;
  try {
    readImages(&frames, filename);
  } catch (std::exception &e) {
    if (e.what()) {
      fprintf(stderr, "%s\n", e.what());
    }
    return result;
  }

  if (frames.empty()) {
    fprintf(stderr, "No image found.\n");
    return result;
  }

  if (frames.size() > 1) {
    Magick::coalesceImages(&result, frames.begin(), frames.end());
  } else {
    result.push_back(frames[0]);
  }

  for (ImageVector::iterator it = result.begin(); it != result.end(); ++it) {
    it->scale(Magick::Geometry(target_width, target_height));
  }
  return result;
}

static int usage(const char *progname) {
  fprintf(stderr,
          "Usage: %s [led-matrix-options] <image-or-fseq> [-o <output.fseq>]\n",
          progname);
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

int main(int argc, char *argv[]) {
  Magick::InitializeMagick(*argv);
  const char *progname = argv[0];

  RGBMatrix::Options matrix_options;
  matrix_options.cols = 128;
  matrix_options.rows = 64;
  matrix_options.row_address_type = 5;
  matrix_options.parallel = 2;
  matrix_options.brightness = 50;

  rgb_matrix::RuntimeOptions runtime_options;
  runtime_options.gpio_slowdown = 5;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_options)) {
    return usage(progname);
  }

  if (argc != 2 && argc != 4) {
    return usage(progname);
  }

  const char *filename = argv[1];
  const char *output_fseq = NULL;
  if (argc == 4) {
    if (strcmp(argv[2], "-o") != 0) {
      return usage(progname);
    }
    output_fseq = argv[3];
  }

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  // Decode frames before matrix creation, as matrix init may drop privileges.
  const int target_width = matrix_options.cols * matrix_options.chain_length;
  const int target_height = matrix_options.rows * matrix_options.parallel;
  FrameSequence sequence(target_width, target_height);

  if (HasSuffix(filename, ".fseq")) {
    if (!sequence.ReadFromFile(filename)) {
      fprintf(stderr, "Failed to read frame sequence from '%s'\n", filename);
      return 1;
    }
  } else {
    ImageVector images = LoadAndScaleFrames(filename, target_width, target_height);
    if (images.empty()) {
      return 1;
    }

    const size_t frame_bytes =
        static_cast<size_t>(target_width) * static_cast<size_t>(target_height) * 3;
    std::vector<uint8_t> rgb(frame_bytes);
    for (ImageVector::const_iterator image = images.begin(); image != images.end(); ++image) {
      uint8_t *out = rgb.data();
      for (size_t y = 0; y < image->rows(); ++y) {
        for (size_t x = 0; x < image->columns(); ++x) {
          const Magick::Color &c = image->pixelColor(x, y);
          if (c.alphaQuantum() < 256) {
            *out++ = ScaleQuantumToChar(c.redQuantum());
            *out++ = ScaleQuantumToChar(c.greenQuantum());
            *out++ = ScaleQuantumToChar(c.blueQuantum());
          } else {
            *out++ = 0;
            *out++ = 0;
            *out++ = 0;
          }
        }
      }

      const uint32_t hold_us = image->animationDelay() > 0
                                   ? image->animationDelay() * 10000u
                                   : 100000u;
      if (!sequence.AddFrame(rgb.data(), rgb.size(), hold_us)) {
        fprintf(stderr, "Failed to add frame to sequence.\n");
        return 1;
      }
    }
  }

  if (output_fseq != NULL) {
    if (!sequence.WriteToFile(output_fseq)) {
      fprintf(stderr, "Failed to write frame sequence to '%s'\n", output_fseq);
      return 1;
    }
    fprintf(stderr, "Wrote %zu frames to %s\n", sequence.frame_count(), output_fseq);
    return 0;
  }

  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options,
                                                   runtime_options);
  if (matrix == NULL) {
    return 1;
  }

  sequence.Play(matrix, &interrupt_received);

  matrix->Clear();
  delete matrix;
  return 0;
}