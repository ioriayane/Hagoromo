#ifndef CONFIGURABLELABELS_H
#define CONFIGURABLELABELS_H

#include "atprotocol/accessatprotocol.h"

enum class ConfigurableLabelStatus : int {
    Hide,
    Warning,
    Show,
    Unknown, // デフォルトでしか使用しないこと
};

enum class ConfigurableLabelLevel : int {
    Alert,
    Inform,
    None,
};

struct LabelerItem
{
    QString did;
    QString handle;
    QString display_name;
    QString description;
};

struct ConfigurableLabelItem
{
    QString labeler_did; // ラベルの持ち主のラベラーのDID
    QString id; // プリファレンスとかと一致させるID
    QString title; // ラベル一覧に表示するタイトル
    QString subtitle; // ラベル一覧に表示する説明文
    QString warning; // 隠すときに表示するメッセージ
    QStringList values; // プロフィールやポストに設定されているラベルとマッチングさせる値
    bool is_adult_imagery = false; // 画像を隠すかどうか
    ConfigurableLabelLevel level =
            ConfigurableLabelLevel::Alert; // ユーザーへの通知レベル(今のところ表現上の差はなし)
    ConfigurableLabelStatus status =
            ConfigurableLabelStatus::Hide; // プロフィールやポストをどうするか
    ConfigurableLabelStatus default_status = ConfigurableLabelStatus::Unknown;
    bool configurable = true; // ラベルの設定を上書きできるか
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

    int count(const QString &labeler_did = QString()) const;
    bool load();
    void loadLabelers(const QStringList &dids, std::function<void(bool)> callback);
    bool save();
    int indexOf(const QString &id, const QString &labeler_did = QString()) const;
    ConfigurableLabelStatus visibility(const QString &label, const bool for_image,
                                       const QString &labeler_did = QString()) const;
    QString message(const QString &label, const bool for_image,
                    const QString &labeler_did = QString()) const;
    QString title(const int index, const QString &labeler_did = QString()) const;
    QString title(const QString &label, const bool for_image,
                  const QString &labeler_did = QString()) const;
    QString description(const int index, const QString &labeler_did = QString()) const;
    ConfigurableLabelStatus status(const int index, const QString &labeler_did = QString()) const;
    void setStatus(const int index, const ConfigurableLabelStatus status,
                   const QString &labeler_did = QString());
    bool isAdultImagery(const int index, const QString &labeler_did = QString()) const;
    bool configurable(const int index, const QString &labeler_did = QString()) const;

    int labelerCount() const;
    QStringList labelerDids() const;
    QString labelerHandle(const QString &did) const;
    QString labelerDisplayName(const QString &did) const;
    QString labelerDescription(const QString &did) const;

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

    bool refreshLabelers() const;
    void setRefreshLabelers(bool newRefreshLabelers);
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
    QString removeSharp(const QString &value) const;
    ConfigurableLabelStatus toLabelStatus(const QString &visibility) const;
    ConfigurableLabelLevel toLabelLevel(const QString &severity) const;

    QRegularExpression m_regSpace;
    QMap<QString, LabelerItem> m_labelers; // QMap<labeler_did, labeler>
    QMap<QString, QList<ConfigurableLabelItem>> m_labels; // QMap<labeler_did, labels>
    // ポストをスペース区切りするときはこれとは別にワードをハッシュで保存しておく
    // 日本語のときは逆向きで部分一致させる
    QList<MutedWordItem> m_mutedWords;
    QHash<QString, MutedWordItem> m_mutedWordsHash;
    QHash<QString, MutedWordItem> m_mutedWordsTagHash;
    bool m_refreshLabelers;
    bool m_enableAdultContent;
    bool m_running;
};

#endif // CONFIGURABLELABELS_H
