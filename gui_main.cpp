#define _WIN32_IE 0x0500
#include <windows.h>
#include <commctrl.h>
#include "datamanager.h"
#include <cstdio>
#include <sstream>
#include <cstring>
#include <cstdlib>

#pragma comment(lib, "comctl32.lib")

// GCC 3.4.5 compat: Ptr variants not available, 32-bit only
#ifndef GetWindowLongPtr
#define GetWindowLongPtr GetWindowLong
#endif
#ifndef SetWindowLongPtr
#define SetWindowLongPtr SetWindowLong
#endif
#ifndef GWLP_USERDATA
#define GWLP_USERDATA GWL_USERDATA
#endif
#ifndef DS_CENTER
#define DS_CENTER 0x0800
#endif

DataManager* g_dm = NULL;
HINSTANCE g_hInst = NULL;
HFONT g_hFont = NULL;
User* g_loginResult = NULL;  // 登录结果，用于跨消息循环传递

// ============ IDs ============
enum {
    // Login
    ID_LOGN_USER = 1001, ID_LOGN_PASS, ID_LOGN_OK, ID_LOGN_REG, ID_LOGN_EXIT,
    // Register
    ID_REG_USER, ID_REG_PASS, ID_REG_PASS2, ID_REG_MAJOR, ID_REG_CLASS, ID_REG_SID, ID_REG_OK, ID_REG_CANCEL,
    // Student tabs
    ID_ST_TAB = 2000,
    ID_ST_AN_LIST, ID_ST_ACC_LIST,           // tab 0: announcements
    ID_ST_PT, ID_ST_PN, ID_ST_DUR, ID_ST_PERF, ID_ST_DESC, ID_ST_SAVE, // tab 1: program
    ID_ST_MAT, ID_ST_MAT_SAVE,                // tab 2: materials
    ID_ST_INFO, ID_ST_CHP_OLD, ID_ST_CHP_NEW, ID_ST_CHP_NEW2, ID_ST_CHP_OK, // tab 3: profile
    // Admin tabs
    ID_AD_TAB = 3000,
    ID_AD_AN_LIST, ID_AD_AN_TITLE, ID_AD_AN_CONT, ID_AD_AN_TYPE, ID_AD_AN_ADD, ID_AD_AN_EDIT, ID_AD_AN_DEL,
    ID_AD_RV_LIST, ID_AD_RV_DETAIL, ID_AD_RV_APPR, ID_AD_RV_REJ, ID_AD_RV_UNDO,
    ID_AD_MJ_LIST, ID_AD_CL_LIST, ID_AD_MJ_ADD, ID_AD_MJ_DEL, ID_AD_CL_ADD, ID_AD_CL_DEL,
    ID_AD_CHP_OLD, ID_AD_CHP_NEW, ID_AD_CHP_NEW2, ID_AD_CHP_OK,
};

// Forward declares
LRESULT CALLBACK LoginWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK StudentWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AdminWndProc(HWND, UINT, WPARAM, LPARAM);
void CreateStudentTabs(HWND hwnd);
void CreateAdminTabs(HWND hwnd);
void ShowStudentTab(HWND hwnd, int tab);
void ShowAdminTab(HWND hwnd, int tab);
void RefreshStAnnounceList(HWND hList);
void RefreshStAcceptList(HWND hList);
void RefreshAdAnnounceList(HWND hList);
void RefreshAdReviewList(HWND hList);
void RefreshAdMajorList(HWND hList);
void RefreshAdClassList(HWND hList, const char* major);

// ==================== 辅助 ====================

void AddLVColumn(HWND hList, int idx, const char* text, int w) {
    LVCOLUMN lvc; memset(&lvc,0,sizeof(lvc));
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = w; lvc.pszText = (LPSTR)text;
    ListView_InsertColumn(hList, idx, &lvc);
}

void SetLVItem(HWND hList, int row, int col, const char* text) {
    LVITEM lvi; memset(&lvi,0,sizeof(lvi));
    lvi.iItem = row; lvi.iSubItem = col; lvi.mask = LVIF_TEXT; lvi.pszText = (LPSTR)text;
    if (col == 0) ListView_InsertItem(hList, &lvi);
    else ListView_SetItem(hList, &lvi);
}

void RefreshStAnnounceList(HWND hList) {
    ListView_DeleteAllItems(hList);
    const std::vector<Announcement*>& anns = g_dm->getAnnouncements();
    for (size_t i = 0; i < anns.size(); ++i) {
        std::ostringstream oss; oss << anns[i]->getId();
        SetLVItem(hList,i,0,oss.str().c_str());
        SetLVItem(hList,i,1,anns[i]->getTitle().c_str());
        SetLVItem(hList,i,2,anns[i]->getAuthor().c_str());
        SetLVItem(hList,i,3,anns[i]->getCreateTime().c_str());
    }
}

void RefreshStAcceptList(HWND hList) {
    ListView_DeleteAllItems(hList);
    const std::vector<Registration*>& all = g_dm->getAllRegistrations();
    int row = 0;
    for (size_t i = 0; i < all.size(); ++i) {
        if (all[i]->isApproved()) {
            std::ostringstream oss; oss << all[i]->getId();
            SetLVItem(hList,row,0,oss.str().c_str());
            SetLVItem(hList,row,1,all[i]->getProgramName().c_str());
            SetLVItem(hList,row,2,all[i]->getProgramType().c_str());
            SetLVItem(hList,row,3,all[i]->getPerformer().c_str());
            row++;
        }
    }
}

void RefreshAdAnnounceList(HWND hList) {
    ListView_DeleteAllItems(hList);
    const std::vector<Announcement*>& anns = g_dm->getAnnouncements();
    for (size_t i = 0; i < anns.size(); ++i) {
        std::ostringstream oss; oss << anns[i]->getId();
        SetLVItem(hList,i,0,oss.str().c_str());
        SetLVItem(hList,i,1,anns[i]->getTitle().c_str());
        SetLVItem(hList,i,2,anns[i]->isAcceptedList()?"入选名单":"普通公告");
        SetLVItem(hList,i,3,anns[i]->getCreateTime().c_str());
    }
}

void RefreshAdReviewList(HWND hList) {
    ListView_DeleteAllItems(hList);
    const std::vector<Registration*>& all = g_dm->getAllRegistrations();
    for (size_t i = 0; i < all.size(); ++i) {
        std::ostringstream oss; oss << all[i]->getId();
        SetLVItem(hList,i,0,oss.str().c_str());
        SetLVItem(hList,i,1,all[i]->getUsername().c_str());
        SetLVItem(hList,i,2,all[i]->getProgramName().c_str());
        SetLVItem(hList,i,3,all[i]->getStatusDisplay().c_str());
    }
}

void RefreshAdMajorList(HWND hList) {
    ListView_DeleteAllItems(hList);
    const std::vector<std::string>& majors = g_dm->getMajors();
    for (size_t i = 0; i < majors.size(); ++i)
        SetLVItem(hList,i,0,majors[i].c_str());
}

void RefreshAdClassList(HWND hList, const char* major) {
    ListView_DeleteAllItems(hList);
    if (!major || !*major) return;
    const std::vector<std::string>& classes = g_dm->getClasses(std::string(major));
    for (size_t i = 0; i < classes.size(); ++i)
        SetLVItem(hList,i,0,classes[i].c_str());
}

