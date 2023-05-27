#include "atpabstractlistmodel.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "translator.h"

#include <QPointer>

using namespace AtProtocolType;

AtpAbstractListModel::AtpAbstractListModel(QObject *parent)
    : QAbstractListModel { parent }, m_running(false), m_loadingInterval(5 * 60 * 1000)
{
    connect(&m_timer, &QTimer::timeout, this, &AtpAbstractListModel::getLatest);
}

AtProtocolInterface::AccountData AtpAbstractListModel::account() const
{
    return m_account;
}

void AtpAbstractListModel::setAccount(const QString &service, const QString &did,
                                      const QString &handle, const QString &email,
                                      const QString &accessJwt, const QString &refreshJwt)
{
    m_account.service = service;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
}

void AtpAbstractListModel::translate(const QString &cid)
{
    // indexで指定しないと同じcidが複数含まれる場合に正しく対応できない
    // indexにすると処理を始めてから追加の読み込みがあるとズレる

    QString record_text = getRecordText(cid);
    if (record_text.isEmpty())
        return;
    int row = indexOf(cid);
    if (row == -1)
        return;

    QPointer<AtpAbstractListModel> aliving(this);

    Translator *translator = new Translator();
    connect(translator, &Translator::finished, [=](const QString text) {
        if (aliving) {
            if (!text.isEmpty()) {
                m_translations[cid] = text;
                emit dataChanged(index(row), index(row));
            }
        }
        translator->deleteLater();
    });
    m_translations[cid] = "Now translating ...";
    emit dataChanged(index(row), index(row));
    translator->translate(record_text);
}

bool AtpAbstractListModel::running() const
{
    return m_running;
}

void AtpAbstractListModel::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QString AtpAbstractListModel::formatDateTime(const QString &value) const
{
    return QDateTime::fromString(value, Qt::ISODateWithMs).toLocalTime().toString("MM/dd hh:mm");
}

bool AtpAbstractListModel::autoLoading() const
{
    return m_timer.isActive();
}

void AtpAbstractListModel::setAutoLoading(bool newAutoLoading)
{
    if (newAutoLoading) {
        // Off -> On
        if (m_timer.isActive())
            return;
        m_timer.start(m_loadingInterval);
    } else {
        // * -> Off
        m_timer.stop();
    }
    emit autoLoadingChanged();
}

int AtpAbstractListModel::loadingInterval() const
{
    return m_loadingInterval;
}

void AtpAbstractListModel::setLoadingInterval(int newLoadingInterval)
{
    if (m_loadingInterval == newLoadingInterval)
        return;
    m_loadingInterval = newLoadingInterval;
    m_timer.setInterval(m_loadingInterval);
    emit loadingIntervalChanged();
}
