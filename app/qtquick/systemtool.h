#ifndef SYSTEMTOOL_H
#define SYSTEMTOOL_H

#include <QObject>
#include <QRegularExpression>

class SystemTool : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString applicationVersion READ applicationVersion CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)

public:
    explicit SystemTool(QObject *parent = nullptr);

    Q_INVOKABLE void copyToClipboard(const QString &text) const;

    QString applicationVersion() const;
    QString qtVersion() const;

    QString markupText(const QString &text);

private:
    QRegularExpression m_rxUrl;
};

#endif // SYSTEMTOOL_H
