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

#include "csavefilemessage.h"
#include "asctabwidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QMessageBox>
#include "defines.h"

#ifdef Q_WS_WIN32
//#define WINVER 0x0500
#include <windows.h>
#endif // Q_WS_WIN32

#include <QDebug>
extern BYTE g_dpi_ratio;
extern QString g_lang;

#if defined(_WIN32)
CSaveFileMessage::CSaveFileMessage(HWND hParentWnd)
    : QWinWidget(hParentWnd),
      m_pDlg(this),
#else
CSaveFileMessage::CSaveFileMessage(QWidget * parent)
    : QWidget(parent),
    m_pDlg(parent),
#endif
    m_result(MODAL_RESULT_CANCEL), m_fLayout(new QFormLayout), m_mapFiles(NULL)
{
    m_pDlg.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                          | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);

//        HWND hwnd = (HWND)winId();
//        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
//        style &= ~WS_SYSMENU; // unset the system menu flag
//        SetWindowLongPtr(hwnd, GWL_STYLE, style);
//        // force Windows to refresh some cached window styles
//        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);


    QVBoxLayout * layout = new QVBoxLayout;
    QHBoxLayout * h_layout2 = new QHBoxLayout;
    QHBoxLayout * h_layout1 = new QHBoxLayout;
    layout->addLayout(h_layout2, 1);
    layout->addLayout(h_layout1, 0);

//    QVBoxLayout * v_layout1 = new QVBoxLayout;
    QLabel * icon = new QLabel;
    icon->setProperty("class","msg-icon");
    icon->setProperty("type","msg-question");
    icon->setFixedSize(35*g_dpi_ratio, 35*g_dpi_ratio);

    QLabel * question = new QLabel(tr("Do you want to save modified files?"));
    question->setObjectName("label-question");
//    question->setStyleSheet(QString("margin-bottom: %1px;").arg(8*g_dpi_ratio));
    m_fLayout->addWidget(question);
    h_layout2->addWidget(icon, 0, Qt::AlignTop);
    h_layout2->addLayout(m_fLayout, 1);
    h_layout2->setContentsMargins(15,10,15,10);
    m_fLayout->setContentsMargins(10,0,0,0);

    QPushButton * btn_yes       = new QPushButton(tr("&Yes"));
    QPushButton * btn_no        = new QPushButton(tr("&No"));
    QPushButton * btn_cancel    = new QPushButton(tr("&Cancel"));

    QWidget * box = new QWidget;
    box->setLayout(new QHBoxLayout);
    box->layout()->addWidget(btn_yes);
    box->layout()->addWidget(btn_no);
    box->layout()->addWidget(btn_cancel);
    box->layout()->setContentsMargins(0,8*g_dpi_ratio,0,0);
    h_layout1->addWidget(box, 0, Qt::AlignCenter);

    m_pDlg.setLayout(layout);
    m_pDlg.setMinimumWidth(400*g_dpi_ratio);
    m_pDlg.setWindowTitle(APP_TITLE);

    connect(btn_yes, &QPushButton::clicked, this, &CSaveFileMessage::onYesClicked);
    connect(btn_no, &QPushButton::clicked, [=]{m_result = MODAL_RESULT_NO; m_pDlg.reject();});
    connect(btn_cancel, &QPushButton::clicked, [=]{m_result = MODAL_RESULT_CANCEL;m_pDlg.reject();});

    setStyleSheet("QPushButton:focus{border-color:#3a83db;}");
}

CSaveFileMessage::~CSaveFileMessage()
{
}

int CSaveFileMessage::showModal()
{
    m_pDlg.adjustSize();

#if defined(_WIN32)
    RECT rc;
    ::GetWindowRect(parentWindow(), &rc);

    int x = rc.left + (rc.right - rc.left - m_pDlg.width())/2;
    int y = (rc.bottom - rc.top - m_pDlg.height())/2;

    m_pDlg.move(x, y);
#endif

    m_pDlg.exec();

    return m_result;
}

void CSaveFileMessage::setFiles(QMap<int, QString> * f)
{
    m_mapFiles = f;

    QWidget * chb;
    if (m_mapFiles->size() > 1) {
        for (auto k : m_mapFiles->keys()) {
            chb = new QCheckBox( m_mapFiles->value(k) );
            qobject_cast<QCheckBox*>(chb)->setCheckState(Qt::Checked);
            chb->setProperty("view_id", k);

            m_fLayout->addWidget(chb);
        }
    } else {
        chb = new QLabel(m_mapFiles->begin().value());
        m_fLayout->addWidget(chb);
    }
}

void CSaveFileMessage::setFiles(const QString& file)
{
    m_fLayout->addWidget(new QLabel(file));
}

void CSaveFileMessage::setText(const QString& text)
{
    QLabel * question = m_pDlg.findChild<QLabel *>("label-question");
    if (question) question->setText(text);
}

void CSaveFileMessage::onYesClicked()
{
    if (m_mapFiles && m_mapFiles->size() > 1) {
        QCheckBox * chb;
        int count = m_fLayout->count();
        for (int i = count; i-- > 1; ) {
            chb = (QCheckBox *)m_fLayout->itemAt(i, QFormLayout::FieldRole)->widget();

            if (chb->checkState() == Qt::Unchecked) {
                m_mapFiles->remove(chb->property("view_id").toInt());
            }
        }
    }

    m_result = MODAL_RESULT_YES;
    m_pDlg.accept();
}