// ==================== 登录窗口（含内嵌注册表单） ====================

// 控件句柄 —— 登录
static HWND hLoginUser, hLoginPass, hLoginBtn, hRegSwitchBtn, hExitBtn;
// 控件句柄 —— 注册（与登录控件共享同一父窗口，互斥显示）
static HWND hRegUser, hRegPass, hRegPass2, hRegMajor, hRegClass, hRegSid, hRegOk, hRegCancel;
// 标签
static HWND hLoginLabels[3];  // 用户名、密码标签 + 登录按钮等
static HWND hRegLabels[7];    // 注册表单的标签

void ShowLoginControls(BOOL show) {
    int sw = show ? SW_SHOW : SW_HIDE;
    ShowWindow(hLoginUser, sw);
    ShowWindow(hLoginPass, sw);
    ShowWindow(hLoginBtn, sw);
    ShowWindow(hRegSwitchBtn, sw);
    ShowWindow(hExitBtn, sw);
    for (int i = 0; i < 3; ++i) ShowWindow(hLoginLabels[i], sw);
}

void ShowRegisterControls(BOOL show) {
    int sw = show ? SW_SHOW : SW_HIDE;
    ShowWindow(hRegUser, sw);
    ShowWindow(hRegPass, sw);
    ShowWindow(hRegPass2, sw);
    ShowWindow(hRegMajor, sw);
    ShowWindow(hRegClass, sw);
    ShowWindow(hRegSid, sw);
    ShowWindow(hRegOk, sw);
    ShowWindow(hRegCancel, sw);
    for (int i = 0; i < 7; ++i) ShowWindow(hRegLabels[i], sw);
}

