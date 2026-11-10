#ifndef QSTRINGEX_H
#define QSTRINGEX_H

#include <QString>

class QStringEx : public QString
{
public:
    inline QStringEx() { }
    inline QStringEx(const QStringEx &other) : QString(other) { }
    inline QStringEx(const QString &other) : QString(other) { }

    void operator=(const QStringEx &value) { QString::operator=(value); }

    int length() const
    {
        int count = 0;
        for (int i = 0; i < QString::length(); i++) {
            if (at(i).isLowSurrogate() || at(i).combiningClass() > 0) {
                continue;
            }
            count++;
        }
        return count;
    }
};

#endif // QSTRINGEX_H
