#ifndef CONFIGURABLELABELS_H
#define CONFIGURABLELABELS_H

#include "atprotocol/accessatprotocol.h"

enum class ConfigurableLabelStatus : int {
    Hide,
    Warning,
    Show,
};

struct ConfigurableLabelItem
{
    QString id;
    QString title;
    QString subtitle;
    QString warning;
    QStringList values;
    bool is_adult_imagery = false;
    ConfigurableLabelStatus status = ConfigurableLabelStatus::Hide;
    bool configurable = true;
};

enum class MutedWordTarget : int {
    Content,
    Tag,
};

struct MutedWordItem
{
    int group = 0; // app.bsky.actor.defs#mutedWordsPrefが複数入れられるので区別する番号
    QString value;
    QList<MutedWordTarget> targets;
};

class ConfigurableLabels : public AtProtocolInterface::AtProtocolAccount
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

public:
    explicit ConfigurableLabels(QObject *parent = nullptr);

    ConfigurableLabels &operator=(ConfigurableLabels &other);

    int count() const;
    bool load();
    bool save();
    int indexOf(const QString &id) const;
    ConfigurableLabelStatus visibility(const QString &label, const bool for_image) const;
    QString message(const QString &label, const bool for_image) const;
    QString title(const int index) const;
    QString title(const QString &label, const bool for_image) const;
    QString description(const int index) const;
    ConfigurableLabelStatus status(const int index) const;
    void setStatus(const int index, const ConfigurableLabelStatus status);
    bool isAdultImagery(const int index) const;
    bool configurable(const int index) const;

    int mutedWordCount() const;
    MutedWordItem getMutedWordItem(const int index) const;
    void insertMutedWord(const int index, const QString &value,
                         const QList<MutedWordTarget> &targets);
    void updateMutedWord(const int index, const QString &value,
                         const QList<MutedWordTarget> &targets);
    void removeMutedWordItem(const int index);
    void moveMutedWordItem(const int from, const int to);
    int indexOfMutedWordItem(const QString &value) const;
    bool containsMutedWords(const QString &text, const QStringList &tags,
                            const bool partial_match) const;
    void clearMutedWord();

    bool enableAdultContent() const;
    void setEnableAdultContent(bool newEnableAdultContent);
    bool running() const;
    void setRunning(bool newRunning);

signals:
    void runningChanged(bool running);
    void finished(bool success);

protected:
    virtual bool parseJson(bool success, const QString reply_json)
    {
        Q_UNUSED(success)
        Q_UNUSED(reply_json)
        return true;
    }

private:
    void initializeLabels();
    bool putPreferences(const QString &json);
    QString updatePreferencesJson(const QString &src_json);

    QRegularExpression m_regSpace;
    QList<ConfigurableLabelItem> m_labels;
    // ポストをスペース区切りするときはこれとは別にワードをハッシュで保存しておく
    // 日本語のときは逆向きで部分一致させる
    QList<MutedWordItem> m_mutedWords;
    QHash<QString, MutedWordItem> m_mutedWordsHash;
    bool m_enableAdultContent;
    bool m_running;
};

#endif // CONFIGURABLELABELS_H
