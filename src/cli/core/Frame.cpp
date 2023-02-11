// Copyright (c) Signal Estimator authors
// Licensed under MIT

#include "core/Frame.hpp"

#include <cassert>

namespace signal_estimator {

Frame::Frame(const Config& config, Pool<Frame>* pool)
    : config_(&config)
    , pool_(pool)
    , data_(config_->io_period_size) {
    assert(data_.size() > 0);
}

size_t Frame::size() const {
    return data_.size();
}

const sample_t* Frame::data() const {
    return &data_[0];
}

sample_t* Frame::data() {
    return &data_[0];
}

sample_t Frame::at(size_t sample_index) const {
    return data_[sample_index];
}

FrameType Frame::type() const {
    return io_type_;
}

void Frame::set_type(FrameType type) {
    io_type_ = type;
}

void Frame::set_time() {
    io_time_ = monotonic_timestamp_ns();
}

nanoseconds_t Frame::sw_frame_time() const {
    return io_time_;
}

nanoseconds_t Frame::hw_frame_time() const {
    switch (io_type_) {
    case FrameType::Output:
        return io_time_
            + config_->samples_to_ns(
                (config_->io_num_periods - 1) * config_->io_period_size);

    case FrameType::Input:
        return io_time_ - config_->samples_to_ns(config_->io_period_size);
    }

    return 0;
}

nanoseconds_t Frame::hw_sample_time(size_t sample_index) const {
    switch (io_type_) {
    case FrameType::Output:
        return io_time_
            + config_->samples_to_ns(
                (config_->io_num_periods - 1) * config_->io_period_size + sample_index);

    case FrameType::Input:
        return io_time_ - config_->samples_to_ns(config_->io_period_size - sample_index);
    }

    return 0;
}

Pool<Frame>* Frame::pool() {
    return pool_;
}

} // namespace signal_estimator
