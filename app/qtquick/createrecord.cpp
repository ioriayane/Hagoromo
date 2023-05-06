#include "createrecord.h"
#include "../atprotocol/comatprotorepocreaterecord.h"

CreateRecord::CreateRecord(QObject *parent) : CreateSession { parent } { }

void CreateRecord::post(const QString &text) const
{
    ComAtprotoRepoCreateRecord *create_record = new ComAtprotoRepoCreateRecord();
    connect(create_record, &ComAtprotoRepoCreateRecord::finished, [=](bool success) {
        qDebug() << "ComAtprotoRepoCreateRecord::finish" << success;
        create_record->deleteLater();
    });
    create_record->setService(service());
    create_record->setSession(did(), handle(), email(), accessJwt(), refreshJwt());
    create_record->createRecord(text);
}
