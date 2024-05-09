#include "base32.h"

QString Base32::encode(const QByteArray &data, const bool with_padding)
{
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz234567";
    QString result;

    for (int i = 0; i < data.size(); i += 5) {
        quint64 val = 0;
        int count = qMin(5, data.size() - i);
        for (int j = 0; j < count; ++j) {
            val = (val << 8) | (data[i + j] & 0xFF);
        }

        for (int j = 0; j < 8; ++j) {
            if ((count * 8 - j * 5) > 0) {
                if ((count * 8 - (j + 1) * 5) >= 0) {
                    result.append(alphabet[(val >> (count * 8 - (j + 1) * 5)) & 0x1F]);
                } else {
                    result.append(alphabet[(val << -1 * (count * 8 - (j + 1) * 5)) & 0x1F]);
                }
            } else if (with_padding) {
                result.append('=');
            }
        }
    }

    return result;
}
