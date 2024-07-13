#include "languagelistmodel.h"
#include <QLocale>
#include <QDebug>

LanguageListModel::LanguageListModel(QObject *parent) : QAbstractListModel(parent)
{
    m_languageList << LanguageItem(tr("Afar"), "aa");
    m_languageList << LanguageItem(tr("Abkhazian"), "ab");
    m_languageList << LanguageItem(tr("Avestan"), "ae");
    m_languageList << LanguageItem(tr("Afrikaans"), "af");
    m_languageList << LanguageItem(tr("Akan"), "ak");
    m_languageList << LanguageItem(tr("Amharic"), "am");
    m_languageList << LanguageItem(tr("Aragonese"), "an");
    m_languageList << LanguageItem(tr("Arabic"), "ar");
    m_languageList << LanguageItem(tr("Assamese"), "as");
    m_languageList << LanguageItem(tr("Avaric"), "av");
    m_languageList << LanguageItem(tr("Aymara"), "ay");
    m_languageList << LanguageItem(tr("Azerbaijani"), "az");
    m_languageList << LanguageItem(tr("Bashkir"), "ba");
    m_languageList << LanguageItem(tr("Belarusian"), "be");
    m_languageList << LanguageItem(tr("Bulgarian"), "bg");
    m_languageList << LanguageItem(tr("Bihari languages"), "bh");
    m_languageList << LanguageItem(tr("Bislama"), "bi");
    m_languageList << LanguageItem(tr("Bambara"), "bm");
    m_languageList << LanguageItem(tr("Bengali"), "bn");
    m_languageList << LanguageItem(tr("Tibetan"), "bo");
    m_languageList << LanguageItem(tr("Breton"), "br");
    m_languageList << LanguageItem(tr("Bosnian"), "bs");
    m_languageList << LanguageItem(tr("Catalan"), "ca");
    m_languageList << LanguageItem(tr("Chechen"), "ce");
    m_languageList << LanguageItem(tr("Chamorro"), "ch");
    m_languageList << LanguageItem(tr("Corsican"), "co");
    m_languageList << LanguageItem(tr("Cree"), "cr");
    m_languageList << LanguageItem(tr("Czech"), "cs");
    m_languageList << LanguageItem(tr("Old Church Slavonic"), "cu");
    m_languageList << LanguageItem(tr("Chuvash"), "cv");
    m_languageList << LanguageItem(tr("Welsh"), "cy");
    m_languageList << LanguageItem(tr("Danish"), "da");
    m_languageList << LanguageItem(tr("German"), "de");
    m_languageList << LanguageItem(tr("Divehi"), "dv");
    m_languageList << LanguageItem(tr("Dzongkha"), "dz");
    m_languageList << LanguageItem(tr("Ewe"), "ee");
    m_languageList << LanguageItem(tr("Greek"), "el");
    m_languageList << LanguageItem(tr("English"), "en");
    m_languageList << LanguageItem(tr("Esperanto"), "eo");
    m_languageList << LanguageItem(tr("Spanish"), "es");
    m_languageList << LanguageItem(tr("Estonian"), "et");
    m_languageList << LanguageItem(tr("Basque"), "eu");
    m_languageList << LanguageItem(tr("Persian"), "fa");
    m_languageList << LanguageItem(tr("Fulah"), "ff");
    m_languageList << LanguageItem(tr("Finnish"), "fi");
    m_languageList << LanguageItem(tr("Fijian"), "fj");
    m_languageList << LanguageItem(tr("Faroese"), "fo");
    m_languageList << LanguageItem(tr("French"), "fr");
    m_languageList << LanguageItem(tr("Western Frisian"), "fy");
    m_languageList << LanguageItem(tr("Irish"), "ga");
    m_languageList << LanguageItem(tr("Scottish Gaelic"), "gd");
    m_languageList << LanguageItem(tr("Galician"), "gl");
    m_languageList << LanguageItem(tr("Guarani"), "gn");
    m_languageList << LanguageItem(tr("Gujarati"), "gu");
    m_languageList << LanguageItem(tr("Manx"), "gv");
    m_languageList << LanguageItem(tr("Hausa"), "ha");
    m_languageList << LanguageItem(tr("Hebrew"), "he");
    m_languageList << LanguageItem(tr("Hindi"), "hi");
    m_languageList << LanguageItem(tr("Hiri Motu"), "ho");
    m_languageList << LanguageItem(tr("Croatian"), "hr");
    m_languageList << LanguageItem(tr("Haitian"), "ht");
    m_languageList << LanguageItem(tr("Hungarian"), "hu");
    m_languageList << LanguageItem(tr("Armenian"), "hy");
    m_languageList << LanguageItem(tr("Herero"), "hz");
    m_languageList << LanguageItem(tr("Interlingua (International Auxiliary Language Association)"),
                                   "ia");
    m_languageList << LanguageItem(tr("Indonesian"), "id");
    m_languageList << LanguageItem(tr("Interlingue"), "ie");
    m_languageList << LanguageItem(tr("Igbo"), "ig");
    m_languageList << LanguageItem(tr("Sichuan Yi"), "ii");
    m_languageList << LanguageItem(tr("Inupiaq"), "ik");
    m_languageList << LanguageItem(tr("Ido"), "io");
    m_languageList << LanguageItem(tr("Icelandic"), "is");
    m_languageList << LanguageItem(tr("Italian"), "it");
    m_languageList << LanguageItem(tr("Inuktitut"), "iu");
    m_languageList << LanguageItem(tr("Japanese"), "ja");
    m_languageList << LanguageItem(tr("Javanese"), "jv");
    m_languageList << LanguageItem(tr("Georgian"), "ka");
    m_languageList << LanguageItem(tr("Kongo"), "kg");
    m_languageList << LanguageItem(tr("Kikuyu"), "ki");
    m_languageList << LanguageItem(tr("Kuanyama"), "kj");
    m_languageList << LanguageItem(tr("Kazakh"), "kk");
    m_languageList << LanguageItem(tr("Greenlandic"), "kl");
    m_languageList << LanguageItem(tr("Central Khmer"), "km");
    m_languageList << LanguageItem(tr("Kannada"), "kn");
    m_languageList << LanguageItem(tr("Korean"), "ko");
    m_languageList << LanguageItem(tr("Kanuri"), "kr");
    m_languageList << LanguageItem(tr("Kashmiri"), "ks");
    m_languageList << LanguageItem(tr("Kurdish"), "ku");
    m_languageList << LanguageItem(tr("Komi"), "kv");
    m_languageList << LanguageItem(tr("Cornish"), "kw");
    m_languageList << LanguageItem(tr("Kirghiz"), "ky");
    m_languageList << LanguageItem(tr("Latin"), "la");
    m_languageList << LanguageItem(tr("Luxembourgish"), "lb");
    m_languageList << LanguageItem(tr("Ganda"), "lg");
    m_languageList << LanguageItem(tr("Limburgan"), "li");
    m_languageList << LanguageItem(tr("Lingala"), "ln");
    m_languageList << LanguageItem(tr("Lao"), "lo");
    m_languageList << LanguageItem(tr("Lithuanian"), "lt");
    m_languageList << LanguageItem(tr("Luba-Katanga"), "lu");
    m_languageList << LanguageItem(tr("Latvian"), "lv");
    m_languageList << LanguageItem(tr("Malagasy"), "mg");
    m_languageList << LanguageItem(tr("Marshallese"), "mh");
    m_languageList << LanguageItem(tr("Maori"), "mi");
    m_languageList << LanguageItem(tr("Macedonian"), "mk");
    m_languageList << LanguageItem(tr("Malayalam"), "ml");
    m_languageList << LanguageItem(tr("Mongolian"), "mn");
    m_languageList << LanguageItem(tr("Marathi"), "mr");
    m_languageList << LanguageItem(tr("Malay"), "ms");
    m_languageList << LanguageItem(tr("Maltese"), "mt");
    m_languageList << LanguageItem(tr("Burmese"), "my");
    m_languageList << LanguageItem(tr("Nauru"), "na");
    m_languageList << LanguageItem(tr("Norwegian Bokmål"), "nb");
    m_languageList << LanguageItem(tr("North Ndebele"), "nd");
    m_languageList << LanguageItem(tr("Nepali"), "ne");
    m_languageList << LanguageItem(tr("Ndonga"), "ng");
    m_languageList << LanguageItem(tr("Dutch"), "nl");
    m_languageList << LanguageItem(tr("Norwegian Nynorsk"), "nn");
    m_languageList << LanguageItem(tr("Norwegian"), "no");
    m_languageList << LanguageItem(tr("South Ndebele"), "nr");
    m_languageList << LanguageItem(tr("Navajo"), "nv");
    m_languageList << LanguageItem(tr("Chichewa"), "ny");
    m_languageList << LanguageItem(tr("Occitan"), "oc");
    m_languageList << LanguageItem(tr("Ojibwa"), "oj");
    m_languageList << LanguageItem(tr("Oromo"), "om");
    m_languageList << LanguageItem(tr("Oriya"), "or");
    m_languageList << LanguageItem(tr("Ossetian"), "os");
    m_languageList << LanguageItem(tr("Punjabi"), "pa");
    m_languageList << LanguageItem(tr("Pali"), "pi");
    m_languageList << LanguageItem(tr("Polish"), "pl");
    m_languageList << LanguageItem(tr("Pashto"), "ps");
    m_languageList << LanguageItem(tr("Portuguese"), "pt");
    m_languageList << LanguageItem(tr("Quechua"), "qu");
    m_languageList << LanguageItem(tr("Romansh"), "rm");
    m_languageList << LanguageItem(tr("Rundi"), "rn");
    m_languageList << LanguageItem(tr("Romanian"), "ro");
    m_languageList << LanguageItem(tr("Russian"), "ru");
    m_languageList << LanguageItem(tr("Kinyarwanda"), "rw");
    m_languageList << LanguageItem(tr("Sanskrit"), "sa");
    m_languageList << LanguageItem(tr("Sardinian"), "sc");
    m_languageList << LanguageItem(tr("Sindhi"), "sd");
    m_languageList << LanguageItem(tr("Northern Sami"), "se");
    m_languageList << LanguageItem(tr("Sango"), "sg");
    m_languageList << LanguageItem(tr("Sinhala"), "si");
    m_languageList << LanguageItem(tr("Slovak"), "sk");
    m_languageList << LanguageItem(tr("Slovenian"), "sl");
    m_languageList << LanguageItem(tr("Samoan"), "sm");
    m_languageList << LanguageItem(tr("Shona"), "sn");
    m_languageList << LanguageItem(tr("Somali"), "so");
    m_languageList << LanguageItem(tr("Albanian"), "sq");
    m_languageList << LanguageItem(tr("Serbian"), "sr");
    m_languageList << LanguageItem(tr("Swati"), "ss");
    m_languageList << LanguageItem(tr("Southern Sotho"), "st");
    m_languageList << LanguageItem(tr("Sundanese"), "su");
    m_languageList << LanguageItem(tr("Swedish"), "sv");
    m_languageList << LanguageItem(tr("Swahili"), "sw");
    m_languageList << LanguageItem(tr("Tamil"), "ta");
    m_languageList << LanguageItem(tr("Telugu"), "te");
    m_languageList << LanguageItem(tr("Tajik"), "tg");
    m_languageList << LanguageItem(tr("Thai"), "th");
    m_languageList << LanguageItem(tr("Tigrinya"), "ti");
    m_languageList << LanguageItem(tr("Turkmen"), "tk");
    m_languageList << LanguageItem(tr("Tagalog"), "tl");
    m_languageList << LanguageItem(tr("Tswana"), "tn");
    m_languageList << LanguageItem(tr("Tonga (Tonga Islands)"), "to");
    m_languageList << LanguageItem(tr("Turkish"), "tr");
    m_languageList << LanguageItem(tr("Tsonga"), "ts");
    m_languageList << LanguageItem(tr("Tatar"), "tt");
    m_languageList << LanguageItem(tr("Twi"), "tw");
    m_languageList << LanguageItem(tr("Tahitian"), "ty");
    m_languageList << LanguageItem(tr("Uighur"), "ug");
    m_languageList << LanguageItem(tr("Ukrainian"), "uk");
    m_languageList << LanguageItem(tr("Urdu"), "ur");
    m_languageList << LanguageItem(tr("Uzbek"), "uz");
    m_languageList << LanguageItem(tr("Venda"), "ve");
    m_languageList << LanguageItem(tr("Vietnamese"), "vi");
    m_languageList << LanguageItem(tr("Volapük"), "vo");
    m_languageList << LanguageItem(tr("Walloon"), "wa");
    m_languageList << LanguageItem(tr("Wolof"), "wo");
    m_languageList << LanguageItem(tr("Xhosa"), "xh");
    m_languageList << LanguageItem(tr("Yiddish"), "yi");
    m_languageList << LanguageItem(tr("Yoruba"), "yo");
    m_languageList << LanguageItem(tr("Zhuang"), "za");
    m_languageList << LanguageItem(tr("Chinese"), "zh");
    m_languageList << LanguageItem(tr("Zulu"), "zu");
    m_languageList << LanguageItem(tr("Angika"), "anp");

    for (int i = 0; i < m_languageList.count() - 1; i++) {
        for (int j = i + 1; j < m_languageList.count(); j++) {
            if (m_languageList[i].name.compare(m_languageList[j].name) > 0) {
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
                m_languageList.swap(i, j);
#else
                m_languageList.swapItemsAt(i, j);
#endif
            }
        }
    }

    // システム設定の言語を先頭に持ってくる
    for (int i = 0; i < m_languageList.count(); i++) {
        if (m_languageList.at(i).iso639_1 == QLocale::system().bcp47Name()) {
            m_languageList.move(i, 0);
        }
    }

    m_iso639toIndexHash.clear();
    for (int i = 0; i < m_languageList.count(); i++) {
        m_iso639toIndexHash[m_languageList.at(i).iso639_1] = i;
    }
}

int LanguageListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_languageList.count();
}

QVariant LanguageListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<LanguageListModelRoles>(role));
}

QVariant LanguageListModel::item(int row, LanguageListModelRoles role) const
{
    if (row < 0 || row >= m_languageList.count())
        return QVariant();

    if (role == NameRole)
        return m_languageList.at(row).name;
    else if (role == CheckedRole)
        return m_languageList.at(row).checked;
    else if (role == EnabledRole)
        return m_languageList.at(row).enabled;

    return QVariant();
}

void LanguageListModel::update(int row, LanguageListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_languageList.count())
        return;

    if (role == CheckedRole) {
        m_languageList[row].checked = value.toBool();
        updateEnable();
    }
}

void LanguageListModel::setSelectedLanguages(const QStringList &langs)
{
    for (int i = 0; i < m_languageList.count(); i++) {
        m_languageList[i].checked = langs.contains(m_languageList.at(i).iso639_1);
    }
    updateEnable();
}

QStringList LanguageListModel::selectedLanguages() const
{
    QStringList list;
    for (const auto &item : qAsConst(m_languageList)) {
        if (item.checked) {
            list.append(item.iso639_1);
        }
    }
    return list;
}

QStringList LanguageListModel::convertLanguageNames(const QStringList &langs) const
{
    QStringList list;
    for (const auto &lang : langs) {
        if (m_iso639toIndexHash.contains(lang)) {
            list.append(m_languageList.at(m_iso639toIndexHash[lang]).name);
        }
    }
    return list;
}

QHash<int, QByteArray> LanguageListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[NameRole] = "name";
    roles[CheckedRole] = "checked";
    roles[EnabledRole] = "enabled";

    return roles;
}

void LanguageListModel::updateEnable()
{
    // 3個までしか選択できないようにする(Lexiconの仕様。上限を外部から指定できるようにするのが理想かもだけど）
    int count = 0;
    for (const auto &item : qAsConst(m_languageList)) {
        if (item.checked) {
            count++;
        }
    }
    bool new_enabled = (count < 3);
    for (int i = 0; i < m_languageList.count(); i++) {
        if (m_languageList.at(i).checked) {
            m_languageList[i].enabled = true;
        } else if (m_languageList.at(i).enabled != new_enabled) {
            m_languageList[i].enabled = new_enabled;
        }
        emit dataChanged(index(i), index(i));
    }
}
