#ifndef SUPPORTPROVIDER_H
#define SUPPORTPROVIDER_H

#include <QString>

// プロフィールのリンクが支援サイト（Patreon、Ko-fiなど）のアカウントページか判定する
// 1stparty/social-app/src/features/profileLinks/providers.ts の getSupportProvider() 相当
class SupportProvider
{
public:
    // 該当する支援サイトの名前（該当しない場合は空）
    static QString name(const QString &url);
    // 該当する支援サイトのアイコン名（app/qml/images/community/<logo>.svg、該当しない場合は空）
    static QString logo(const QString &url);
    static bool isSupportProvider(const QString &url);
};

#endif // SUPPORTPROVIDER_H
