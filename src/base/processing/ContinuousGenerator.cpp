// Copyright (c) Signal Estimator authors
// Licensed under MIT

#include "processing/ContinuousGenerator.hpp"

#include <cmath>
#include <cstring>

namespace signal_estimator {

ContinuousGenerator::ContinuousGenerator(const Config& config)
    : config_(config) {
}

void ContinuousGenerator::generate(Frame& frame) {
    for (size_t sn = 0; sn < frame.size();) {
        for (size_t cn = 0; cn < config_.n_channels; cn++) {
            frame[sn++] = sample_t(MaxSample * config_.volume
                * std::sin(2 * M_PI / config_.sample_rate * 500 * pos_));
        }
        pos_++;
    }
}

} // namespace signal_estimator