LRESULT CALLBACK LoginWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch(msg) {
        case WM_CREATE: {
            // ===== 登录控件 =====
            hLoginLabels[0] = CreateWindow("STATIC", "用户名:", WS_CHILD|WS_VISIBLE, 30,30,65,25, hwnd,NULL,g_hInst,NULL);
            hLoginUser = CreateWindow("EDIT","",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL,100,28,220,25,hwnd,(HMENU)ID_LOGN_USER,g_hInst,NULL);

            hLoginLabels[1] = CreateWindow("STATIC","密  码:",WS_CHILD|WS_VISIBLE,30,72,65,25,hwnd,NULL,g_hInst,NULL);
            hLoginPass = CreateWindow("EDIT","",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_PASSWORD|ES_AUTOHSCROLL,100,70,220,25,hwnd,(HMENU)ID_LOGN_PASS,g_hInst,NULL);

            hLoginBtn = CreateWindow("BUTTON","登录",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,40,130,90,35,hwnd,(HMENU)ID_LOGN_OK,g_hInst,NULL);
            hRegSwitchBtn = CreateWindow("BUTTON","注册",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,145,130,90,35,hwnd,(HMENU)ID_LOGN_REG,g_hInst,NULL);
            hExitBtn = CreateWindow("BUTTON","退出",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,250,130,90,35,hwnd,(HMENU)ID_LOGN_EXIT,g_hInst,NULL);

            // ===== 注册控件（初始隐藏）=====
            hRegLabels[0] = CreateWindow("STATIC","用户名:",WS_CHILD|WS_VISIBLE,25,18,65,22,hwnd,NULL,g_hInst,NULL);
            hRegUser = CreateWindow("EDIT","",WS_CHILD|WS_BORDER,110,16,260,22,hwnd,(HMENU)ID_REG_USER,g_hInst,NULL);

            hRegLabels[1] = CreateWindow("STATIC","密码:",WS_CHILD|WS_VISIBLE,25,52,65,22,hwnd,NULL,g_hInst,NULL);
            hRegPass = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_PASSWORD,110,50,260,22,hwnd,(HMENU)ID_REG_PASS,g_hInst,NULL);

            hRegLabels[2] = CreateWindow("STATIC","确认密码:",WS_CHILD|WS_VISIBLE,25,86,65,22,hwnd,NULL,g_hInst,NULL);
            hRegPass2 = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_PASSWORD,110,84,260,22,hwnd,(HMENU)ID_REG_PASS2,g_hInst,NULL);

            hRegLabels[3] = CreateWindow("STATIC","专业:",WS_CHILD|WS_VISIBLE,25,120,65,22,hwnd,NULL,g_hInst,NULL);
            hRegMajor = CreateWindow("COMBOBOX","",WS_CHILD|CBS_DROPDOWNLIST,110,118,260,200,hwnd,(HMENU)ID_REG_MAJOR,g_hInst,NULL);
            {
                const std::vector<std::string>& mj = g_dm->getMajors();
                for (size_t i=0;i<mj.size();++i) SendMessage(hRegMajor,CB_ADDSTRING,0,(LPARAM)mj[i].c_str());
                if (!mj.empty()) SendMessage(hRegMajor,CB_SETCURSEL,0,0);
            }

            hRegLabels[4] = CreateWindow("STATIC","班级:",WS_CHILD|WS_VISIBLE,25,154,65,22,hwnd,NULL,g_hInst,NULL);
            hRegClass = CreateWindow("COMBOBOX","",WS_CHILD|CBS_DROPDOWNLIST,110,152,260,200,hwnd,(HMENU)ID_REG_CLASS,g_hInst,NULL);
            {
                const std::vector<std::string>& mj = g_dm->getMajors();
                if (!mj.empty()) {
                    const std::vector<std::string>& cl = g_dm->getClasses(mj[0]);
                    for (size_t i=0;i<cl.size();++i) SendMessage(hRegClass,CB_ADDSTRING,0,(LPARAM)cl[i].c_str());
                    if (!cl.empty()) SendMessage(hRegClass,CB_SETCURSEL,0,0);
                }
            }

            hRegLabels[5] = CreateWindow("STATIC","学号:",WS_CHILD|WS_VISIBLE,25,188,65,22,hwnd,NULL,g_hInst,NULL);
            hRegSid = CreateWindow("EDIT","",WS_CHILD|WS_BORDER,110,186,260,22,hwnd,(HMENU)ID_REG_SID,g_hInst,NULL);

            hRegLabels[6] = CreateWindow("STATIC","",0,0,0,0,0,hwnd,NULL,g_hInst,NULL);  // placeholder
            hRegOk = CreateWindow("BUTTON","注册",WS_CHILD|BS_PUSHBUTTON,70,240,100,35,hwnd,(HMENU)ID_REG_OK,g_hInst,NULL);
            hRegCancel = CreateWindow("BUTTON","返回登录",WS_CHILD|BS_PUSHBUTTON,240,240,100,35,hwnd,(HMENU)ID_REG_CANCEL,g_hInst,NULL);

            // 初始状态：显示登录，隐藏注册
            ShowRegisterControls(FALSE);

            SendMessage(hwnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hLoginUser, EM_SETLIMITTEXT, 127, 0);
            SendMessage(hLoginPass, EM_SETLIMITTEXT, 127, 0);
            break;
        }

        case WM_COMMAND: {
            WORD id = LOWORD(wp);

            if (id == ID_LOGN_OK) {
                char u[128], p[128];
                GetWindowText(hLoginUser, u, sizeof(u));
                GetWindowText(hLoginPass, p, sizeof(p));
                User* user = g_dm->authenticate(u, p);
                if (user) {
                    g_loginResult = user;
                    DestroyWindow(hwnd);
                } else {
                    MessageBox(hwnd, "用户名或密码错误！", "错误", MB_ICONERROR);
                }
            }
            else if (id == ID_LOGN_EXIT) {
                g_loginResult = NULL;
                DestroyWindow(hwnd);
            }
            else if (id == ID_LOGN_REG) {
                // 切换到注册界面：拉高窗口
                SetWindowPos(hwnd, NULL, 0,0,380,340, SWP_NOMOVE|SWP_NOZORDER);
                SetWindowText(hwnd, "学生注册 - 我新我秀");
                ShowLoginControls(FALSE);
                ShowRegisterControls(TRUE);
            }
            else if (id == ID_REG_CANCEL) {
                // 返回登录界面：还原窗口
                SetWindowPos(hwnd, NULL, 0,0,380,240, SWP_NOMOVE|SWP_NOZORDER);
                SetWindowText(hwnd, "我新我秀 报名系统 - 登录");
                ShowRegisterControls(FALSE);
                ShowLoginControls(TRUE);
                // 清空注册表单
                SetWindowText(hRegUser, "");
                SetWindowText(hRegPass, "");
                SetWindowText(hRegPass2, "");
                SetWindowText(hRegSid, "");
            }
            else if (id == ID_REG_OK) {
                char u[128],p1[128],p2[128],mj[128],cl[128],sid[128];
                GetWindowText(hRegUser,u,sizeof(u));
                GetWindowText(hRegPass,p1,sizeof(p1));
                GetWindowText(hRegPass2,p2,sizeof(p2));
                int idx = SendMessage(hRegMajor,CB_GETCURSEL,0,0);
                if (idx >= 0) SendMessage(hRegMajor,CB_GETLBTEXT,idx,(LPARAM)mj); else mj[0]=0;
                idx = SendMessage(hRegClass,CB_GETCURSEL,0,0);
                if (idx >= 0) SendMessage(hRegClass,CB_GETLBTEXT,idx,(LPARAM)cl); else cl[0]=0;
                GetWindowText(hRegSid,sid,sizeof(sid));

                if (!u[0]||!p1[0]) { MessageBox(hwnd,"用户名和密码不能为空！","错误",MB_ICONERROR); return 0; }
                if (strcmp(p1,p2)) { MessageBox(hwnd,"两次密码不一致！","错误",MB_ICONERROR); return 0; }
                if (strlen(p1)<4) { MessageBox(hwnd,"密码至少4位！","错误",MB_ICONERROR); return 0; }
                if (g_dm->userExists(u)) { MessageBox(hwnd,"用户名已存在！","错误",MB_ICONERROR); return 0; }
                if (!mj[0]||!cl[0]||!sid[0]) { MessageBox(hwnd,"请完善所有信息！","错误",MB_ICONERROR); return 0; }

                Student* stu = new Student(u,p1,mj,cl,sid);
                if (g_dm->addUser(stu)) {
                    MessageBox(hwnd,"注册成功！请登录。","提示",MB_ICONINFORMATION);
                    // 返回登录界面：还原窗口
                    SetWindowPos(hwnd, NULL, 0,0,380,240, SWP_NOMOVE|SWP_NOZORDER);
                    SetWindowText(hwnd, "我新我秀 报名系统 - 登录");
                    ShowRegisterControls(FALSE);
                    ShowLoginControls(TRUE);
                    SetWindowText(hLoginUser, u);
                    SetWindowText(hLoginPass, "");
                } else {
                    delete stu;
                    MessageBox(hwnd,"注册失败！","错误",MB_ICONERROR);
                }
            }
            else if (id == ID_REG_MAJOR && HIWORD(wp) == CBN_SELCHANGE) {
                char buf[256];
                int idx = SendMessage(hRegMajor,CB_GETCURSEL,0,0);
                if (idx >= 0) {
                    SendMessage(hRegMajor,CB_GETLBTEXT,idx,(LPARAM)buf);
                    SendMessage(hRegClass,CB_RESETCONTENT,0,0);
                    const std::vector<std::string>& cl = g_dm->getClasses(std::string(buf));
                    for (size_t i=0;i<cl.size();++i) SendMessage(hRegClass,CB_ADDSTRING,0,(LPARAM)cl[i].c_str());
                    if (!cl.empty()) SendMessage(hRegClass,CB_SETCURSEL,0,0);
                }
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

// ==================== 学生窗口 ====================

// Tab 0: 公告查看
static HWND stAnList, stAnContent, stLabelAn, stLabelAnContent;
// Tab 1: 节目报名
static HWND stPT, stPN, stDur, stPerf, stDesc, stSaveBtn;
static HWND stLabelPt, stLabelPn, stLabelDur, stLabelPerf, stLabelDesc;
// Tab 2: 材料提交
static HWND stMat, stMatSaveBtn, stLabelMatTitle, stLabelMatHint;
// Tab 3: 个人中心
static HWND stInfo, stLabelPwdTitle, stLabelPwdOld, stLabelPwdNew, stLabelPwdNew2;
static HWND stChpOld, stChpNew, stChpNew2, stChpOkBtn;

void CreateStudentTabs(HWND hwnd) {
    RECT rc; GetClientRect(hwnd, &rc);
    int tabH = 30;
    HWND hTab = CreateWindow(WC_TABCONTROL, "", WS_CHILD|WS_VISIBLE|TCS_FIXEDWIDTH,
        0,0,rc.right,tabH, hwnd, (HMENU)ID_ST_TAB, g_hInst, NULL);
    SendMessage(hTab, WM_SETFONT, (WPARAM)g_hFont, TRUE);

    TCITEM tci; memset(&tci,0,sizeof(tci));
    tci.mask = TCIF_TEXT;
    tci.pszText = "公告查看"; TabCtrl_InsertItem(hTab, 0, &tci);
    tci.pszText = "节目报名"; TabCtrl_InsertItem(hTab, 1, &tci);
    tci.pszText = "材料提交"; TabCtrl_InsertItem(hTab, 2, &tci);
    tci.pszText = "个人中心"; TabCtrl_InsertItem(hTab, 3, &tci);

    int y = tabH + 2;
    int w = rc.right - 10;

    // ===== Tab 0: 公告查看 =====
    // 上方：公告列表
    stLabelAn = CreateWindow("STATIC","公告列表（点击查看内容）:",WS_CHILD|WS_VISIBLE,5,y,200,20,hwnd,NULL,g_hInst,NULL);
    stAnList = CreateWindow(WC_LISTVIEW,"",WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT|LVS_SINGLESEL,
        5,y+20,w,120,hwnd,(HMENU)ID_ST_AN_LIST,g_hInst,NULL);
    AddLVColumn(stAnList,0,"ID",40); AddLVColumn(stAnList,1,"标题",180);
    AddLVColumn(stAnList,2,"发布者",80); AddLVColumn(stAnList,3,"时间",140);

    // 下方：公告内容
    stLabelAnContent = CreateWindow("STATIC","公告内容:",WS_CHILD|WS_VISIBLE,5,y+146,200,20,hwnd,NULL,g_hInst,NULL);
    stAnContent = CreateWindow("EDIT","",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|ES_READONLY|WS_VSCROLL,
        5,y+166,w,rc.bottom-y-176,hwnd,(HMENU)ID_ST_AN_LIST+1,g_hInst,NULL);

    // ===== Tab 1: 节目报名 =====
    stLabelPt = CreateWindow("STATIC","节目类型:",WS_CHILD,5,y,70,20,hwnd,NULL,g_hInst,NULL);
    stPT = CreateWindow("COMBOBOX","",WS_CHILD|CBS_DROPDOWNLIST,80,y-2,200,200,hwnd,(HMENU)ID_ST_PT,g_hInst,NULL);
    {
        const std::vector<std::string>& ts = Registration::getProgramTypes();
        for (size_t i=0;i<ts.size();++i) SendMessage(stPT,CB_ADDSTRING,0,(LPARAM)ts[i].c_str());
        SendMessage(stPT,CB_SETCURSEL,0,0);
    }
    stLabelPn = CreateWindow("STATIC","节目名称:",WS_CHILD,5,y+30,70,20,hwnd,NULL,g_hInst,NULL);
    stPN = CreateWindow("EDIT","",WS_CHILD|WS_BORDER,80,y+28,w-85,22,hwnd,(HMENU)ID_ST_PN,g_hInst,NULL);
    stLabelDur = CreateWindow("STATIC","时长(分钟):",WS_CHILD,5,y+58,70,20,hwnd,NULL,g_hInst,NULL);
    stDur = CreateWindow("EDIT","",WS_CHILD|WS_BORDER,80,y+56,60,22,hwnd,(HMENU)ID_ST_DUR,g_hInst,NULL);
    stLabelPerf = CreateWindow("STATIC","表演者:",WS_CHILD,155,y+58,60,20,hwnd,NULL,g_hInst,NULL);
    stPerf = CreateWindow("EDIT","",WS_CHILD|WS_BORDER,215,y+56,w-220,22,hwnd,(HMENU)ID_ST_PERF,g_hInst,NULL);
    stLabelDesc = CreateWindow("STATIC","节目描述 (创意说明/特殊需求等):",WS_CHILD,5,y+84,250,20,hwnd,NULL,g_hInst,NULL);
    stDesc = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL,5,y+106,w,100,hwnd,(HMENU)ID_ST_DESC,g_hInst,NULL);
    stSaveBtn = CreateWindow("BUTTON","保存报名信息",WS_CHILD|BS_PUSHBUTTON,5,y+214,120,30,hwnd,(HMENU)ID_ST_SAVE,g_hInst,NULL);

    // ===== Tab 2: 材料提交 =====
    stLabelMatTitle = CreateWindow("STATIC","节目材料编辑:",WS_CHILD,5,y,280,20,hwnd,NULL,g_hInst,NULL);
    stLabelMatHint = CreateWindow("STATIC","请在此填写您节目需要的材料，如背景音乐名称、道具清单、剧本摘要等。这些信息将供审核参考。",
        WS_CHILD,5,y+20,w,30,hwnd,NULL,g_hInst,NULL);
    stMat = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL,5,y+54,w,150,hwnd,(HMENU)ID_ST_MAT,g_hInst,NULL);
    stMatSaveBtn = CreateWindow("BUTTON","保存材料",WS_CHILD|BS_PUSHBUTTON,5,y+214,120,30,hwnd,(HMENU)ID_ST_MAT_SAVE,g_hInst,NULL);

    // ===== Tab 3: 个人中心 =====
    stInfo = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_MULTILINE|ES_READONLY|WS_VSCROLL,5,y,w,100,hwnd,(HMENU)ID_ST_INFO,g_hInst,NULL);
    stLabelPwdTitle = CreateWindow("STATIC","修改密码:",WS_CHILD,5,y+108,70,20,hwnd,NULL,g_hInst,NULL);
    stLabelPwdOld = CreateWindow("STATIC","旧密码:",WS_CHILD,5,y+132,50,20,hwnd,NULL,g_hInst,NULL);
    stChpOld = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_PASSWORD,60,y+130,100,22,hwnd,(HMENU)ID_ST_CHP_OLD,g_hInst,NULL);
    stLabelPwdNew = CreateWindow("STATIC","新密码:",WS_CHILD,170,y+132,50,20,hwnd,NULL,g_hInst,NULL);
    stChpNew = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_PASSWORD,225,y+130,100,22,hwnd,(HMENU)ID_ST_CHP_NEW,g_hInst,NULL);
    stLabelPwdNew2 = CreateWindow("STATIC","确认:",WS_CHILD,335,y+132,40,20,hwnd,NULL,g_hInst,NULL);
    stChpNew2 = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_PASSWORD,370,y+130,100,22,hwnd,(HMENU)ID_ST_CHP_NEW2,g_hInst,NULL);
    stChpOkBtn = CreateWindow("BUTTON","修改密码",WS_CHILD|BS_PUSHBUTTON,5,y+162,90,28,hwnd,(HMENU)ID_ST_CHP_OK,g_hInst,NULL);

    ShowStudentTab(hwnd, 0);
}

