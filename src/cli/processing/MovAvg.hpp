// Copyright (c) Signal Estimator authors
// Licensed under MIT

#pragma once

#include <algorithm>
#include <array>

namespace signal_estimator {

template <typename T> class MovAvg {
public:
    explicit MovAvg(const size_t window_sz)
        : window_sz_(window_sz)
        , buff_(window_sz)
        , init_counter_(0)
        , cur_i_(0)
        , accum_(0) {
        std::fill(buff_.begin(), buff_.end(), T { 0 });
    }

    T operator()(const T x) {
        accum_ -= buff_[cur_i_];
        accum_ += x;
        buff_[cur_i_] = x;
        if (++cur_i_ >= window_sz_) {
            cur_i_ = 0;
        }
        if(init_counter_ < window_sz_){
            init_counter_++;
        }
        return accum_ / T(init_counter_);
    }

private:
    const size_t window_sz_;
    std::vector<T> buff_;
    size_t init_counter_;
    size_t cur_i_;
    T accum_;
};

} // signal_estimator
