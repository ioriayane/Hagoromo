#ifndef SYSTEMTOOL_H
#define SYSTEMTOOL_H

#include <QObject>
#include <QRegularExpression>
#include <QUrl>

#define REG_EXP_URL                                                                                \
    "http[s]?://"                                                                                  \
    "(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F])|#(?:[a-zA-Z]|[0-9]|[$-_@" \
    ".&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))*)+"
#define REG_EXP_MENTION                                                                            \
    "@(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\.)+(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-" \
    "zA-Z0-9])?)"
#define REG_EXP_MENTION_PART "[a-zA-Z0-9][a-zA-Z0-9.-]*"
#define REG_EXP_HASH_TAG                                                                           \
    QStringLiteral("(?:^|"                                                                         \
                   "[ \\f\\n\\r\\t\\v%1%2%3-%4%5%6%7%8%9%10])"                                     \
                   "(?:#[^0-9 \\f\\n\\r\\t\\v%1%2%3-%4%5%6%7%8%9%10]"                              \
                   "[^ \\f\\n\\r\\t\\v%1%2%3-%4%5%6%7%8%9%10]*)"                                   \
                   "(?:$|[ \\f\\n\\r\\t\\v%1%2%3-%4%5%6%7%8%9%10]?)")                              \
            .arg(QChar(0x00a0))                                                                    \
            .arg(QChar(0x1680))                                                                    \
            .arg(QChar(0x2000))                                                                    \
            .arg(QChar(0x200a))                                                                    \
            .arg(QChar(0x2028))                                                                    \
            .arg(QChar(0x2029))                                                                    \
            .arg(QChar(0x202f))                                                                    \
            .arg(QChar(0x205f))                                                                    \
            .arg(QChar(0x3000))                                                                    \
            .arg(QChar(0xfeff))

// #define REG_EXP_HASH_TAG "(?:^|\\s)(#[^\\d\\s]\\S*)(?=\\s)"
// (?:^|[ \t\r\n])(#[^0-9 \t\r\n][^ \t\r\n]*)(?=[ \t\r\n])?
// (?:^|[ \t\r\n])(#[^0-9 \t\r\n][^ \t\r\n]*)([^ \t\r\n])?
class SystemTool : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString applicationVersion READ applicationVersion CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)

public:
    explicit SystemTool(QObject *parent = nullptr);

    Q_INVOKABLE void copyToClipboard(const QString &text) const;
    Q_INVOKABLE int countText(const QString &text) const;
    Q_INVOKABLE QUrl clipImage(const QUrl &url, const int x, const int y, const int width,
                               const int height) const;
    Q_INVOKABLE void updateFont(const QString &family);
    Q_INVOKABLE static QString defaultFontFamily();

    QString applicationVersion() const;
    QString qtVersion() const;

    QString markupText(const QString &text) const;

private:
    QObject *findRootType(QObject *object);
    void updateFontOfChildType(QObject *object, const QFont &font);
    void updateFontProperty(QObject *item, const QFont &font);

    QRegularExpression m_rxUrl;
};

#endif // SYSTEMTOOL_H