void ShowStudentTab(HWND hwnd, int tab) {
    int s0 = (tab==0)?SW_SHOW:SW_HIDE;
    int s1 = (tab==1)?SW_SHOW:SW_HIDE;
    int s2 = (tab==2)?SW_SHOW:SW_HIDE;
    int s3 = (tab==3)?SW_SHOW:SW_HIDE;

    // Tab 0: 公告查看
    ShowWindow(stLabelAn, s0);
    ShowWindow(stAnList, s0);
    ShowWindow(stLabelAnContent, s0);
    ShowWindow(stAnContent, s0);

    // Tab 1: 节目报名
    ShowWindow(stLabelPt, s1);
    ShowWindow(stPT, s1);
    ShowWindow(stLabelPn, s1);
    ShowWindow(stPN, s1);
    ShowWindow(stLabelDur, s1);
    ShowWindow(stDur, s1);
    ShowWindow(stLabelPerf, s1);
    ShowWindow(stPerf, s1);
    ShowWindow(stLabelDesc, s1);
    ShowWindow(stDesc, s1);
    ShowWindow(stSaveBtn, s1);

    // Tab 2: 材料提交
    ShowWindow(stLabelMatTitle, s2);
    ShowWindow(stLabelMatHint, s2);
    ShowWindow(stMat, s2);
    ShowWindow(stMatSaveBtn, s2);

    // Tab 3: 个人中心
    ShowWindow(stInfo, s3);
    ShowWindow(stLabelPwdTitle, s3);
    ShowWindow(stLabelPwdOld, s3);
    ShowWindow(stChpOld, s3);
    ShowWindow(stLabelPwdNew, s3);
    ShowWindow(stChpNew, s3);
    ShowWindow(stLabelPwdNew2, s3);
    ShowWindow(stChpNew2, s3);
    ShowWindow(stChpOkBtn, s3);
}

