#include "reporter.h"

#include "atprotocol/com/atproto/moderation/comatprotomoderationcreatereport.h"

using AtProtocolInterface::ComAtprotoModerationCreateReport;

Reporter::Reporter(QObject *parent) : QObject { parent }, m_running(false)
{
    m_reasonHash[ReasonSpam] = "reasonSpam";
    m_reasonHash[ReasonSexual] = "reasonSexual";
    m_reasonHash[ReasonRude] = "reasonRude";
    m_reasonHash[ReasonViolation] = "reasonViolation";
    m_reasonHash[ReasonOther] = "reasonOther";
    m_reasonHash[ReasonMisleading] = "reasonMisleading";
}

void Reporter::setAccount(const QString &service, const QString &did, const QString &handle,
                          const QString &email, const QString &accessJwt, const QString &refreshJwt)
{
    m_account.service = service;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
}

void Reporter::reportPost(const QString &uri, const QString &cid, Reporter::ReportReason reason)
{
    qDebug() << "reportPost" << uri << cid << reason;
    if (uri.isEmpty() || cid.isEmpty() || !m_reasonHash.contains(reason))
        return;
    if (running())
        return;
    setRunning(true);

    ComAtprotoModerationCreateReport *report = new ComAtprotoModerationCreateReport(this);
    connect(report, &ComAtprotoModerationCreateReport::finished, [=](bool success) {
        setRunning(false);
        emit errorOccured(report->errorCode(), report->errorMessage());
        emit finished(success);
        report->deleteLater();
    });
    report->setAccount(m_account);
    report->reportPost(uri, cid, m_reasonHash[reason]);
}

void Reporter::reportAccount(const QString &did, ReportReason reason)
{
    qDebug() << "reportAccount" << did << reason;
    if (did.isEmpty() || !m_reasonHash.contains(reason))
        return;
    if (running())
        return;
    setRunning(true);

    ComAtprotoModerationCreateReport *report = new ComAtprotoModerationCreateReport(this);
    connect(report, &ComAtprotoModerationCreateReport::finished, [=](bool success) {
        setRunning(false);
        emit errorOccured(report->errorCode(), report->errorMessage());
        emit finished(success);
        report->deleteLater();
    });
    report->setAccount(m_account);
    report->reportAccount(did, m_reasonHash[reason]);
}

bool Reporter::running() const
{
    return m_running;
}

void Reporter::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}
