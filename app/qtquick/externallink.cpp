#include "externallink.h"
#include "tools/opengraphprotocol.h"

#include <QPointer>

ExternalLink::ExternalLink(QObject *parent) : QObject { parent }, m_running(false), m_valid(false)
{
    m_thumbLocal.open();
}

void ExternalLink::getExternalLink(const QString &uri)
{
    if (running())
        return;
    setRunning(true);

    setValid(false);
    setUri(QString());
    setTitle(QString());
    setDescription(QString());

    QPointer<ExternalLink> aliving(this);

    OpenGraphProtocol *open_graph = new OpenGraphProtocol();
    connect(open_graph, &OpenGraphProtocol::finished, [=](bool success) {
        bool do_download = false;
        if (aliving) {
            qDebug() << open_graph->uri() << open_graph->title() << open_graph->thumb();
            if (success && !open_graph->thumb().isEmpty()) {
                do_download = true;
                open_graph->downloadThumb(m_thumbLocal.fileName());
            }
        }
        if (!do_download) {
            open_graph->deleteLater();
            setRunning(false);
        }
    });
    connect(open_graph, &OpenGraphProtocol::finishedDownload, [=](bool success) {
        if (aliving) {
            if (success) {
                setUri(open_graph->uri());
                setTitle(open_graph->title());
                setDescription(open_graph->description());
                emit thumbLocalChanged();
                setValid(true);
            }
            setRunning(false);
        }
        open_graph->deleteLater();
    });
    open_graph->getData(uri);
}

QString ExternalLink::uri() const
{
    return m_uri;
}

void ExternalLink::setUri(const QString &newUri)
{
    if (m_uri == newUri)
        return;
    m_uri = newUri;
    emit uriChanged();
}

QString ExternalLink::title() const
{
    return m_title;
}

void ExternalLink::setTitle(const QString &newTitle)
{
    if (m_title == newTitle)
        return;
    m_title = newTitle;
    emit titleChanged();
}

QString ExternalLink::description() const
{
    return m_description;
}

void ExternalLink::setDescription(const QString &newDescription)
{
    if (m_description == newDescription)
        return;
    m_description = newDescription;
    emit descriptionChanged();
}

QString ExternalLink::thumb() const
{
    return m_thumb;
}

void ExternalLink::setThumb(const QString &newThumb)
{
    if (m_thumb == newThumb)
        return;
    m_thumb = newThumb;
    emit thumbChanged();
}

QString ExternalLink::thumbLocal() const
{
    return QUrl(m_thumbLocal.fileName()).toString();
}

void ExternalLink::setThumbLocal(const QString &newThumbLocal)
{
    if (m_thumbLocal.fileName() == newThumbLocal)
        return;
    m_thumbLocal.setFileName(newThumbLocal);
    emit thumbLocalChanged();
}

bool ExternalLink::running() const
{
    return m_running;
}

void ExternalLink::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

bool ExternalLink::valid() const
{
    return m_valid;
}

void ExternalLink::setValid(bool newValid)
{
    if (m_valid == newValid)
        return;
    m_valid = newValid;
    emit validChanged();
}
