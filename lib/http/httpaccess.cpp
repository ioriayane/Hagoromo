#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include "httpaccess.h"

#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

#include <QDateTime>
#define LOG_DATETIME QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")

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
    qDebug().noquote() << LOG_DATETIME << "HttpAccess::Private(" << this << ") Private()";
}

HttpAccess::Private::~Private()
{
    qDebug().noquote() << LOG_DATETIME << "HttpAccess::Private(" << this << ") ~Private()";
}

bool HttpAccess::Private::process(HttpReply *reply)
{
    qDebug().noquote() << LOG_DATETIME << "HttpAccess::Private(" << this << ") process()";
    if (reply == nullptr)
        return false;

    QUrl uri = reply->request().url();
    httplib::Client cli(uri.toString(QUrl::RemovePath | QUrl::RemoveQuery).toStdString());

    qDebug().noquote() << LOG_DATETIME << "  uri"
                       << uri.toString(QUrl::RemovePath | QUrl::RemoveQuery);
    qDebug().noquote() << LOG_DATETIME << "  uri.path()" << uri.path();
    qDebug().noquote() << LOG_DATETIME << "  ContentType"
                       << reply->request().header(QNetworkRequest::ContentTypeHeader).toString();
    qDebug().noquote() << LOG_DATETIME << "  size()" << reply->sendData().size();

    httplib::Headers headers;
    for (const auto &header : reply->request().rawHeaderList()) {
        qDebug().noquote() << LOG_DATETIME << "  header:" << header
                           << reply->request().rawHeader(header);
        headers.emplace(header.toStdString(), reply->request().rawHeader(header).toStdString());
    }
    QUrlQuery url_query(reply->request().url());
    httplib::Params params;
    for (const auto &query : url_query.queryItems()) {
        qDebug().noquote() << LOG_DATETIME << "  query:" << query.first << query.second;
        params.emplace(query.first.toStdString(), query.second.toStdString());
    }

    bool result = false;
    httplib::Result res;

    if (reply->operation() == HttpReply::GetOperation) {
        res = cli.Get(uri.path().toStdString(), params, headers, [=](uint64_t len, uint64_t total) {
            qDebug().noquote()
                    << LOG_DATETIME
                    << QString("%1 / %2 (%3%)").arg(len).arg(total).arg((int)(len * 100 / total));
            emit reply->downloadProgress(len, total);
            return true;
        });
    } else if (reply->operation() == HttpReply::PostOperation) {
        res = cli.Post(
                uri.path().toStdString(), headers, reply->sendData().size(),
                [=](size_t offset, size_t length, httplib::DataSink &sink) {
                    length = 1024;
                    if (reply->sendData().length() < static_cast<int>(offset + length)) {
                        length = reply->sendData().length() - offset;
                    }
                    if (length > 0) {
                        qDebug().noquote() << LOG_DATETIME << "  posting..." << offset << length
                                           << "=" << (offset + length);
                        sink.write(reply->sendData().mid(offset, length).data(), length);
                        emit reply->uploadProgress(offset, reply->sendData().size());
                    } else {
                        qDebug().noquote() << LOG_DATETIME << "  posting..." << offset << length
                                           << "=" << (offset + length) << "done";
                        sink.done();
                    }
                    return true;
                },
                reply->request()
                        .header(QNetworkRequest::ContentTypeHeader)
                        .toString()
                        .toStdString());
    } else {
        res = httplib::Result { nullptr, httplib::Error::Unknown, std::move(headers) };
    }

    qDebug().noquote() << LOG_DATETIME << "response";
    if (res) {
        qDebug().noquote() << LOG_DATETIME << "  status" << res->status;
        for (const auto &header : res->headers) {
            qDebug().noquote() << LOG_DATETIME
                               << "  header:" << QString::fromStdString(header.first)
                               << QString::fromStdString(header.second);
            reply->setRawHeader(QByteArray::fromStdString(header.first),
                                QByteArray::fromStdString(header.second));
        }
        reply->setRecvData(QByteArray::fromStdString(res->body));
        if (res->status == 200) {
            reply->setError(HttpReply::Success);
            result = true;
        } else {
            reply->setError(HttpReply::Unknown);
        }
    } else {
        auto err = res.error();
        qDebug().noquote() << LOG_DATETIME
                           << "HTTP error: " << QString::fromStdString(httplib::to_string(err));
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
    qDebug().noquote() << LOG_DATETIME << this << "HttpAccess()";
    connect(this, &QObject::destroyed, [this]() { delete d; });
}

HttpAccess::~HttpAccess()
{
    qDebug().noquote() << LOG_DATETIME << this << "~HttpAccess()";
}

void HttpAccess::process(HttpReply *reply)
{
    qDebug().noquote() << LOG_DATETIME << this << "process() in " << this->thread();

    if (reply == nullptr) {
        emit finished(nullptr);
        return;
    }
    qDebug().noquote() << LOG_DATETIME << "  reply" << reply;
    qDebug().noquote() << LOG_DATETIME << "  reply->operation()" << reply->operation();
    qDebug().noquote() << LOG_DATETIME << "  reply->request()->url().toString()"
                       << reply->request().url().toString();

    bool result = d->process(reply);

    emit finished(reply);
    qDebug().noquote() << LOG_DATETIME << this << "process() emit finished" << result;
}
