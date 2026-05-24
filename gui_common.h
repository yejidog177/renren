#ifndef GUI_COMMON_H
#define GUI_COMMON_H

#include <windows.h>
#include <commctrl.h>

// 控件ID定义
#define IDC_LOGIN_USERNAME     1001
#define IDC_LOGIN_PASSWORD     1002
#define IDC_LOGIN_BTN          1003
#define IDC_REGISTER_BTN       1004
#define IDC_EXIT_BTN           1005

#define IDC_REG_USERNAME       2001
#define IDC_REG_PASSWORD       2002
#define IDC_REG_PASSWORD2      2003
#define IDC_REG_MAJOR          2004
#define IDC_REG_CLASS          2005
#define IDC_REG_STUDENTID      2006
#define IDC_REG_OK             2007
#define IDC_REG_CANCEL         2008

// Tab control
#define IDC_TAB                3001

// Student tabs
#define IDC_STU_ANNOUNCE_LIST  3101
#define IDC_STU_ACCEPT_LIST    3102
#define IDC_STU_PROGTYPE       3103
#define IDC_STU_PROGNAME       3104
#define IDC_STU_DURATION       3105
#define IDC_STU_PERFORMER      3106
#define IDC_STU_DESC           3107
#define IDC_STU_SAVE           3108
#define IDC_STU_MATERIALS      3109
#define IDC_STU_MAT_SAVE       3110
#define IDC_STU_INFO_TEXT      3111
#define IDC_STU_CHPWD_OLD      3112
#define IDC_STU_CHPWD_NEW      3113
#define IDC_STU_CHPWD_NEW2     3114
#define IDC_STU_CHPWD_OK       3115

// Admin tabs
#define IDC_ADM_ANNOUNCE_LIST  3201
#define IDC_ADM_AN_TITLE       3202
#define IDC_ADM_AN_CONTENT     3203
#define IDC_ADM_AN_TYPE        3204
#define IDC_ADM_AN_ADD         3205
#define IDC_ADM_AN_EDIT        3206
#define IDC_ADM_AN_DEL         3207
#define IDC_ADM_REVIEW_LIST    3208
#define IDC_ADM_REVIEW_DETAIL  3209
#define IDC_ADM_REVIEW_APPROVE 3210
#define IDC_ADM_REVIEW_REJECT  3211
#define IDC_ADM_REVIEW_UNDO    3212
#define IDC_ADM_MAJOR_LIST     3213
#define IDC_ADM_CLASS_LIST     3214
#define IDC_ADM_MAJOR_ADD      3215
#define IDC_ADM_MAJOR_DEL      3216
#define IDC_ADM_CLASS_ADD      3217
#define IDC_ADM_CLASS_DEL      3218
#define IDC_ADM_CHPWD_OLD      3219
#define IDC_ADM_CHPWD_NEW      3220
#define IDC_ADM_CHPWD_NEW2     3221
#define IDC_ADM_CHPWD_OK       3222

// 从数据层获取全局 DataManager
class DataManager;
extern DataManager* g_dm;
extern const char* g_currentUser;

// 辅助函数
void AddListColumn(HWND hList, int idx, const char* text, int width);
void RefreshAnnounceList(HWND hList, DataManager* dm);
void RefreshAcceptList(HWND hList, DataManager* dm);
void RefreshReviewList(HWND hList, DataManager* dm);
void RefreshMajorList(HWND hList, DataManager* dm);
void RefreshClassList(HWND hList, DataManager* dm, const char* major);

// 对话框
BOOL CALLBACK LoginDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK RegisterDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

// 主窗口
BOOL CALLBACK StudentWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK AdminWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

#endif