LRESULT CALLBACK StudentWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static User* user = NULL;
    switch(msg) {
        case WM_CREATE:
            user = (User*)((CREATESTRUCT*)lp)->lpCreateParams;
            CreateStudentTabs(hwnd);
            RefreshStAnnounceList(stAnList);
            break;
        case WM_NOTIFY: {
            NMHDR* nm = (NMHDR*)lp;
            if (nm->idFrom == ID_ST_TAB && nm->code == TCN_SELCHANGE) {
                int tab = TabCtrl_GetCurSel(nm->hwndFrom);
                ShowStudentTab(hwnd, tab);
                if (tab == 0) {
                    RefreshStAnnounceList(stAnList);
                    SetWindowText(stAnContent, "");
                }
                if (tab == 1) {
                    Registration* reg = g_dm->getRegistrationByUser(user->getUsername());
                    if (reg) {
                        SetWindowText(stPN, reg->getProgramName().c_str());
                        SetWindowText(stPerf, reg->getPerformer().c_str());
                        char buf[32]; sprintf(buf,"%d",reg->getDuration());
                        SetWindowText(stDur, buf);
                        SetWindowText(stDesc, reg->getDescription().c_str());
                        const std::vector<std::string>& ts = Registration::getProgramTypes();
                        for (size_t i=0;i<ts.size();++i) {
                            if (ts[i] == reg->getProgramType()) { SendMessage(stPT,CB_SETCURSEL,i,0); break; }
                        }
                        if (reg->isApproved()) {
                            EnableWindow(stPT, FALSE); EnableWindow(stPN, FALSE);
                            EnableWindow(stDur, FALSE); EnableWindow(stPerf, FALSE);
                            EnableWindow(stDesc, FALSE); EnableWindow(stSaveBtn, FALSE);
                        }
                    }
                }
                if (tab == 2) {
                    Registration* reg = g_dm->getRegistrationByUser(user->getUsername());
                    if (reg) {
                        SetWindowText(stMat, reg->getMaterials().c_str());
                        if (reg->isApproved()) EnableWindow(stMatSaveBtn, FALSE);
                    }
                }
                if (tab == 3) {
                    Student* stu = dynamic_cast<Student*>(user);
                    std::string info = "用户名: " + user->getUsername() + "\r\n";
                    if (stu) {
                        info += "专业: " + stu->getMajor() + "\r\n";
                        info += "班级: " + stu->getClassName() + "\r\n";
                        info += "学号: " + stu->getStudentId() + "\r\n";
                    }
                    SetWindowText(stInfo, info.c_str());
                }
            }
            // 点击公告列表中的公告 → 下方显示内容
            if (nm->idFrom == ID_ST_AN_LIST && nm->code == LVN_ITEMCHANGED) {
                NMLISTVIEW* nmlv = (NMLISTVIEW*)lp;
                if (nmlv->uNewState & LVIS_SELECTED) {
                    char buf[32];
                    ListView_GetItemText(stAnList, nmlv->iItem, 0, buf, sizeof(buf));
                    Announcement* ann = g_dm->findAnnouncement(atoi(buf));
                    if (ann) {
                        std::string content = "标题: " + ann->getTitle() + "\r\n";
                        content += "发布者: " + ann->getAuthor() + "\r\n";
                        content += "时间: " + ann->getCreateTime() + "\r\n";
                        content += "类型: " + std::string(ann->isAcceptedList()?"入选名单":"普通公告") + "\r\n";
                        content += "----------------------------------------\r\n";
                        content += ann->getContent();
                        SetWindowText(stAnContent, content.c_str());
                    }
                }
            }
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wp) == ID_ST_SAVE) {
                char pn[256], perf[256], dur[32], desc[4096];
                GetWindowText(stPN, pn, sizeof(pn));
                GetWindowText(stPerf, perf, sizeof(perf));
                GetWindowText(stDur, dur, sizeof(dur));
                GetWindowText(stDesc, desc, sizeof(desc));
                if (!pn[0]) { MessageBox(hwnd,"请输入节目名称！","错误",MB_ICONERROR); break; }
                int d = atoi(dur);
                if (d <= 0 || d > 30) { MessageBox(hwnd,"时长需在1-30分钟之间！","错误",MB_ICONERROR); break; }
                int ptIdx = SendMessage(stPT, CB_GETCURSEL, 0, 0);
                char pt[128];
                SendMessage(stPT, CB_GETLBTEXT, ptIdx, (LPARAM)pt);
                Registration* existing = g_dm->getRegistrationByUser(user->getUsername());
                Registration* reg;
                if (existing) {
                    reg = new Registration(existing->getId(), user->getUsername(), pt, d, pn, desc, perf);
                } else {
                    reg = new Registration(0, user->getUsername(), pt, d, pn, desc, perf);
                }
                g_dm->saveRegistration(reg);
                MessageBox(hwnd,"报名信息保存成功！","提示",MB_ICONINFORMATION);
            } else if (LOWORD(wp) == ID_ST_MAT_SAVE) {
                Registration* reg = g_dm->getRegistrationByUser(user->getUsername());
                if (!reg) { MessageBox(hwnd,"请先完成节目报名！","错误",MB_ICONERROR); break; }
                char mat[4096];
                GetWindowText(stMat, mat, sizeof(mat));
                reg->setMaterials(mat);
                g_dm->saveAll();
                MessageBox(hwnd,"材料保存成功！","提示",MB_ICONINFORMATION);
            } else if (LOWORD(wp) == ID_ST_CHP_OK) {
                char old[128], n1[128], n2[128];
                GetWindowText(stChpOld, old, sizeof(old));
                GetWindowText(stChpNew, n1, sizeof(n1));
                GetWindowText(stChpNew2, n2, sizeof(n2));
                if (!user->checkPassword(old)) { MessageBox(hwnd,"旧密码错误！","错误",MB_ICONERROR); break; }
                if (strcmp(n1,n2)) { MessageBox(hwnd,"两次密码不一致！","错误",MB_ICONERROR); break; }
                if (strlen(n1)<4) { MessageBox(hwnd,"密码至少4位！","错误",MB_ICONERROR); break; }
                user->setPassword(n1);
                g_dm->saveAll();
                MessageBox(hwnd,"密码修改成功！","提示",MB_ICONINFORMATION);
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

// ==================== 管理员窗口 ====================

// Tab 0: 公告管理
static HWND adAnList, adAnTitle, adAnCont, adAnType;
static HWND adLabelAnTitle, adLabelAnCont, adLabelAnType;
static HWND adAnAddBtn, adAnEditBtn, adAnDelBtn;
// Tab 1: 审核报名
static HWND adRvList, adRvDetail;
static HWND adLabelRvList, adLabelRvDetail;
static HWND adRvApprBtn, adRvRejBtn, adRvUndoBtn;
// Tab 2: 数据管理
static HWND adMjList, adClList;
static HWND adLabelMj, adLabelCl;
static HWND adMjAddBtn, adMjDelBtn, adClAddBtn, adClDelBtn;
// Tab 3: 修改密码
static HWND adChpOld, adChpNew, adChpNew2, adChpOkBtn;
static HWND adLabelChpTitle, adLabelChpOld, adLabelChpNew, adLabelChpNew2;

void CreateAdminTabs(HWND hwnd) {
    RECT rc; GetClientRect(hwnd, &rc);
    int tabH = 30;
    HWND hTab = CreateWindow(WC_TABCONTROL, "", WS_CHILD|WS_VISIBLE|TCS_FIXEDWIDTH,
        0,0,rc.right,tabH, hwnd, (HMENU)ID_AD_TAB, g_hInst, NULL);
    SendMessage(hTab, WM_SETFONT, (WPARAM)g_hFont, TRUE);

    TCITEM tci; memset(&tci,0,sizeof(tci)); tci.mask = TCIF_TEXT;
    tci.pszText = "公告管理"; TabCtrl_InsertItem(hTab,0,&tci);
    tci.pszText = "审核报名"; TabCtrl_InsertItem(hTab,1,&tci);
    tci.pszText = "数据管理"; TabCtrl_InsertItem(hTab,2,&tci);
    tci.pszText = "修改密码"; TabCtrl_InsertItem(hTab,3,&tci);

    int y = tabH + 2, w = rc.right - 10;

    // ===== Tab 0: 公告管理 =====
    adAnList = CreateWindow(WC_LISTVIEW,"",WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT|LVS_SINGLESEL,
        5,y,w,120,hwnd,(HMENU)ID_AD_AN_LIST,g_hInst,NULL);
    AddLVColumn(adAnList,0,"ID",40); AddLVColumn(adAnList,1,"标题",180);
    AddLVColumn(adAnList,2,"类型",80); AddLVColumn(adAnList,3,"时间",140);

    adLabelAnTitle = CreateWindow("STATIC","标题:",WS_CHILD|WS_VISIBLE,5,y+126,40,20,hwnd,NULL,g_hInst,NULL);
    adAnTitle = CreateWindow("EDIT","",WS_CHILD|WS_VISIBLE|WS_BORDER,50,y+124,w-55,22,hwnd,(HMENU)ID_AD_AN_TITLE,g_hInst,NULL);
    adLabelAnCont = CreateWindow("STATIC","内容:",WS_CHILD|WS_VISIBLE,5,y+152,40,20,hwnd,NULL,g_hInst,NULL);
    adAnCont = CreateWindow("EDIT","",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|WS_VSCROLL,5,y+172,w,70,hwnd,(HMENU)ID_AD_AN_CONT,g_hInst,NULL);
    adLabelAnType = CreateWindow("STATIC","类型:",WS_CHILD|WS_VISIBLE,5,y+248,40,20,hwnd,NULL,g_hInst,NULL);
    adAnType = CreateWindow("COMBOBOX","",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,50,y+246,100,100,hwnd,(HMENU)ID_AD_AN_TYPE,g_hInst,NULL);
    SendMessage(adAnType,CB_ADDSTRING,0,(LPARAM)"普通公告");
    SendMessage(adAnType,CB_ADDSTRING,0,(LPARAM)"入选名单");
    SendMessage(adAnType,CB_SETCURSEL,0,0);
    adAnAddBtn = CreateWindow("BUTTON","发布",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,5,y+278,70,28,hwnd,(HMENU)ID_AD_AN_ADD,g_hInst,NULL);
    adAnEditBtn = CreateWindow("BUTTON","修改",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,85,y+278,70,28,hwnd,(HMENU)ID_AD_AN_EDIT,g_hInst,NULL);
    adAnDelBtn = CreateWindow("BUTTON","删除",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,165,y+278,70,28,hwnd,(HMENU)ID_AD_AN_DEL,g_hInst,NULL);

    // ===== Tab 1: 审核报名 =====
    adLabelRvList = CreateWindow("STATIC","报名列表（点击查看详情）:",WS_CHILD,5,y,200,20,hwnd,NULL,g_hInst,NULL);
    adRvList = CreateWindow(WC_LISTVIEW,"",WS_CHILD|WS_BORDER|LVS_REPORT|LVS_SINGLESEL,
        5,y+20,w,110,hwnd,(HMENU)ID_AD_RV_LIST,g_hInst,NULL);
    AddLVColumn(adRvList,0,"ID",40); AddLVColumn(adRvList,1,"用户",80);
    AddLVColumn(adRvList,2,"节目",150); AddLVColumn(adRvList,3,"状态",80);
    adLabelRvDetail = CreateWindow("STATIC","报名详情:",WS_CHILD,5,y+136,200,20,hwnd,NULL,g_hInst,NULL);
    adRvDetail = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_MULTILINE|ES_READONLY|WS_VSCROLL,
        5,y+156,w,90,hwnd,(HMENU)ID_AD_RV_DETAIL,g_hInst,NULL);
    adRvApprBtn = CreateWindow("BUTTON","通过",WS_CHILD|BS_PUSHBUTTON,5,y+252,70,28,hwnd,(HMENU)ID_AD_RV_APPR,g_hInst,NULL);
    adRvRejBtn = CreateWindow("BUTTON","驳回",WS_CHILD|BS_PUSHBUTTON,85,y+252,70,28,hwnd,(HMENU)ID_AD_RV_REJ,g_hInst,NULL);
    adRvUndoBtn = CreateWindow("BUTTON","撤销",WS_CHILD|BS_PUSHBUTTON,165,y+252,70,28,hwnd,(HMENU)ID_AD_RV_UNDO,g_hInst,NULL);

    // ===== Tab 2: 数据管理 =====
    adLabelMj = CreateWindow("STATIC","专业列表（点击专业查看对应班级）:",WS_CHILD,5,y,220,20,hwnd,NULL,g_hInst,NULL);
    adMjList = CreateWindow(WC_LISTVIEW,"",WS_CHILD|WS_BORDER|LVS_REPORT|LVS_SINGLESEL,
        5,y+20,w/2-10,140,hwnd,(HMENU)ID_AD_MJ_LIST,g_hInst,NULL);
    AddLVColumn(adMjList,0,"专业名称",w/2-30);
    adMjAddBtn = CreateWindow("BUTTON","添加专业",WS_CHILD|BS_PUSHBUTTON,5,y+166,80,28,hwnd,(HMENU)ID_AD_MJ_ADD,g_hInst,NULL);
    adMjDelBtn = CreateWindow("BUTTON","删除专业",WS_CHILD|BS_PUSHBUTTON,95,y+166,80,28,hwnd,(HMENU)ID_AD_MJ_DEL,g_hInst,NULL);

    adLabelCl = CreateWindow("STATIC","班级列表:",WS_CHILD,w/2+5,y,220,20,hwnd,NULL,g_hInst,NULL);
    adClList = CreateWindow(WC_LISTVIEW,"",WS_CHILD|WS_BORDER|LVS_REPORT|LVS_SINGLESEL,
        w/2+5,y+20,w/2-10,140,hwnd,(HMENU)ID_AD_CL_LIST,g_hInst,NULL);
    AddLVColumn(adClList,0,"班级名称",w/2-30);
    adClAddBtn = CreateWindow("BUTTON","添加班级",WS_CHILD|BS_PUSHBUTTON,w/2+5,y+166,80,28,hwnd,(HMENU)ID_AD_CL_ADD,g_hInst,NULL);
    adClDelBtn = CreateWindow("BUTTON","删除班级",WS_CHILD|BS_PUSHBUTTON,w/2+95,y+166,80,28,hwnd,(HMENU)ID_AD_CL_DEL,g_hInst,NULL);

    // ===== Tab 3: 修改密码 =====
    adLabelChpTitle = CreateWindow("STATIC","修改登录密码:",WS_CHILD,5,y,200,20,hwnd,NULL,g_hInst,NULL);
    adLabelChpOld = CreateWindow("STATIC","旧密码:",WS_CHILD,5,y+30,50,20,hwnd,NULL,g_hInst,NULL);
    adChpOld = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_PASSWORD,60,y+28,150,22,hwnd,(HMENU)ID_AD_CHP_OLD,g_hInst,NULL);
    adLabelChpNew = CreateWindow("STATIC","新密码:",WS_CHILD,5,y+60,50,20,hwnd,NULL,g_hInst,NULL);
    adChpNew = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_PASSWORD,60,y+58,150,22,hwnd,(HMENU)ID_AD_CHP_NEW,g_hInst,NULL);
    adLabelChpNew2 = CreateWindow("STATIC","确认新密码:",WS_CHILD,5,y+90,80,20,hwnd,NULL,g_hInst,NULL);
    adChpNew2 = CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_PASSWORD,90,y+88,120,22,hwnd,(HMENU)ID_AD_CHP_NEW2,g_hInst,NULL);
    adChpOkBtn = CreateWindow("BUTTON","修改密码",WS_CHILD|BS_PUSHBUTTON,5,y+120,90,28,hwnd,(HMENU)ID_AD_CHP_OK,g_hInst,NULL);

    ShowAdminTab(hwnd, 0);
}

