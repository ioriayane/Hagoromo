#include "systemtool.h"
#include "qregularexpression.h"
#include "tools/qstringex.h"
#include "common.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QDebug>
#include <QImage>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>
#include <QFont>
#include <QFontDatabase>
#include <QQuickItem>
#include <QPainter>

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
    QString folder = Common::appTempFolder();

    QString new_path = QString("%1/%2.jpg").arg(folder, QUuid::createUuid().toString(QUuid::Id128));

    qDebug() << "clipImage" << url.toLocalFile();
    qDebug() << "dest" << new_path;

    QImage src(url.toLocalFile());
    QImage dest(src.size(), QImage::Format_RGB32);
    dest.fill(QColor(Qt::white));
    QPainter p_dest(&dest);
    p_dest.drawImage(0, 0, src, x, y, width, height);
    dest.save(new_path);

    return QUrl::fromLocalFile(new_path);
}

void SystemTool::updateFont(const QString &family)
{
    QFontDatabase db;
    if (db.families().contains(family)) {
        qDebug().noquote() << "Update font : " << QGuiApplication::font().family() << " -> "
                           << family;
        QGuiApplication::setFont(QFont(family));
        QObject *root = findRootType(this->parent());
        if (root != nullptr) {
            updateFontOfChildType(root, QFont(family));
        }
    }
}

QString SystemTool::defaultFontFamily()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("Yu Gothic UI");
#elif defined(Q_OS_OSX)
    return QStringLiteral("Hiragino Sans");
#else
    return QStringLiteral("Noto Sans CJK JP");
#endif
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

QObject *SystemTool::findRootType(QObject *object)
{
    QObject *parent = object->parent();
    QObject *ret = nullptr;
    while (parent) {
        // Popup の子供はitem->window()==null だけど辿ると見える
        qDebug().noquote().nospace() << parent->metaObject()->className();
        if (parent->parent() == nullptr) {
            return parent;
        }
        parent = parent->parent();
    }
    return ret;
}

void SystemTool::updateFontOfChildType(QObject *object, const QFont &font)
{
    for (auto *child : object->children()) {
        // qDebug().noquote().nospace() << "> " << child->metaObject()->className();
        updateFontProperty(child, font);
        updateFontOfChildType(child, font);
    }

    if (object->property("delegate").isValid() && object->property("model").isValid()
        && object->property("count").isValid()) {
        // ListViewやRepeaterのようなタイプはchildrenにデータがいないので
        int count = 0;
        QString method_name;
        const QMetaObject *meta = object->metaObject();
        for (int i = 0; i < meta->methodCount(); i++) {
            if (meta->method(i).returnType() != QMetaType::type("QQuickItem *")) {
            } else if (meta->method(i).parameterCount() != 1) {
            } else if (meta->method(i).parameterType(0) != QMetaType::type("int")) {
            } else if (meta->method(i).name() == "itemAt") {
                method_name = "itemAt";
                break;
            } else if (meta->method(i).name() == "itemAtIndex") {
                method_name = "itemAtIndex";
                break;
            }
        }
        count = object->property("count").toInt();
        if (!method_name.isEmpty() && count > 0) {
            for (int i = 0; i < count; i++) {
                QQuickItem *item = nullptr;
                if (QMetaObject::invokeMethod(object, method_name.toLocal8Bit().constData(),
                                              Q_RETURN_ARG(QQuickItem *, item), Q_ARG(int, i))) {
                    if (item != nullptr) {
                        // qDebug().noquote().nospace() << "> " << item->metaObject()->className();
                        updateFontProperty(item, font);
                        updateFontOfChildType(item, font);
                    }
                }
            }
        }
    }
}

void SystemTool::updateFontProperty(QObject *item, const QFont &font)
{
    QVariant p = item->property("font");
    if (p.isValid() && p.canConvert<QFont>()) {
        QFont f = p.value<QFont>();
        qDebug().noquote().nospace() << "#" << item->metaObject()->className() << " " << p.isValid()
                                     << " " << p.typeName() << " " << f.family();
        f.setFamily(font.family());
        item->setProperty("font", f);
    }
}
