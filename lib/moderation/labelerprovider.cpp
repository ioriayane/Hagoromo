#include "labelerprovider.h"

#include <QDebug>
#include <QPointer>
#include <QDateTime>

class LabelerProvider::Private : public AtProtocolInterface::AtProtocolAccount
{
public:
    explicit Private(LabelerProvider *parent);
    ~Private();

    void appendConnector(LabelerConnector *connector);
    void cleanConnector();

    void update(LabelerConnector *connector, const RefleshMode mode);
    QStringList labelerDids() const;
    ConfigurableLabelStatus visibility(const QString &label, const bool for_image,
                                       const QString &labeler_did = QString()) const;
    QString message(const QString &label, const bool for_image,
                    const QString &labeler_did = QString()) const;
    QString title(const QString &label, const bool for_image,
                  const QString &labeler_did = QString()) const;
    bool containsMutedWords(const QString &text, const QStringList &tags,
                            const bool partial_match) const;
    ConfigurableLabelStatus
    getContentFilterStatus(const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels,
                           const bool for_media) const;

    int refreshInterval() const;
    void setRefreshInterval(qint64 newRefreshInterval);

private:
    LabelerProvider *q;

    bool checkRefleshAll(const RefleshMode mode);

    QList<QPointer<LabelerConnector>> m_connector;
    ConfigurableLabels m_labels;
    bool m_initialized;
    qint64 m_lastUpdatedTime;
    qint64 m_refreshInterval; // sec, default 1hour
};

LabelerProvider::Private::Private(LabelerProvider *parent)
    : q(parent), m_initialized(false), m_lastUpdatedTime(0), m_refreshInterval(60 * 60)
{
    connect(&m_labels, &ConfigurableLabels::finished, this, [=](bool success) {
        qDebug().noquote() << this << "finished" << success;
        for (auto connector : std::as_const(m_connector)) {
            if (!connector) {
                // already deleted
                qDebug().noquote() << this << "already deleted";
            } else {
                emit connector->finished(success);
            }
        }
        m_initialized = success;
    });
}

LabelerProvider::Private::~Private()
{
    qDebug().noquote() << this << "LabelerProvider::Private::~Private()";
}

void LabelerProvider::Private::appendConnector(LabelerConnector *connector)
{
    if (!m_connector.contains(connector)) {
        m_connector.append(connector);
    }
}

void LabelerProvider::Private::cleanConnector()
{
    for (int i = m_connector.length() - 1; i >= 0; i--) {
        if (!m_connector[i]) {
            m_connector.removeAt(i);
            qDebug().noquote() << "remove connector" << m_connector.length();
        }
    }
}

void LabelerProvider::Private::update(LabelerConnector *connector, const RefleshMode mode)
{
    if (connector == nullptr)
        return;

    cleanConnector();

    if (m_labels.running()) {
        // すでに実行中の場合はコネクタだけ登録して抜ける
        qDebug().noquote() << this << "already running";
        appendConnector(connector);
        return;
    }
    if (m_initialized && mode == RefleshMode::None) {
        qDebug().noquote() << this << "don't update labeler and filter";
        emit connector->finished(true);
        return;
    }

    appendConnector(connector);
    m_labels.setAccount(account());
    m_labels.setRefreshLabelers(checkRefleshAll(mode));
    if (!m_labels.load()) {
        emit connector->finished(false);
    }
}

QStringList LabelerProvider::Private::labelerDids() const
{
    return m_labels.labelerDids();
}

ConfigurableLabelStatus LabelerProvider::Private::visibility(const QString &label,
                                                             const bool for_image,
                                                             const QString &labeler_did) const
{
    return m_labels.visibility(label, for_image, labeler_did);
}

QString LabelerProvider::Private::message(const QString &label, const bool for_image,
                                          const QString &labeler_did) const
{
    return m_labels.message(label, for_image, labeler_did);
}

QString LabelerProvider::Private::title(const QString &label, const bool for_image,
                                        const QString &labeler_did) const
{
    return m_labels.title(label, for_image, labeler_did);
}

bool LabelerProvider::Private::containsMutedWords(const QString &text, const QStringList &tags,
                                                  const bool partial_match) const
{
    return m_labels.containsMutedWords(text, tags, partial_match);
}

ConfigurableLabelStatus LabelerProvider::Private::getContentFilterStatus(
        const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels, const bool for_media) const
{
    for (const auto &label : labels) {
        ConfigurableLabelStatus status = m_labels.visibility(label.val, for_media, label.src);
        if (status != ConfigurableLabelStatus::Show) {
            return status;
        }
    }
    return ConfigurableLabelStatus::Show;
}

int LabelerProvider::Private::refreshInterval() const
{
    return m_refreshInterval;
}

