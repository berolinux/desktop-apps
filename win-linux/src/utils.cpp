/*
 * (c) Copyright Ascensio System SIA 2010-2016
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "utils.h"
#include "defines.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QRegularExpression>
#include <QIcon>
#include <QSysInfo>
#include <QApplication>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QUrl>
#include <QJsonDocument>
#include <QProcess>

#include "applicationmanager.h"
#include "applicationmanager_events.h"

#ifdef _WIN32
#include "shlobj.h"
#else
#include <sys/stat.h>
#endif

#include <QDebug>

bool is_file_browser_supported = true;

QStringList * Utils::getInputFiles(const QStringList& inlist)
{
    QStringList * _ret_files_list = new QStringList;

    QStringListIterator i(inlist); i.next();
    while (i.hasNext()) {
        QString arg = i.next();

        if ( arg.startsWith("--new:") )
            _ret_files_list->append( arg );
        else {
            QFileInfo info( arg );
            if ( info.isFile() ) {
                _ret_files_list->append(info.absoluteFilePath());
            }
        }
    }

    return _ret_files_list;
}

QString Utils::lastPath(int t)
{
    GET_REGISTRY_USER(_reg_user)

    QString _path;
    if (t == LOCAL_PATH_OPEN)
        _path = _reg_user.value("openPath").value<QString>(); else
        _path = _reg_user.value("savePath").value<QString>();

    return _path.length() > 0 && QDir(_path).exists() ?
        _path : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

void Utils::keepLastPath(int t, const QString& p)
{
    GET_REGISTRY_USER(_reg_user)
    if (t == LOCAL_PATH_OPEN)
        _reg_user.setValue("openPath", p); else
        _reg_user.setValue("savePath", p);
}

bool Utils::makepath(const QString& p)
{
#ifdef __linux
    mode_t _mask = umask(0);
    (_mask & S_IRWXO) && umask(_mask & ~S_IRWXO);
#endif
    return QDir().mkpath(p);
}

QRect Utils::getScreenGeometry(const QPoint& leftTop)
{
//    int _scr_num = QApplication::desktop()->screenNumber(leftTop); - return the wrong number
//    return QApplication::desktop()->screenGeometry(_scr_num);

    auto pointToRect = [](const QPoint &p, const QRect &r) -> int {
        int dx = 0, dy = 0;
        if (p.x() < r.left()) dx = r.left() - p.x(); else
        if (p.x() > r.right()) dx = p.x() - r.right();

        if (p.y() < r.top()) dy = r.top() - p.y(); else
        if (p.y() > r.bottom()) dy = p.y() - r.bottom();

        return dx + dy;
    };

    int closestScreen = 0;
    int shortestDistance = pointToRect(leftTop, QApplication::desktop()->screenGeometry(0));

    for (int i = 0; ++i < QApplication::desktop()->screenCount(); ) {
        int thisDistance = pointToRect(leftTop, QApplication::desktop()->screenGeometry(i));
        if (thisDistance < shortestDistance) {
            shortestDistance = thisDistance;
            closestScreen = i;
        }
    }

    return QApplication::desktop()->screenGeometry(closestScreen);
}

QString Utils::systemLocationCode()
{
#define LOCATION_MAX_LEN 9
#ifdef _WIN32
    WCHAR _country_code[LOCATION_MAX_LEN]{0};
    // "no entry point for GetLocaleInfoEx" error on win_xp
//    if ( QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA ) {
//        if (!GetLocaleInfoEx(LOCALE_NAME_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME, _country_code, LOCATION_MAX_LEN))
//            return "unknown";
//    } else {
        if (!GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME, _country_code, LOCATION_MAX_LEN))
            return "unknown";
//    }

    return QString::fromWCharArray(_country_code);
#else
    return QLocale().name().split('_').at(1);
#endif
}

void Utils::openUrl(const QString& url)
{
#ifdef __linux
    QUrl _url(url);
    if ( _url.scheme() == "mailto" ) {
        system(QString("LD_LIBRARY_PATH='' xdg-email %1")                   // xdg-email filepath email
                            .arg(QString( _url.toEncoded() )).toUtf8());
    } else {
        system(QString("LD_LIBRARY_PATH='' xdg-open %1")                    // xdg-open workingpath path
                            .arg(QString( _url.toEncoded() )).toUtf8());
    }
#else
    QDesktopServices::openUrl(QUrl(url));
#endif
}

void Utils::openFileLocation(const QString& path)
{
#if defined(Q_OS_WIN)
    QStringList args{"/select,", QDir::toNativeSeparators(path)};
    QProcess::startDetached("explorer", args);
#else
    static QString is_browser_checked;
    if ( is_browser_checked.isEmpty() ) {
        is_browser_checked = "checked";

        auto _get_cmd_output = [](const QString& cmd, const QStringList& args, QString& error) {
            QProcess process;
            process.start(cmd, args);
            process.waitForFinished(-1);

            error = process.readAllStandardError();
            return process.readAllStandardOutput();
        };

        QString _error;
        QString _file_browser = _get_cmd_output("xdg-mime", QStringList{"query", "default", "inode/directory"}, _error);

        //    if ( _error.isEmpty() )
        {
            bool is_file_browser_nautilus =
                        _file_browser.contains(QRegularExpression("nautilus\\.desktop", QRegularExpression::CaseInsensitiveOption)) ||
                            _file_browser.contains(QRegularExpression("nautilus-folder-handler\\.desktop", QRegularExpression::CaseInsensitiveOption));

            if ( is_file_browser_nautilus ) {
                QString _version = _get_cmd_output("nautilus", QStringList{"--version"}, _error);

        //        if ( _error.isEmpty() )
                {
                    QRegularExpression _regex("nautilus\\s(\\d{1,3})\\.(\\d{1,3})(?:\\.(\\d{1,5}))?");
                    QRegularExpressionMatch match = _regex.match(_version);
                    if ( match.hasMatch() ) {
                        bool is_verion_supported = match.captured(1).toInt() > 3;
                        if ( !is_verion_supported && match.captured(1).toInt() == 3 ) {
                            is_verion_supported = match.captured(2).toInt() > 0;

                            if ( !is_verion_supported && !match.captured(3).isEmpty() )
                                is_verion_supported = !(match.captured(3).toInt() < 2);
                        }

                        is_file_browser_supported = is_verion_supported;
                    }
                }
            }
        }
    }


    QFileInfo fileInfo(path);
    is_file_browser_supported ?
        system(QString("nautilus \"" + fileInfo.absoluteFilePath() + "\"").toUtf8()) :
        system(QString("LD_LIBRARY_PATH='' xdg-open \"%1\"").arg(fileInfo.path()).toUtf8());
#endif
}

QString Utils::getPortalName(const QString& url)
{
    if ( !url.isEmpty() ) {
        QRegularExpressionMatch match = QRegularExpression(rePortalName).match(url);
        if (match.hasMatch()) {
            QString out = match.captured(1);
            return out.endsWith('/') ? out.remove(-1, 1) : out;
        }
    }

    return url;
}

QString Utils::encodeJson(const QJsonObject& obj)
{
    return Utils::encodeJson(
                QJsonDocument(obj).toJson(QJsonDocument::Compact) );
}

QString Utils::encodeJson(const QString& s)
{
    return QString(s).replace("\"", "\\\"");
}
