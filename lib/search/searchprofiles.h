#ifndef SEARCHPROFILES_H
#define SEARCHPROFILES_H

#include <atprotocol/accessatprotocol.h>

namespace SearchInterface {

class SearchProfiles : public AtProtocolInterface::AccessAtProtocol
{
    Q_OBJECT
public:
    explicit SearchProfiles(QObject *parent = nullptr);

    void search(const QString &text);

    const QStringList *didList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QStringList m_didList;
};

}

#endif // SEARCHPROFILES_H