void LabelerProvider::Private::setRefreshInterval(qint64 newRefreshInterval)
{
    m_refreshInterval = newRefreshInterval;
}

bool LabelerProvider::Private::checkRefleshAll(const RefleshMode mode)
{
    bool ret = false;
    if (m_initialized) {
        qint64 now = QDateTime::currentSecsSinceEpoch();
        if (mode == RefleshMode::RefleshAll) {
            ret = true;
        } else if (now < m_lastUpdatedTime) {
            // ループしたときは面倒なので即時
            ret = true;
        } else {
            ret = ((now - m_lastUpdatedTime) >= m_refreshInterval);
        }
    } else {
        // 初期化前はつねにフルスペックで取得
        ret = true;
    }

    if (ret) {
        m_lastUpdatedTime = QDateTime::currentSecsSinceEpoch();
    }
    return ret;
}

LabelerProvider::LabelerProvider(QObject *parent) : QObject { parent } { }

LabelerProvider::~LabelerProvider()
{
    qDebug().noquote() << this << "~LabelerProvider()";
    clear();
}

LabelerProvider *LabelerProvider::getInstance()
{
    static LabelerProvider instance;
    return &instance;
}

void LabelerProvider::clear()
{
    for (const auto &key : d.keys()) {
        delete d[key];
    }
    d.clear();
}

void LabelerProvider::setAccount(const AtProtocolInterface::AccountData &account)
{
    if (!account.isValid())
        return;
    const QString key = account.accountKey();
    if (!d.contains(key)) {
        d[key] = new LabelerProvider::Private(this);
    }
    d[key]->setAccount(account);
}

void LabelerProvider::update(const AtProtocolInterface::AccountData &account,
                             LabelerConnector *connector, const RefleshMode mode)
{
    if (connector == nullptr)
        return;

    const QString key = getAccountKey(account);
    if (key.isEmpty()) {
        emit connector->finished(false);
        return;
    }
    d[key]->update(connector, mode);
}

int LabelerProvider::refreshInterval(const AtProtocolInterface::AccountData &account) const
{
    const QString key = getAccountKey(account);
    if (key.isEmpty()) {
        return -1;
    }
    return d[key]->refreshInterval();
}

void LabelerProvider::setRefreshInterval(const AtProtocolInterface::AccountData &account,
                                         qint64 newRefreshInterval)
{
    const QString key = getAccountKey(account);
    if (key.isEmpty()) {
        return;
    }
    d[key]->setRefreshInterval(newRefreshInterval);
}

QStringList LabelerProvider::labelerDids(const AtProtocolInterface::AccountData &account) const
{
    const QString key = getAccountKey(account);
    if (key.isEmpty()) {
        return QStringList();
    }
    return d[key]->labelerDids();
}

ConfigurableLabelStatus LabelerProvider::visibility(const AtProtocolInterface::AccountData &account,
                                                    const QString &label, const bool for_image,
                                                    const QString &labeler_did) const
{
    const QString key = getAccountKey(account);
    if (key.isEmpty()) {
        return ConfigurableLabelStatus::Show;
    }
    return d[key]->visibility(label, for_image, labeler_did);
}

QString LabelerProvider::message(const AtProtocolInterface::AccountData &account,
                                 const QString &label, const bool for_image,
                                 const QString &labeler_did) const
{
    const QString key = getAccountKey(account);
    if (key.isEmpty()) {
        return QString();
    }
    return d[key]->message(label, for_image, labeler_did);
}

QString LabelerProvider::title(const AtProtocolInterface::AccountData &account,
                               const QString &label, const bool for_image,
                               const QString &labeler_did) const
{
    const QString key = getAccountKey(account);
    if (key.isEmpty()) {
        return QString();
    }
    return d[key]->title(label, for_image, labeler_did);
}

bool LabelerProvider::containsMutedWords(const AtProtocolInterface::AccountData &account,
                                         const QString &text, const QStringList &tags,
                                         const bool partial_match) const
{
    const QString key = getAccountKey(account);
    if (key.isEmpty()) {
        return false;
    }
    return d[key]->containsMutedWords(text, tags, partial_match);
}

ConfigurableLabelStatus LabelerProvider::getContentFilterStatus(
        const AtProtocolInterface::AccountData &account,
        const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels, const bool for_media) const
{
    const QString key = getAccountKey(account);
    if (key.isEmpty()) {
        return ConfigurableLabelStatus::Show;
    }
    return d[key]->getContentFilterStatus(labels, for_media);
}

QString LabelerProvider::getAccountKey(const AtProtocolInterface::AccountData &account) const
{
    if (!account.isValid()) {
        return QString();
    }
    const QString key = account.accountKey();
    if (!d.contains(key)) {
        return QString();
    }
    return key;
}
