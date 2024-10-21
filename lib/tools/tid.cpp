#include "tid.h"
#include "base32.h"
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>

QString Tid::next()
{
    static qint64 prev_time = 0;
    static qint64 prev = 0;
    qint64 current_time = QDateTime::currentMSecsSinceEpoch();
    if (current_time <= prev_time) {
        current_time = prev_time + 1;
    }
    prev_time = current_time;

    qint64 clock_id = QRandomGenerator::global()->bounded(0, 1024);
    qint64 id = (((current_time * 1000) << 10) & 0x7FFFFFFFFFFFFC00) | (clock_id & 0x3FF);

    if (prev == id) {
        prev_time = current_time++;
        id = (((current_time * 1000) << 10) & 0x7FFFFFFFFFFFFC00) | (clock_id & 0x3FF);
    }
    prev = id;
    return Base32::encode_s(id);
}
