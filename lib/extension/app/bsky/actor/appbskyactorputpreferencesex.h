#ifndef APPBSKYACTORPUTPREFERENCESEX_H
#define APPBSKYACTORPUTPREFERENCESEX_H

#include "atprotocol/app/bsky/actor/appbskyactorputpreferences.h"

namespace AtProtocolInterface {

class AppBskyActorPutPreferencesEx : public AppBskyActorPutPreferences
{
public:
    explicit AppBskyActorPutPreferencesEx(QObject *parent = nullptr);

    QJsonArray updatePreferencesJson(const QString &src, const QString &type,
                                     const QJsonObject &part);
    QJsonObject makePostInteractionSettingsPref(const QString &thread_gate_type,
                                                const QStringList &thread_gate_options,
                                                const bool post_gate_quote_enabled) const;

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYACTORPUTPREFERENCESEX_H
