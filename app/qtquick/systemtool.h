#ifndef SYSTEMTOOL_H
#define SYSTEMTOOL_H

#include <QObject>
#include <QRegularExpression>

#define REG_EXP_URL                                                                                \
    "http[s]?://"                                                                                  \
    "(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F])|#(?:[a-zA-Z]|[0-9]|[$-_@" \
    ".&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))*)+"
#define REG_EXP_MENTION                                                                            \
    "@(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\.)+(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-" \
    "zA-Z0-9])?)"

class SystemTool : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString applicationVersion READ applicationVersion CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)

public:
    explicit SystemTool(QObject *parent = nullptr);

    Q_INVOKABLE void copyToClipboard(const QString &text) const;
    Q_INVOKABLE int countText(const QString &text) const;

    QString applicationVersion() const;
    QString qtVersion() const;

    QString markupText(const QString &text);

private:
    QRegularExpression m_rxUrl;
};

#endif // SYSTEMTOOL_H
