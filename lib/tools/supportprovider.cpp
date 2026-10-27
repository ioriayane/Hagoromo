#include "supportprovider.h"

#include <QUrl>

namespace {

struct ProviderRule
{
    const char *name;
    const char *domain; // 登録可能ドメイン、または buy.stripe.com のような特定ホスト
    const char *path_prefix; // ドメインを支援以外のページと共有している場合のパス
};

// providers.ts の SUPPORT_PROVIDERS から scripts/update_support_providers.py で生成
// BEGIN SUPPORT_PROVIDERS (generated, please do not edit)
const ProviderRule SUPPORT_PROVIDERS[] = {
    { "BackerKit", "backerkit.com", nullptr },
    { "Big Cartel", "bigcartel.com", nullptr },
    { "Boosty", "boosty.to", nullptr },
    { "BOOTH", "booth.pm", nullptr },
    { "Buy Me a Coffee", "buymeacoffee.com", nullptr },
    { "Cash App", "cash.app", nullptr },
    { "Chuffed", "chuffed.org", "/campaign/" },
    { "Chuffed", "chuffed.org", "/project/" },
    { "Crowdfundr", "crowdfundr.com", nullptr },
    { "Donorbox", "donorbox.org", nullptr },
    { "Etsy", "etsy.com", nullptr },
    { "Fourthwall", "fourthwall.com", nullptr },
    { "Givebutter", "givebutter.com", nullptr },
    { "GiveSendGo", "givesendgo.com", nullptr },
    { "GoFundMe", "gofundme.com", nullptr },
    { "Gumroad", "gumroad.com", nullptr },
    { "Indiegogo", "indiegogo.com", nullptr },
    { "INPRNT", "inprnt.com", "/gallery/" },
    { "itch.io", "itch.io", nullptr },
    { "JustGiving", "justgiving.com", nullptr },
    { "Kickstarter", "kickstarter.com", nullptr },
    { "Ko-fi", "ko-fi.com", nullptr },
    { "Liberapay", "liberapay.com", nullptr },
    { "Open Collective", "opencollective.com", nullptr },
    { "Patreon", "patreon.com", nullptr },
    { "PayPal", "paypal.com", "/paypalme/" },
    { "PayPal", "paypal.me", nullptr },
    { "pixivFANBOX", "fanbox.cc", nullptr },
    { "Redbubble", "redbubble.com", nullptr },
    { "Skeb", "skeb.jp", nullptr },
    { "Stripe", "buy.stripe.com", nullptr },
    { "Stripe", "donate.stripe.com", nullptr },
    { "Substack", "substack.com", nullptr },
    { "Throne", "throne.com", nullptr },
    { "Venmo", "venmo.com", nullptr },
    { "VGen", "vgen.co", nullptr },
    { "Zeffy", "zeffy.com", nullptr },
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

}

QString SupportProvider::name(const QString &url)
{
    const QUrl parsed(url);
    if (!parsed.isValid()
        || (parsed.scheme() != QStringLiteral("https")
            && parsed.scheme() != QStringLiteral("http")))
        return QString();
    const QString host = parsed.host().toLower();
    if (host.isEmpty())
        return QString();
    const QString path = trimTrailingSlashes(parsed.path().toLower());

    for (const auto &rule : SUPPORT_PROVIDERS) {
        if (hostMatchesDomain(host, QString::fromLatin1(rule.domain))
            && isAccountUrl(host, path, rule)) {
            return QString::fromUtf8(rule.name);
        }
    }
    return QString();
}

bool SupportProvider::isSupportProvider(const QString &url)
{
    return !name(url).isEmpty();
}
