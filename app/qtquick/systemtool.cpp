#include "systemtool.h"
#include "qregularexpression.h"
#include "tools/qstringex.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QDebug>
#include <QImage>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>

SystemTool::SystemTool(QObject *parent) : QObject { parent }
{
    m_rxUrl = QRegularExpression(QString("%1").arg(REG_EXP_URL));
}

void SystemTool::copyToClipboard(const QString &text) const
{
    QGuiApplication::clipboard()->setText(text);
}

int SystemTool::countText(const QString &text) const
{
    return QStringEx(text).length();
}

QUrl SystemTool::clipImage(const QUrl &url, const int x, const int y, const int width,
                           const int height) const
{
    QString folder =
            QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation),
                                 QCoreApplication::applicationName());
    QDir dir(folder);
    dir.mkpath(folder);

    QString new_path = QString("%1/%2.jpg").arg(folder, QUuid::createUuid().toString(QUuid::Id128));

    qDebug() << "clipImage" << url.toLocalFile();
    qDebug() << "dest" << new_path;

    QImage img(url.toLocalFile());
    img.copy(x, y, width, height).save(new_path);

    return QUrl::fromLocalFile(new_path);
}

QString SystemTool::applicationVersion() const
{
    return QCoreApplication::applicationVersion();
}

QString SystemTool::qtVersion() const
{
    return QT_VERSION_STR;
}

QString SystemTool::markupText(const QString &text) const
{
    if (text.isEmpty())
        return text;

    QString temp;
    QRegularExpressionMatch match = m_rxUrl.match(text);
    if (match.capturedTexts().isEmpty()) {
        temp = text;
    } else {
        int pos;
        int start_pos = 0;
        while ((pos = match.capturedStart()) != -1) {
            temp += text.midRef(start_pos, pos - start_pos);
            temp += QString("<a href=\"%1\">%1</a>").arg(match.captured());

            start_pos = pos + match.capturedLength();
            match = m_rxUrl.match(text, start_pos);
        }
        if (start_pos < text.length() - 1) {
            temp += text.midRef(start_pos, text.length() - start_pos);
        }
    }
    temp.replace("\r\n", "<br/>");
    temp.replace("\n", "<br/>");

    return temp;
}
