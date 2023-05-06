#include "createrecord.h"
#include "../atprotocol/comatprotorepocreaterecord.h"

CreateRecord::CreateRecord(QObject *parent) : QObject { parent } { }

void CreateRecord::setAccount(const QString &service, const QString &did, const QString &handle,
                              const QString &email, const QString &accessJwt,
                              const QString &refreshJwt)
{
    m_account.service = service;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
}

void CreateRecord::post(const QString &text)
{
    ComAtprotoRepoCreateRecord *create_record = new ComAtprotoRepoCreateRecord();
    connect(create_record, &ComAtprotoRepoCreateRecord::finished, [=](bool success) {
        emit finished(success);
        create_record->deleteLater();
    });
    create_record->setService(m_account.service);
    create_record->setSession(m_account.did, m_account.handle, m_account.email, m_account.accessJwt,
                              m_account.refreshJwt);
    create_record->createRecord(text);
}
