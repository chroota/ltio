
#include "count_min_sketch.h"
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include "hash/murmurhash3.h"

namespace component {
/*
//static
CountMinSketch::CreateWithWidthAndDepth(uint64_t w, uint32_t d) {
}
static CreateWithErrorRateAndCertainty(uint64_t w, uint32_t d);
*/

/* d=[ln(1−certainty)/ln(1/2)] */
CountMinSketch::CountMinSketch(double err, double certainty)
  : certainty_(certainty), error_rate_(err), distinct_count_(0) {
  width_ = static_cast<uint64_t>(std::ceil(2.0 / err));
  depth_ = static_cast<uint64_t>(
      std::ceil(std::log(1.0 - certainty_) / std::log(1.0 / 2.0)));

  for (uint32_t i = 0; i < depth_; i++) {
    CountType* d = (CountType*)malloc(width_ * sizeof(CountType));
    memset(d, 0, width_ * sizeof(CountType));
    matrix_.push_back(d);
  }
}

CountMinSketch::~CountMinSketch() {
  for (auto* p : matrix_) {
    free(p);
  }
  matrix_.clear();
}

void CountMinSketch::Increase(const std::string& key, int32_t count) {
  Increase(key.data(), key.size(), count);
}

void CountMinSketch::Increase(const void* data, size_t len, int32_t count) {
  uint64_t result[2];
  bool brand_new = false;
  for (uint32_t depth = 0; depth < depth_;) {
    MurmurHash3_x64_128(data, len, depth << 0x1F | depth, result);
    if (0 == __sync_fetch_and_add(&matrix_[depth++][result[0] % width_],
                                  count) &&
        !brand_new) {
      brand_new = true;
    };
    if (depth < depth_) {
      if (0 == __sync_fetch_and_add(&matrix_[depth++][result[1] % width_],
                                    count) &&
          !brand_new) {
        brand_new = true;
      };
    }
  }
  if (brand_new) {
    distinct_count_++;
  }
}

uint64_t CountMinSketch::Estimate(const std::string& key) {
  return Estimate(key.data(), key.size());
}

uint64_t CountMinSketch::Estimate(const void* data, size_t len) {
  uint64_t result = 0xFFFFFFFFFFFFFFFF;

  uint64_t hash[2];
  for (uint32_t depth = 0; depth < depth_;) {
    MurmurHash3_x64_128(data, len, depth << 0x1F | depth, hash);
    result = std::min(uint64_t(matrix_[depth++][hash[0] % width_]), result);
    if (depth < depth_) {
      result = std::min(uint64_t(matrix_[depth++][hash[1] % width_]), result);
    }
  }
  return result;
}

}  // namespace component
