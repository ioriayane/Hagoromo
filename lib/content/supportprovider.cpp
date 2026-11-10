#include "supportprovider.h"

#include <QUrl>

namespace {

struct ProviderRule
{
    const char *name;
    const char *logo; // app/qml/images/community/ のアイコンのファイル名（拡張子なし）
    const char *domain; // 登録可能ドメイン、または buy.stripe.com のような特定ホスト
    const char *path_prefix; // ドメインを支援以外のページと共有している場合のパス
};

// providers.ts の SUPPORT_PROVIDERS から scripts/update_support_providers.py で生成
// BEGIN SUPPORT_PROVIDERS (generated, please do not edit)
const ProviderRule SUPPORT_PROVIDERS[] = {
    { "BackerKit", "BackerKit", "backerkit.com", nullptr },
    { "Big Cartel", "BigCartel", "bigcartel.com", nullptr },
    { "Boosty", "Boosty", "boosty.to", nullptr },
    { "BOOTH", "Booth", "booth.pm", nullptr },
    { "Buy Me a Coffee", "BuyMeACoffee", "buymeacoffee.com", nullptr },
    { "Cash App", "CashApp", "cash.app", nullptr },
    { "Chuffed", "Chuffed", "chuffed.org", "/campaign/" },
    { "Chuffed", "Chuffed", "chuffed.org", "/project/" },
    { "Crowdfundr", "Crowdfundr", "crowdfundr.com", nullptr },
    { "Donorbox", "Donorbox", "donorbox.org", nullptr },
    { "Etsy", "Etsy", "etsy.com", nullptr },
    { "Fourthwall", "Fourthwall", "fourthwall.com", nullptr },
    { "Givebutter", "Givebutter", "givebutter.com", nullptr },
    { "GiveSendGo", "GiveSendGo", "givesendgo.com", nullptr },
    { "GoFundMe", "GoFundMe", "gofundme.com", nullptr },
    { "Gumroad", "Gumroad", "gumroad.com", nullptr },
    { "Indiegogo", "Indiegogo", "indiegogo.com", nullptr },
    { "INPRNT", "Inprint", "inprnt.com", "/gallery/" },
    { "itch.io", "ItchIo", "itch.io", nullptr },
    { "JustGiving", "JustGiving", "justgiving.com", nullptr },
    { "Kickstarter", "Kickstarter", "kickstarter.com", nullptr },
    { "Ko-fi", "KoFi", "ko-fi.com", nullptr },
    { "Liberapay", "Liberapay", "liberapay.com", nullptr },
    { "Open Collective", "OpenCollective", "opencollective.com", nullptr },
    { "Patreon", "Patreon", "patreon.com", nullptr },
    { "PayPal", "PayPal", "paypal.com", "/paypalme/" },
    { "PayPal", "PayPal", "paypal.me", nullptr },
    { "pixivFANBOX", "PixivFanbox", "fanbox.cc", nullptr },
    { "Redbubble", "Redbubble", "redbubble.com", nullptr },
    { "Skeb", "Skeb", "skeb.jp", nullptr },
    { "Stripe", "Stripe", "buy.stripe.com", nullptr },
    { "Stripe", "Stripe", "donate.stripe.com", nullptr },
    { "Substack", "Substack", "substack.com", nullptr },
    { "Throne", "Throne", "throne.com", nullptr },
    { "Venmo", "Venmo", "venmo.com", nullptr },
    { "VGen", "Vgen", "vgen.co", nullptr },
    { "Zeffy", "Zeffy", "zeffy.com", nullptr },
};
// END SUPPORT_PROVIDERS

// ドメイン自体とそのサブドメインのみ一致（ko-fi.com.example.net などは不一致）
bool hostMatchesDomain(const QString &host, const QString &domain)
{
    return host == domain || host.endsWith(QStringLiteral(".") + domain);
}

QString trimTrailingSlashes(QString text)
{
    while (text.endsWith(QLatin1Char('/'))) {
        text.chop(1);
    }
    return text;
}

// トップページではなくアカウント等の個別ページを指しているか
// （name.itch.io のようなサブドメイン、または ko-fi.com/name のようなパス）
bool isAccountUrl(const QString &host, const QString &path, const ProviderRule &rule)
{
    const QString domain = QString::fromLatin1(rule.domain);
    if (rule.path_prefix != nullptr) {
        const QString prefix = trimTrailingSlashes(QString::fromLatin1(rule.path_prefix));
        return path.startsWith(prefix + QStringLiteral("/"));
    }
    const bool is_account_subdomain = (host != domain && host != QStringLiteral("www.") + domain);
    return is_account_subdomain || !path.isEmpty();
}

const ProviderRule *findRule(const QString &url)
{
    const QUrl parsed(url);
    if (!parsed.isValid()
        || (parsed.scheme() != QStringLiteral("https")
            && parsed.scheme() != QStringLiteral("http")))
        return nullptr;
    const QString host = parsed.host().toLower();
    if (host.isEmpty())
        return nullptr;
    const QString path = trimTrailingSlashes(parsed.path().toLower());

    for (const auto &rule : SUPPORT_PROVIDERS) {
        if (hostMatchesDomain(host, QString::fromLatin1(rule.domain))
            && isAccountUrl(host, path, rule)) {
            return &rule;
        }
    }
    return nullptr;
}

}

QString SupportProvider::name(const QString &url)
{
    const ProviderRule *rule = findRule(url);
    return rule != nullptr ? QString::fromUtf8(rule->name) : QString();
}

QString SupportProvider::logo(const QString &url)
{
    const ProviderRule *rule = findRule(url);
    return rule != nullptr ? QString::fromLatin1(rule->logo) : QString();
}

bool SupportProvider::isSupportProvider(const QString &url)
{
    return findRule(url) != nullptr;
}
