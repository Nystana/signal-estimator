// Copyright (c) Signal Estimator authors
// Licensed under MIT

#include "run/Runner.hpp"
#include "core/Log.hpp"
#include "core/Realtime.hpp"
#include "dumps/AsyncDumper.hpp"
#include "dumps/CsvDumper.hpp"
#include "io/AlsaReader.hpp"
#include "io/AlsaWriter.hpp"
#include "processing/ContinuousGenerator.hpp"
#include "processing/CorrelationLatencyEstimator.hpp"
#include "processing/Impulse.hpp"
#include "processing/ImpulseGenerator.hpp"
#include "processing/LossEstimator.hpp"
#include "processing/StepsGenerator.hpp"
#include "processing/StepsLatencyEstimator.hpp"
#include "reports/JsonReporter.hpp"
#include "reports/TextReporter.hpp"

namespace signal_estimator {

Runner::Runner(const Config& config)
    : config_(config) {
}

Runner::~Runner() {
    stop();
    wait();
}

bool Runner::failed() const {
    return fail_;
}

bool Runner::start() {
    {
        auto alsa_writer = std::make_unique<AlsaWriter>();
        // may update config
        if (!alsa_writer->open(config_, config_.output_dev.c_str())) {
            fail_ = true;
            return false;
        }
        output_writer_ = std::move(alsa_writer);
    }

    {
        auto alsa_reader = std::make_unique<AlsaReader>();
        // may update config
        if (!alsa_reader->open(config_, config_.input_dev.c_str())) {
            fail_ = true;
            return false;
        }
        input_reader_ = std::move(alsa_reader);
    }

    if (!config_.output_dump.empty()) {
        auto csv_dumper = std::make_unique<CsvDumper>(config_);
        if (!csv_dumper->open(config_.output_dump.c_str())) {
            fail_ = true;
            return false;
        }
        output_dumper_ = std::move(csv_dumper);
    }

    if (!config_.input_dump.empty()) {
        auto csv_dumper = std::make_unique<CsvDumper>(config_);
        if (!csv_dumper->open(config_.input_dump.c_str())) {
            fail_ = true;
            return false;
        }
        input_dumper_ = std::move(csv_dumper);
    }

    if (output_dumper_) {
        output_dumper_ = std::make_unique<AsyncDumper>(std::move(output_dumper_));
    }

    if (input_dumper_) {
        input_dumper_ = std::make_unique<AsyncDumper>(std::move(input_dumper_));
    }

    se_log_info("starting measurement");

    frame_pool_ = std::make_unique<FramePool>(config_);

    if (config_.report_format == "text") {
        reporter_ = std::make_unique<TextReporter>();
    } else if (config_.report_format == "json") {
        reporter_ = std::make_unique<JsonReporter>();
    }

    if (config_.mode == "latency_step") {
        generator_ = std::make_unique<StepsGenerator>(config_);
    } else if (config_.mode == "latency_corr") {
        generator_ = std::make_unique<ImpulseGenerator>(config_, impulse);
    } else if (config_.mode == "losses") {
        generator_ = std::make_unique<ContinuousGenerator>(config_);
    }

    if (config_.mode == "latency_step") {
        estimator_ = std::make_unique<StepsLatencyEstimator>(config_, *reporter_);
    } else if (config_.mode == "latency_corr") {
        estimator_ = std::make_unique<CorrelationLatencyEstimator>(config_, *reporter_);
    } else if (config_.mode == "losses") {
        estimator_ = std::make_unique<LossEstimator>(config_, *reporter_);
    }

    output_thread_ = std::thread(&Runner::output_loop_, this);
    input_thread_ = std::thread(&Runner::input_loop_, this);

    return true;
}

void Runner::stop() {
    stop_ = true;
}

void Runner::wait() {
    if (output_thread_.joinable()) {
        output_thread_.join();
    }

    if (input_thread_.joinable()) {
        input_thread_.join();
    }
}

void Runner::output_loop_() {
    make_realtime();

    size_t n = 0;

    for (; config_.io_num_periods > n; n++) {
        if (stop_ || fail_) {
            break;
        }

        auto frame = frame_pool_->allocate();

        if (!output_writer_->write(*frame)) {
            se_log_error("got error from output device, exiting");
            fail_ = true;
            break;
        }
    }

    for (; n < config_.total_periods(); n++) {
        if (stop_ || fail_) {
            break;
        }

        auto frame = frame_pool_->allocate();

        generator_->generate(*frame);

        if (!output_writer_->write(*frame)) {
            se_log_error("got error from output device, exiting");
            fail_ = true;
            break;
        }

        if (n < config_.warmup_periods()) {
            continue;
        }

        if (estimator_) {
            estimator_->add_output(frame);
        }

        if (output_dumper_) {
            output_dumper_->write(frame);
        }
    }

    if (estimator_) {
        estimator_->add_output(nullptr);
    }
}

void Runner::input_loop_() {
    make_realtime();

    for (size_t n = 0; n < config_.total_periods(); n++) {
        if (stop_ || fail_) {
            break;
        }

        auto frame = frame_pool_->allocate();

        if (!input_reader_->read(*frame)) {
            se_log_error("got error from input device, exiting");
            fail_ = true;
            break;
        }

        if (n < config_.warmup_periods()) {
            continue;
        }

        if (estimator_) {
            estimator_->add_input(frame);
        }

        if (input_dumper_) {
            input_dumper_->write(frame);
        }
    }

    if (estimator_) {
        estimator_->add_input(nullptr);
    }
}

} // namespace signal_estimator
