// Copyright (c) Signal Estimator authors
// Licensed under MIT

#pragma once

#include "core/Config.hpp"
#include "core/Frame.hpp"

#include <cstdint>
#include <cstdlib>

#include <alsa/asoundlib.h>

namespace signal_estimator {

class AlsaReader {
public:
    AlsaReader() = default;
    ~AlsaReader();

    AlsaReader(const AlsaReader&) = delete;
    AlsaReader& operator=(const AlsaReader&) = delete;

    bool open(Config& config, const char* device);
    void close();

    bool read(Frame& frame);

private:
    Config config_;
    snd_pcm_t* pcm_ {};
};

} // namespace signal_estimator
