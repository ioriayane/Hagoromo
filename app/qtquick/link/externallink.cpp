#include "externallink.h"
#include "tools/opengraphprotocol.h"

#include <QDebug>

ExternalLink::ExternalLink(QObject *parent)
    : QObject { parent }, m_thumbLocal(nullptr), m_running(false), m_valid(false)
{
}

ExternalLink::~ExternalLink()
{
    if (m_thumbLocal != nullptr) {
        delete m_thumbLocal;
    }
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
    setThumb(QString());
    // 表示のキャッシュ対策
    if (m_thumbLocal != nullptr) {
        delete m_thumbLocal;
    }
    m_thumbLocal = new QTemporaryFile();
    m_thumbLocal->open();

    OpenGraphProtocol *open_graph = new OpenGraphProtocol(this);
    connect(open_graph, &OpenGraphProtocol::finished, [=](bool success) {
        bool do_download = false;
        qDebug().noquote() << "uri:" << open_graph->uri();
        qDebug().noquote() << "title:" << open_graph->title();
        qDebug().noquote() << "thumb:" << open_graph->thumb();
        if (success && !open_graph->thumb().isEmpty()) {
            do_download = true;
            open_graph->downloadThumb(m_thumbLocal->fileName());
        }
        if (!do_download) {
            setUri(open_graph->uri());
            setTitle(open_graph->title());
            setDescription(open_graph->description());
            setThumb(QString());
            setValid(success);
            open_graph->deleteLater();
            setRunning(false);
        }
    });
    connect(open_graph, &OpenGraphProtocol::finishedDownload, [=](bool success) {
        qDebug().noquote() << "finishedDownload" << success;
        if (!success) {
            setThumb(QString());
        } else {
            setThumb(open_graph->thumb());
        }
        setUri(open_graph->uri());
        setTitle(open_graph->title());
        setDescription(open_graph->description());
        setValid(true);
        setRunning(false);
        open_graph->deleteLater();
    });
    open_graph->getData(uri);
}

void ExternalLink::clear()
{
    setValid(false);
    setUri(QString());
    setTitle(QString());
    setDescription(QString());
    setThumb(QString());
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
    if (m_thumbLocal == nullptr) {
        return QString();
    } else if (valid() && !thumb().isEmpty()) {
        return QUrl::fromLocalFile(m_thumbLocal->fileName()).toString();
    } else {
        return QString();
    }
}

void ExternalLink::setThumbLocal(const QString &newThumbLocal)
{
    if (m_thumbLocal == nullptr)
        return;
    if (m_thumbLocal->fileName() == newThumbLocal)
        return;
    m_thumbLocal->setFileName(newThumbLocal);
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

    emit thumbLocalChanged();
    emit validChanged();
}
