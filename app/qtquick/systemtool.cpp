#include "systemtool.h"
#include "qregularexpression.h"
#include "tools/qstringex.h"

#include <QClipboard>
#include <QGuiApplication>

SystemTool::SystemTool(QObject *parent) : QObject { parent }
{
    m_rxUrl = QRegularExpression(QString("%1").arg(
            "http[s]?://"
            "(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F])|#(?:[a-zA-Z]|[0-9]"
            "|[$-_@.&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))*)+"));
}

void SystemTool::copyToClipboard(const QString &text) const
{
    QGuiApplication::clipboard()->setText(text);
}

int SystemTool::countText(const QString &text) const
{
    return QStringEx(text).length();
}

QString SystemTool::applicationVersion() const
{
    return QCoreApplication::applicationVersion();
}

QString SystemTool::qtVersion() const
{
    return QT_VERSION_STR;
}

QString SystemTool::markupText(const QString &text)
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