void ShowAdminTab(HWND hwnd, int tab) {
    int s0 = (tab==0)?SW_SHOW:SW_HIDE;
    int s1 = (tab==1)?SW_SHOW:SW_HIDE;
    int s2 = (tab==2)?SW_SHOW:SW_HIDE;
    int s3 = (tab==3)?SW_SHOW:SW_HIDE;

    // Tab 0: 公告管理
    ShowWindow(adAnList, s0);
    ShowWindow(adLabelAnTitle, s0);
    ShowWindow(adAnTitle, s0);
    ShowWindow(adLabelAnCont, s0);
    ShowWindow(adAnCont, s0);
    ShowWindow(adLabelAnType, s0);
    ShowWindow(adAnType, s0);
    ShowWindow(adAnAddBtn, s0);
    ShowWindow(adAnEditBtn, s0);
    ShowWindow(adAnDelBtn, s0);

    // Tab 1: 审核报名
    ShowWindow(adLabelRvList, s1);
    ShowWindow(adRvList, s1);
    ShowWindow(adLabelRvDetail, s1);
    ShowWindow(adRvDetail, s1);
    ShowWindow(adRvApprBtn, s1);
    ShowWindow(adRvRejBtn, s1);
    ShowWindow(adRvUndoBtn, s1);

    // Tab 2: 数据管理
    ShowWindow(adLabelMj, s2);
    ShowWindow(adMjList, s2);
    ShowWindow(adMjAddBtn, s2);
    ShowWindow(adMjDelBtn, s2);
    ShowWindow(adLabelCl, s2);
    ShowWindow(adClList, s2);
    ShowWindow(adClAddBtn, s2);
    ShowWindow(adClDelBtn, s2);

    // Tab 3: 修改密码
    ShowWindow(adLabelChpTitle, s3);
    ShowWindow(adLabelChpOld, s3);
    ShowWindow(adChpOld, s3);
    ShowWindow(adLabelChpNew, s3);
    ShowWindow(adChpNew, s3);
    ShowWindow(adLabelChpNew2, s3);
    ShowWindow(adChpNew2, s3);
    ShowWindow(adChpOkBtn, s3);
}

