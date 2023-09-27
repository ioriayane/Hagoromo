#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include "httpaccess.h"

#include <QUrl>
#include <QDebug>

HttpAccess::HttpAccess(QObject *parent) : QObject { parent }
{
    qDebug().noquote() << this << "HttpAccess()";
}

HttpAccess::~HttpAccess()
{
    qDebug().noquote() << this << "~HttpAccess()";
}

bool HttpAccess::get(HttpReply *reply)
{
    qDebug().noquote() << this << "get()";

    QUrl uri = reply->request()->url();
    httplib::Client cli(uri.toString(QUrl::RemovePath).toStdString());

    qDebug().noquote() << uri.toString(QUrl::RemovePath);
    qDebug().noquote() << uri.host() << uri.port();
    qDebug().noquote() << uri.path();

    bool result = false;
    if (auto res = cli.Get(uri.path().toStdString())) {
        if (res->status == 200) {
            reply->setBody(QString::fromStdString(res->body));
            result = true;
        }
    } else {
        auto err = res.error();
        qDebug().noquote() << "HTTP error: " << QString::fromStdString(httplib::to_string(err));
    }

    return result;
}

bool HttpAccess::post(HttpReply *reply)
{
    qDebug().noquote() << this << "post()";

    QUrl uri = reply->request()->url();
    httplib::Client cli(uri.toString(QUrl::RemovePath).toStdString());

    qDebug().noquote() << "  " << uri.toString(QUrl::RemovePath);
    qDebug().noquote() << "  uri.path()" << uri.path();
    qDebug().noquote() << "  ContentType"
                       << reply->request()->header(QNetworkRequest::ContentTypeHeader).toString();
    qDebug().noquote() << "  size()" << reply->sendData().size();

    bool result = false;
    auto res = cli.Post(
            uri.path().toStdString(), reply->sendData().size(),
            [=](size_t offset, size_t length, httplib::DataSink &sink) {
                qDebug().noquote() << "  posting..." << offset << length;
                sink.write(reply->sendData().mid(offset, length).data(), length);
                return true;
            },
            reply->request()->header(QNetworkRequest::ContentTypeHeader).toString().toStdString());
    if (res) {
        qDebug().noquote() << "  status" << res->status;
        if (res->status == 200) {
            reply->setBody(QString::fromStdString(res->body));
            result = true;
        }
    } else {
        auto err = res.error();
        qDebug().noquote() << "HTTP error: " << QString::fromStdString(httplib::to_string(err));
    }

    return result;
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
                       << reply->request()->url().toString();

    bool result = false;

    if (reply->operation() == HttpReply::GetOperation) {
        result = get(reply);
    } else if (reply->operation() == HttpReply::PostOperation) {
        result = post(reply);
    }

    emit reply->finished(result);
    emit finished(result);
    qDebug().noquote() << this << "process() emit finished" << result;
}
