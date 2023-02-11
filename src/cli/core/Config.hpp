// Copyright (c) Signal Estimator authors
// Licensed under MIT

#pragma once

#include "core/Time.hpp"

#include <cstdlib>

namespace signal_estimator {

struct Config {
    // number of pre-allocated frames in frame pool
    size_t frame_pool_size { 128 };

    // total measurement duration
    float measurement_duration { 10 };

    // how long do not process signals in seconds
    float warmup_duration { 0 };

    // number of samples per second per channel
    unsigned int sample_rate { 48000 };

    // stereo (L and R)
    unsigned int n_channels { 2 };

    // test signal volume
    float volume { 0.5f };

    // desired io latency in microseconds
    unsigned int io_latency_us { 8000 };

    // periods (bursts) per io buffer
    unsigned int io_num_periods { 2 };

    // number of samples per io period
    size_t io_period_size { 0 };

    // sma window for latency reports, in seconds
    size_t report_sma_window { 5 };

    // dump compression ratio
    size_t dump_compression { 64 };

    // interval between strikes in seconds
    float step_period { 1 };

    // strike length in seconds
    float step_length { 0.1f };

    // running maximum window for strike detection
    size_t step_detection_window { 96 };

    // strike detection threshold
    float step_detection_threshold { 0.4f };

    // the impulse latency estimator peak detection window length in samples
    size_t impulse_peak_detection_width { 128 };

    // the peak-to-noise minimum ratio threshold
    float impulse_avg_2_peak_ration_threshold { 4.0 };

    // interval between impulses in seconds
    float impulse_period { 1 };

    // running maximum window for signal detection
    size_t signal_detection_window { 48 };

    // signal detection threshold
    float signal_detection_threshold { 0.15f };

    // glitch detection window
    size_t glitch_detection_window { 32 };

    // glitch detection threshold
    float glitch_detection_threshold { 0.05f };

    // get warmup duration in samples
    size_t warmup_samples() const {
        return size_t(sample_rate * warmup_duration) * n_channels;
    }

    // get warmup duration in periods
    size_t warmup_periods() const {
        return warmup_samples() / io_period_size;
    }

    // get test duration in samples
    size_t total_samples() const {
        return size_t(sample_rate * measurement_duration) * n_channels;
    }

    // get test duration in periods
    size_t total_periods() const {
        return total_samples() / io_period_size;
    }

    // convert number of samples (all channels) to number of nanoseconds
    nanoseconds_t samples_to_ns(size_t num_samples) const {
        return nanoseconds_t(num_samples) / n_channels * 1000000000 / sample_rate;
    }

    // convert number of samples (per channel) to number of nanoseconds
    nanoseconds_t frames_to_ns(size_t num_samples) const {
        return nanoseconds_t(num_samples) * 1000000000 / sample_rate;
    }
};

} // namespace signal_estimator