LRESULT CALLBACK AdminWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static User* user = NULL;
    switch(msg) {
        case WM_CREATE:
            user = (User*)((CREATESTRUCT*)lp)->lpCreateParams;
            CreateAdminTabs(hwnd);
            RefreshAdAnnounceList(adAnList);
            break;
        case WM_NOTIFY: {
            NMHDR* nm = (NMHDR*)lp;
            if (nm->idFrom == ID_AD_TAB && nm->code == TCN_SELCHANGE) {
                int tab = TabCtrl_GetCurSel(nm->hwndFrom);
                ShowAdminTab(hwnd, tab);
                if (tab == 0) RefreshAdAnnounceList(adAnList);
                if (tab == 1) RefreshAdReviewList(adRvList);
                if (tab == 2) { RefreshAdMajorList(adMjList); ListView_DeleteAllItems(adClList); }
            }
            if (nm->idFrom == ID_AD_AN_LIST && nm->code == LVN_ITEMCHANGED) {
                NMLISTVIEW* nmlv = (NMLISTVIEW*)lp;
                if (nmlv->uNewState & LVIS_SELECTED) {
                    char buf[32];
                    ListView_GetItemText(adAnList, nmlv->iItem, 0, buf, sizeof(buf));
                    Announcement* ann = g_dm->findAnnouncement(atoi(buf));
                    if (ann) {
                        SetWindowText(adAnTitle, ann->getTitle().c_str());
                        SetWindowText(adAnCont, ann->getContent().c_str());
                        SendMessage(adAnType, CB_SETCURSEL, ann->isAcceptedList()?1:0, 0);
                    }
                }
            }
            if (nm->idFrom == ID_AD_RV_LIST && nm->code == LVN_ITEMCHANGED) {
                NMLISTVIEW* nmlv = (NMLISTVIEW*)lp;
                if (nmlv->uNewState & LVIS_SELECTED) {
                    int idx = nmlv->iItem;
                    char buf[32];
                    ListView_GetItemText(adRvList, idx, 0, buf, sizeof(buf));
                    int id = atoi(buf);
                    Registration* reg = g_dm->findRegistration(id);
                    if (reg) {
                        std::string detail = std::string("ID: ") + buf + std::string("\r\n");
                        detail += "用户: " + reg->getUsername() + "\r\n";
                        Student* stu = dynamic_cast<Student*>(g_dm->findUser(reg->getUsername()));
                        if (stu) {
                            detail += "专业: " + stu->getMajor() + "  班级: " + stu->getClassName() + "  学号: " + stu->getStudentId() + "\r\n";
                        }
                        detail += "节目: " + reg->getProgramName() + "  类型: " + reg->getProgramType() + "  时长: ";
                        char dbuf[16]; sprintf(dbuf,"%d",reg->getDuration());
                        detail += dbuf + std::string("分钟\r\n");
                        detail += "表演者: " + reg->getPerformer() + "\r\n";
                        detail += "描述: " + reg->getDescription() + "\r\n";
                        detail += "材料: " + (reg->getMaterials().empty()?"(无)":reg->getMaterials()) + "\r\n";
                        detail += "状态: " + reg->getStatusDisplay() + "\r\n";
                        if (!reg->getReviewComment().empty()) detail += "审核意见: " + reg->getReviewComment();
                        SetWindowText(adRvDetail, detail.c_str());
                    }
                }
            }
            if (nm->idFrom == ID_AD_MJ_LIST && nm->code == LVN_ITEMCHANGED) {
                NMLISTVIEW* nmlv = (NMLISTVIEW*)lp;
                if (nmlv->uNewState & LVIS_SELECTED) {
                    char buf[256];
                    ListView_GetItemText(adMjList, nmlv->iItem, 0, buf, sizeof(buf));
                    RefreshAdClassList(adClList, buf);
                }
            }
            break;
        }
        case WM_COMMAND: {
            WORD id = LOWORD(wp);
            // Tab 0: 公告管理
            if (id == ID_AD_AN_ADD) {
                char title[256], cont[4096];
                GetWindowText(adAnTitle, title, sizeof(title));
                GetWindowText(adAnCont, cont, sizeof(cont));
                if (!title[0]) { MessageBox(hwnd,"请输入标题！","错误",MB_ICONERROR); break; }
                int t = SendMessage(adAnType, CB_GETCURSEL, 0, 0);
                g_dm->addAnnouncement(title, cont, user->getUsername(), t==1);
                RefreshAdAnnounceList(adAnList);
                MessageBox(hwnd,"公告发布成功！","提示",MB_ICONINFORMATION);
            } else if (id == ID_AD_AN_EDIT) {
                int sel = ListView_GetNextItem(adAnList, -1, LVNI_SELECTED);
                if (sel < 0) { MessageBox(hwnd,"请先选择公告！","提示",MB_ICONINFORMATION); break; }
                char buf[32], title[256], cont[4096];
                ListView_GetItemText(adAnList, sel, 0, buf, sizeof(buf));
                GetWindowText(adAnTitle, title, sizeof(title));
                GetWindowText(adAnCont, cont, sizeof(cont));
                int t = SendMessage(adAnType, CB_GETCURSEL, 0, 0);
                g_dm->updateAnnouncement(atoi(buf), title[0]?title:NULL, cont[0]?cont:NULL, t==1);
                RefreshAdAnnounceList(adAnList);
                MessageBox(hwnd,"公告修改成功！","提示",MB_ICONINFORMATION);
            } else if (id == ID_AD_AN_DEL) {
                int sel = ListView_GetNextItem(adAnList, -1, LVNI_SELECTED);
                if (sel < 0) { MessageBox(hwnd,"请先选择公告！","提示",MB_ICONINFORMATION); break; }
                char buf[32];
                ListView_GetItemText(adAnList, sel, 0, buf, sizeof(buf));
                if (MessageBox(hwnd,"确认删除该公告？","确认",MB_YESNO)==IDYES) {
                    g_dm->deleteAnnouncement(atoi(buf));
                    RefreshAdAnnounceList(adAnList);
                }
            }
            // Tab 1: 审核报名
            else if (id == ID_AD_RV_APPR || id == ID_AD_RV_REJ || id == ID_AD_RV_UNDO) {
                int sel = ListView_GetNextItem(adRvList, -1, LVNI_SELECTED);
                if (sel < 0) { MessageBox(hwnd,"请先选择报名！","提示",MB_ICONINFORMATION); break; }
                char buf[32];
                ListView_GetItemText(adRvList, sel, 0, buf, sizeof(buf));
                Registration* reg = g_dm->findRegistration(atoi(buf));
                if (!reg) break;
                if (id == ID_AD_RV_APPR) { reg->setStatus("approved"); reg->setReviewComment("审核通过"); }
                else if (id == ID_AD_RV_REJ) { reg->setStatus("rejected"); reg->setReviewComment("未通过审核"); }
                else { reg->setStatus("pending"); reg->setReviewComment(""); }
                g_dm->saveAll();
                RefreshAdReviewList(adRvList);
                MessageBox(hwnd, id==ID_AD_RV_APPR?"已通过！":id==ID_AD_RV_REJ?"已驳回！":"已撤销！", "提示", MB_ICONINFORMATION);
            }
            // Tab 2: 数据管理
            else if (id == ID_AD_MJ_ADD) {
                MessageBox(hwnd, "请在控制台版本中使用 /data 管理专业班级\n或直接编辑 data/majors.dat 和 data/classes.dat", "提示", MB_ICONINFORMATION);
            }
            // Tab 3: 修改密码
            else if (id == ID_AD_CHP_OK) {
                char old[128], n1[128], n2[128];
                GetWindowText(adChpOld, old, sizeof(old));
                GetWindowText(adChpNew, n1, sizeof(n1));
                GetWindowText(adChpNew2, n2, sizeof(n2));
                if (!user->checkPassword(old)) { MessageBox(hwnd,"旧密码错误！","错误",MB_ICONERROR); break; }
                if (strcmp(n1,n2)) { MessageBox(hwnd,"两次密码不一致！","错误",MB_ICONERROR); break; }
                if (strlen(n1)<4) { MessageBox(hwnd,"密码至少4位！","错误",MB_ICONERROR); break; }
                user->setPassword(n1);
                g_dm->saveAll();
                MessageBox(hwnd,"密码修改成功！","提示",MB_ICONINFORMATION);
            }
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

// ==================== WinMain ====================

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow) {
    g_hInst = hInst;
    INITCOMMONCONTROLSEX icc; icc.dwSize=sizeof(icc);
    icc.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    g_dm = new DataManager("data/");
    g_hFont = CreateFont(15,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,DEFAULT_PITCH,"微软雅黑");

    // Register window classes
    WNDCLASS wc; memset(&wc,0,sizeof(wc));
    wc.hInstance = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    wc.lpszClassName = "LoginWnd";
    wc.lpfnWndProc = LoginWndProc;
    RegisterClass(&wc);

    wc.lpszClassName = "StudentWnd";
    wc.lpfnWndProc = StudentWndProc;
    RegisterClass(&wc);

    wc.lpszClassName = "AdminWnd";
    wc.lpfnWndProc = AdminWndProc;
    RegisterClass(&wc);

    // 外层循环：登录 → 主窗口 → 注销 → 回到登录
    while (true) {
        g_loginResult = NULL;

        HWND hLogin = CreateWindowEx(WS_EX_DLGMODALFRAME, "LoginWnd",
            "我新我秀 报名系统 - 登录",
            WS_VISIBLE|WS_CAPTION|WS_SYSMENU,
            CW_USEDEFAULT, CW_USEDEFAULT, 380, 240,
            NULL, NULL, hInst, NULL);

        // 登录消息循环
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // GetMessage 返回 0 → PostQuitMessage 被调用（登录成功或退出）

        User* loggedUser = g_loginResult;
        if (!loggedUser) break;  // 退出

        // 打开对应主窗口
        const char* cls = (loggedUser->getRole() == "admin") ? "AdminWnd" : "StudentWnd";
        const char* title = (loggedUser->getRole() == "admin") ? "管理员 - 我新我秀" : "学生 - 我新我秀";
        int ww = (loggedUser->getRole() == "admin") ? 650 : 550;
        int wh = (loggedUser->getRole() == "admin") ? 430 : 380;

        HWND hMain = CreateWindowEx(0, cls, title,
            WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, ww, wh,
            NULL, NULL, hInst, (LPVOID)loggedUser);
        ShowWindow(hMain, nShow);
        UpdateWindow(hMain);

        // 主窗口消息循环
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // 主窗口关闭后，回到外层循环重新创建登录窗口
    }

    delete g_dm;
    DeleteObject(g_hFont);
    return 0;
}
