#pragma once

#include <QPointF>

#include <QVector>

#include <deque>

class PointsBuffer {
public:
    void append_point(QPointF);

    QVector<QPointF> get_current_points();
    void clear_buf();

    int size_milliseconds() const {
        return ring_size_milliseconds_;
    }

private:
    const int ring_size_milliseconds_ = 5000;

    std::deque<QPointF> ringbuf_;
};
