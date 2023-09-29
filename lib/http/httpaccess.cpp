#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include "httpaccess.h"

#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

class HttpAccess::Private
{
public:
    Private(HttpAccess *parent);
    ~Private();

    bool process(HttpReply *reply);

    HttpReply::Error errorFrom(httplib::Error error);

private:
    HttpAccess *q;
};

HttpAccess::Private::Private(HttpAccess *parent) : q(parent)
{
    qDebug().noquote() << "HttpAccess::Private(" << this << ") Private()";
}

HttpAccess::Private::~Private()
{
    qDebug().noquote() << "HttpAccess::Private(" << this << ") ~Private()";
}

bool HttpAccess::Private::process(HttpReply *reply)
{
    qDebug().noquote() << "HttpAccess::Private(" << this << ") post()";
    if (reply == nullptr)
        return false;

    QUrl uri = reply->request().url();
    httplib::Client cli(uri.toString(QUrl::RemovePath | QUrl::RemoveQuery).toStdString());

    qDebug().noquote() << "  " << uri.toString(QUrl::RemovePath | QUrl::RemoveQuery);
    qDebug().noquote() << "  uri.path()" << uri.path();
    qDebug().noquote() << "  ContentType"
                       << reply->request().header(QNetworkRequest::ContentTypeHeader).toString();
    qDebug().noquote() << "  size()" << reply->sendData().size();

    httplib::Headers headers;
    for (const auto &header : reply->request().rawHeaderList()) {
        qDebug() << "  header:" << header << reply->request().rawHeader(header);
        headers.emplace(header.toStdString(), reply->request().rawHeader(header).toStdString());
    }
    QUrlQuery url_query(reply->request().url());
    httplib::Params params;
    for (const auto &query : url_query.queryItems()) {
        qDebug() << "  query:" << query.first << query.second;
        params.emplace(query.first.toStdString(), query.second.toStdString());
    }

    bool result = false;
    httplib::Result res;

    if (reply->operation() == HttpReply::GetOperation) {
        res = cli.Get(uri.path().toStdString(), params, headers, [=](uint64_t len, uint64_t total) {
            qDebug().noquote()
                    << QString("%1 / %2 (%3\%)").arg(len).arg(total).arg((int)(len * 100 / total));
            emit reply->downloadProgress(len, total);
            return true;
        });
    } else if (reply->operation() == HttpReply::PostOperation) {
        res = cli.Post(
                uri.path().toStdString(), headers, reply->sendData().size(),
                [=](size_t offset, size_t length, httplib::DataSink &sink) {
                    qDebug().noquote() << "  posting..." << offset << length;
                    sink.write(reply->sendData().mid(offset, length).data(), length);
                    emit reply->uploadProgress(offset, reply->sendData().size());
                    return true;
                },
                reply->request()
                        .header(QNetworkRequest::ContentTypeHeader)
                        .toString()
                        .toStdString());
    } else {
        res = httplib::Result { nullptr, httplib::Error::Unknown, std::move(headers) };
    }

    qDebug().noquote() << "response";
    if (res) {
        qDebug().noquote() << "  status" << res->status;
        for (const auto &header : res->headers) {
            qDebug().noquote() << "  header:" << QString::fromStdString(header.first)
                               << QString::fromStdString(header.second);
            reply->setRawHeader(QByteArray::fromStdString(header.first),
                                QByteArray::fromStdString(header.second));
        }
        if (res->status == 200) {
            reply->setBody(QString::fromStdString(res->body));
            reply->setError(HttpReply::Success);
            result = true;
        } else {
            reply->setError(HttpReply::Unknown);
        }
    } else {
        auto err = res.error();
        qDebug().noquote() << "HTTP error: " << QString::fromStdString(httplib::to_string(err));
        reply->setError(errorFrom(err));
    }

    return result;
}

HttpReply::Error HttpAccess::Private::errorFrom(httplib::Error error)
{
    switch (error) {
    case httplib::Error::Success:
        return HttpReply::Error::Success;
    case httplib::Error::Connection:
        return HttpReply::Error::Connection;
    case httplib::Error::BindIPAddress:
        return HttpReply::Error::BindIPAddress;
    case httplib::Error::Read:
        return HttpReply::Error::Read;
    case httplib::Error::Write:
        return HttpReply::Error::Write;
    case httplib::Error::ExceedRedirectCount:
        return HttpReply::Error::ExceedRedirectCount;
    case httplib::Error::Canceled:
        return HttpReply::Error::Canceled;
    case httplib::Error::SSLConnection:
        return HttpReply::Error::SSLConnection;
    case httplib::Error::SSLLoadingCerts:
        return HttpReply::Error::SSLLoadingCerts;
    case httplib::Error::SSLServerVerification:
        return HttpReply::Error::SSLServerVerification;
    case httplib::Error::UnsupportedMultipartBoundaryChars:
        return HttpReply::Error::UnsupportedMultipartBoundaryChars;
    case httplib::Error::Compression:
        return HttpReply::Error::Compression;
    case httplib::Error::ConnectionTimeout:
        return HttpReply::Error::ConnectionTimeout;
    case httplib::Error::ProxyConnection:
        return HttpReply::Error::ProxyConnection;
    default:
        return HttpReply::Error::Unknown;
    }
}

HttpAccess::HttpAccess(QObject *parent) : QObject { parent }, d(new Private(this))
{
    qDebug().noquote() << this << "HttpAccess()";
    connect(this, &QObject::destroyed, [this]() { delete d; });
}

HttpAccess::~HttpAccess()
{
    qDebug().noquote() << this << "~HttpAccess()";
}

void HttpAccess::process(HttpReply *reply)
{
    qDebug().noquote() << this << "process()";

    if (reply == nullptr) {
        emit finished(false);
        return;
    }
    qDebug().noquote() << "  reply->operation()" << reply->operation();
    qDebug().noquote() << "  reply->request()->url().toString()"
                       << reply->request().url().toString();

    bool result = d->process(reply);

    emit reply->finished(result);
    emit finished(result);
    qDebug().noquote() << this << "process() emit finished" << result;
}
