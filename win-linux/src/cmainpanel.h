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

#ifndef CMAINPANEL_H
#define CMAINPANEL_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSettings>

#include "asctabwidget.h"
#include "cdownloadwidget.h"
#include "cpushbutton.h"
#include "applicationmanager.h"
#include "ccefeventstransformer.h"


struct printdata;

class CMainPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CMainPanel(QWidget *parent, CAscApplicationManager *pManager, bool isCustomWindow);

    CAscApplicationManager * getAscApplicationManager();
    void applyMainWindowState(Qt::WindowState);

    void goStart();
    void focus();
    void doOpenLocalFile(COpenOptions&);
    void doOpenLocalFiles(const vector<wstring> *);
    void doOpenLocalFiles(const QStringList&);
    void doOpenLocalFiles();
    void setInputFiles(QStringList *);

#ifdef __linux
    QWidget * getTitleWidget();
    void setMouseTracking(bool);
#endif

protected:
    void refreshAboutVersion();
    void cmdMainPage(const QString&, const QString&) const;
    void cmdAppManager(int, void *);

    QString m_saveDocMessage;

private:
//    bool nativeEvent(const QByteArray &, void *msg, long *result);
//    void mousePressEvent( QMouseEvent *event );

    void resizeEvent(QResizeEvent* event);
//    bool eventFilter(QObject *obj, QEvent *event);

    void toggleButtonMain(bool);
    void loadStartPage();
    void doLogout(const QString&, bool);
    int  trySaveDocument(int);

    void readSystemUserName(wstring& first, wstring& last);
signals:
//    void downloadEvent(NSEditorApi::CAscDownloadFileInfo *);
    void mainWindowChangeState(Qt::WindowState);
    void mainWindowClose();
    void mainPageReady();

public slots:
    void pushButtonMinimizeClicked();
    void pushButtonMaximizeClicked();
    void pushButtonCloseClicked();
    void pushButtonMainClicked();

    void onTabClicked(int);
    void onTabChanged(int);
    void onTabClosed(int, int);
    void onTabCloseRequest(int);
    void onAppCloseRequest();

    void onCloudDocumentOpen(std::wstring, int, bool);
    void onDocumentType(int id, int type);
    void onDocumentName(void *);
    void onDocumentChanged(int id, bool changed);
    void onDocumentSave(int id, bool cancel);
    void onDocumentDownload(void * info);

    void onDocumentPrint(void *);
    void onDialogSave(std::wstring sName, uint id);
    void onFullScreen(bool);
    void onKeyDown(void *);
    void onLink(QString);

    void onNeedCheckKeyboard();

    void onLocalFileOpen(const QString&);
    void onLocalFilesOpen(void *);
    void onLocalFileCreate(int);
    void onLocalFileRecent(void *);
    void onLocalFileSaveAs(void *);
    void onLocalFilesCheck(QString);
    void onLocalFileLocation(QString);
    void onLocalGetImage(void *);
    void onPortalOpen(QString);
    void onPortalLogin(QString);
    void onPortalLogout(QString);
    void onPortalNew(QString);
    void onPortalCreate();

    void onMainPageReady();

private:
    std::wstring    m_sDownloadName;

    QPushButton*    m_pButtonMain;
    QWidget*        m_pMainWidget;

    QPushButton*    m_pButtonMinimize;
    QPushButton*    m_pButtonMaximize;
    QPushButton*    m_pButtonClose;
    QPushButton*    m_pButtonProfile;
    CPushButton*    m_pButtonDownload;

    QHBoxLayout *   m_layoutBtns;
    QWidget *       m_boxTitleBtns;
    CAscTabWidget * m_pTabs;
    bool            m_isMaximized;
    bool            m_isCustomWindow;

    CDownloadWidget *       m_pWidgetDownload;
    CAscApplicationManager * m_pManager;

    printdata *    m_printData;
    Qt::WindowState m_mainWindowState;

    QStringList * m_inFiles;

    QString m_savePortal;
    int m_saveAction;
public:
    WId GetHwndForKeyboard()
    {
        return ((QWidget*)m_pTabs->parent())->winId();
    }


private:
    void RecalculatePlaces();

};

#endif // CMAINPANEL_H
