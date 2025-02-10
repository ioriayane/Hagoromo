#ifndef SYSTEMTOOL_H
#define SYSTEMTOOL_H

#include <QObject>
#include <QRegularExpression>
#include <QUrl>

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
    Q_INVOKABLE static void setFlicableWheelDeceleration(qreal deceleration);
    Q_INVOKABLE QStringList possibleRealtimeFeedServiceEndpoints() const;
    Q_INVOKABLE void changeRealtimeFeedServiceEndpoint(const QString &endpoint);

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
