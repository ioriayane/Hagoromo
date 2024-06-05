#ifndef ABSTRACTPOSTSELECTOR_H
#define ABSTRACTPOSTSELECTOR_H

#include <QObject>
#include <QJsonObject>

class AbstractPostSelector : public QObject
{
    Q_OBJECT
public:
    explicit AbstractPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object) = 0;

    void appendChildSelector(AbstractPostSelector *child);
    QString did() const;
    void setDid(const QString &newDid);

signals:
    void selected(const QJsonObject &object);

protected:
    const QList<AbstractPostSelector *> &children() const;

private:
    QList<AbstractPostSelector *> m_children;
    QString m_did;
};

#endif // ABSTRACTPOSTSELECTOR_H
