#include "labelprovider.h"

#include "atprotocol/app/bsky/labeler/appbskylabelergetservices.h"

using AtProtocolInterface::AppBskyLabelerGetServices;

LabelProvider::LabelProvider(QObject *parent)
    : QObject { parent }, m_lang(QLocale::system().language())
{
}

LabelProvider::~LabelProvider() { }

LabelProvider *LabelProvider::getInstance()
{
    static LabelProvider instance;
    return &instance;
}

void LabelProvider::update(const QStringList labelers,
                           const AtProtocolInterface::AccountData &account,
                           LabelConnector *connector)
{
    QStringList labeler_cue;

    for (const auto &labeler : labelers) {
        if (m_lablers.contains(labeler)) {
            // 取得済み
            qDebug().noquote() << "LABEL: Already exist:" << labeler;
            if (connector) {
                emit connector->finished(labeler);
            }
        } else if (nowWorking(labeler)) {
            // 既に取得中
            qDebug().noquote() << "LABEL: Now downloading:" << labeler;
            appendConnector(labeler, connector);
        } else {
            qDebug().noquote() << "LABEL: Download:" << labeler;
            labeler_cue.append(labeler);
            appendConnector(labeler, connector);
        }
    }

    if (labeler_cue.isEmpty()) {
        return;
    }

    // ラベラー取得
    AppBskyLabelerGetServices *services = new AppBskyLabelerGetServices(this);
    connect(services, &AppBskyLabelerGetServices::finished, [=](bool success) {
        if (success) {
            for (const auto &labeler : services->viewsLabelerViewDetailedList()) {
                m_lablers[labeler.creator.did] = labeler;
                for (auto c : m_connectors[labeler.creator.did]) {
                    qDebug().noquote() << labeler.creator.did << c;
                    if (!c) {
                        // already deleted
                    } else {
                        emit c->finished(labeler.creator.did);
                    }
                }
                m_connectors[labeler.creator.did].clear();
            }
        }
        services->deleteLater();
    });
    services->setAccount(account);
    services->getServices(labeler_cue, true);
}

LabelData LabelProvider::getLabel(const QString &labeler_did, const QString &id) const
{
    if (!m_lablers.contains(labeler_did))
        return LabelData();

    LabelData ret;
    AtProtocolType::AppBskyLabelerDefs::LabelerViewDetailed labeler = m_lablers[labeler_did];

    ret.avatar = labeler.creator.avatar;
    for (const auto &def : labeler.policies.labelValueDefinitions) {
        if (def.identifier != id)
            continue;

        for (const auto &locale : def.locales) {
            const QLocale::Language lang = QLocale(locale.lang).language();
            if (ret.name.isEmpty()) {
                // 1個目はフォールバックとして必ず入れておく
                ret.name = locale.name;
            } else if (lang == language()) {
                ret.name = locale.name;
            }
        }
    }
    if (ret.name.isEmpty()) {
        ret.name = id;
    }

    return ret;
}

void LabelProvider::appendConnector(const QString &did, LabelConnector *connector)
{
    if (!m_connectors[did].contains(connector)) {
        m_connectors[did].append(connector);
    }
}

void LabelProvider::setLanguage(const QString &language)
{
    m_lang = QLocale(language).language();
}

QLocale::Language LabelProvider::language() const
{
    return m_lang;
}

bool LabelProvider::nowWorking(const QString &did)
{
    // コネクタが空じゃないということは取得中
    return !(m_connectors[did].isEmpty());
}
