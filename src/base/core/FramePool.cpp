// Copyright (c) Signal Estimator authors
// Licensed under MIT

#include "core/FramePool.hpp"

namespace signal_estimator {

FramePool::FramePool(const Config& config)
    : config_(config) {
    for (size_t n = 0; n < config.frame_pool_size; n++) {
        free_list_.enqueue(create_frame_());
    }
}

FramePool::~FramePool() {
    Frame* frame = nullptr;

    while (free_list_.try_dequeue(frame)) {
        delete_frame_(frame);
    }
}

std::shared_ptr<Frame> FramePool::allocate() {
    Frame* frame = nullptr;

    if (free_list_.try_dequeue(frame)) {
        frame->clear();
    } else {
        frame = create_frame_();
    }

    return std::shared_ptr<Frame>(
        frame, [](auto frame) { frame->pool_.release_frame_(frame); });
}

void FramePool::release_frame_(Frame* frame) {
    free_list_.enqueue(frame);
}

Frame* FramePool::create_frame_() {
    return new Frame(config_, *this);
}

void FramePool::delete_frame_(Frame* frame) {
    delete frame;
}

} // namespace signal_estimator
