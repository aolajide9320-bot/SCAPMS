#include "MainWindow.h"
#include "PatientDialog.h"
#include "AppointmentDialog.h"
#include "../model/GeneralPatient.h"
#include "../model/EmergencyPatient.h"
#include "../model/FollowUpPatient.h"
#include "../utils/FileHandler.h"
#include <commctrl.h>
#include <windowsx.h>
#include <sstream>
#include <iomanip>
#include <ctime>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comdlg32.lib")

namespace scapms {
namespace gui {

namespace {
MainWindow* g_pMainWindow = nullptr;

// Helper to get selected index in a ListView
int getListViewSelectedIndex(HWND hList) {
    return (int)SendMessage(hList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
}

// Helper to get text from a ListView cell
std::string getListViewItemText(HWND hList, int item, int subItem) {
    char buf[256] = {0};
    LVITEMA lvi;
    lvi.iSubItem = subItem;
    lvi.pszText = buf;
    lvi.cchTextMax = sizeof(buf);
    SendMessage(hList, LVM_GETITEMTEXTA, (WPARAM)item, (LPARAM)&lvi);
    return std::string(buf);
}
} // namespace

MainWindow::MainWindow() {
    g_pMainWindow = this;
}

MainWindow::~MainWindow() {
    if (m_hFont) DeleteObject(m_hFont);
    if (m_hBoldFont) DeleteObject(m_hBoldFont);
    if (m_hTitleFont) DeleteObject(m_hTitleFont);
    if (m_hAccel) DestroyAcceleratorTable(m_hAccel);
}

bool MainWindow::initialize(HINSTANCE hInstance, int nCmdShow) {
    // Initialize Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // Register Main Window Class
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindow::WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "SCAPMS_MainWindowClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassExA(&wc);

    // Register Panel Class for child containers
    WNDCLASSEXA wcPanel = {0};
    wcPanel.cbSize = sizeof(WNDCLASSEXA);
    wcPanel.style = CS_HREDRAW | CS_VREDRAW;
    wcPanel.lpfnWndProc = MainWindow::PanelSubclassProc;
    wcPanel.hInstance = hInstance;
    wcPanel.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcPanel.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcPanel.lpszClassName = "SCAPMS_PanelClass";
    RegisterClassExA(&wcPanel);

    // Create Main Window
    m_hwndMain = CreateWindowExA(
        0,
        "SCAPMS_MainWindowClass",
        "Smart Clinic Appointment & Patient Management System (SCAPMS)",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 720,
        NULL, NULL, hInstance, NULL
    );

    if (!m_hwndMain) return false;

    // Create Segoe UI typography fonts
    m_hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

    m_hBoldFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

    m_hTitleFont = CreateFontA(20, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

    // Create Keyboard Accelerators (Mandatory Section 9.f: Ctrl+S, Ctrl+F, Ctrl+Q, Ctrl+Z)
    ACCEL accels[4];
    accels[0].fVirt = FCONTROL | FVIRTKEY; accels[0].key = 'S'; accels[0].cmd = IDA_SAVE;
    accels[1].fVirt = FCONTROL | FVIRTKEY; accels[1].key = 'F'; accels[1].cmd = IDA_SEARCH;
    accels[2].fVirt = FCONTROL | FVIRTKEY; accels[2].key = 'Q'; accels[2].cmd = IDA_EXIT;
    accels[3].fVirt = FCONTROL | FVIRTKEY; accels[3].key = 'Z'; accels[3].cmd = IDA_UNDO;
    m_hAccel = CreateAcceleratorTableA(accels, 4);

    createMenuBar();
    createStatusBar();
    createTabs();

    // Create Panels
    createDashboardPanel();
    createPatientPanel();
    createAppointmentPanel();
    createQueuePanel();
    createSearchPanel();
    createReportPanel();

    // Setup periodic Timers (Mandatory Section 6.d, 9.d)
    // 1-second clock timer for status bar and reminder checks
    SetTimer(m_hwndMain, TIMER_CLOCK_ID, 1000, NULL);
    // 5-second dashboard refresh timer
    SetTimer(m_hwndMain, TIMER_DASHBOARD_ID, 5000, NULL);

    // Initial display
    onTabChanged(0);
    refreshAllViews();

    ShowWindow(m_hwndMain, nCmdShow);
    UpdateWindow(m_hwndMain);

    return true;
}

int MainWindow::runMessageLoop() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(m_hwndMain, m_hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

void MainWindow::createMenuBar() {
    HMENU hMenuBar = CreateMenu();

    // File Menu
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuA(hFileMenu, MF_STRING, IDM_FILE_NEW, "&New Database\tCtrl+N");
    AppendMenuA(hFileMenu, MF_STRING, IDM_FILE_OPEN, "&Open Database (CSV)...\tCtrl+O");
    AppendMenuA(hFileMenu, MF_STRING, IDM_FILE_SAVE, "&Save Database\tCtrl+S");
    AppendMenuA(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hFileMenu, MF_STRING, IDM_FILE_EXPORT, "&Export Executive Report...");
    AppendMenuA(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hFileMenu, MF_STRING, IDM_FILE_EXIT, "E&xit\tCtrl+Q");
    AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, "&File");

    // Edit Menu
    HMENU hEditMenu = CreatePopupMenu();
    AppendMenuA(hEditMenu, MF_STRING, IDM_EDIT_UNDO, "&Undo Last Administrative Action\tCtrl+Z");
    AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hEditMenu, "&Edit");

    // View Menu
    HMENU hViewMenu = CreatePopupMenu();
    AppendMenuA(hViewMenu, MF_STRING, IDM_VIEW_REFRESH, "&Refresh Dashboard && Tables\tF5");
    AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hViewMenu, "&View");

    // Help Menu
    HMENU hHelpMenu = CreatePopupMenu();
    AppendMenuA(hHelpMenu, MF_STRING, IDM_HELP_ABOUT, "&About SCAPMS");
    AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hHelpMenu, "&Help");

    SetMenu(m_hwndMain, hMenuBar);
}

void MainWindow::createStatusBar() {
    m_hStatusBar = CreateWindowExA(
        0, STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        m_hwndMain, (HMENU)(UINT_PTR)ID_STATUS_BAR, GetModuleHandle(NULL), NULL
    );

    int statwidths[] = { 320, 580, 780, -1 };
    SendMessage(m_hStatusBar, SB_SETPARTS, sizeof(statwidths)/sizeof(int), (LPARAM)statwidths);
    SendMessageA(m_hStatusBar, SB_SETTEXTA, 0, (LPARAM)" System Status: Ready | University Clinic Online");
    SendMessageA(m_hStatusBar, SB_SETTEXTA, 1, (LPARAM)" Undo: No pending operation");
    SendMessageA(m_hStatusBar, SB_SETTEXTA, 2, (LPARAM)" Queue: 0 waiting");
    SendMessageA(m_hStatusBar, SB_SETTEXTA, 3, (LPARAM)" 2026-11-06");
}

void MainWindow::createTabs() {
    m_hTabControl = CreateWindowExA(
        0, WC_TABCONTROL, "",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 1000, 600,
        m_hwndMain, (HMENU)(UINT_PTR)ID_MAIN_TAB, GetModuleHandle(NULL), NULL
    );
    SendMessage(m_hTabControl, WM_SETFONT, (WPARAM)m_hFont, TRUE);

    const char* tabTitles[] = {
        "  Dashboard & Analytics  ",
        "  Patient Records  ",
        "  Appointments  ",
        "  Waiting Queue (FIFO)  ",
        "  Search & Sorting  ",
        "  Clinical Reports  "
    };

    TCITEMA tie;
    tie.mask = TCIF_TEXT;
    for (int i = 0; i < 6; ++i) {
        tie.pszText = const_cast<char*>(tabTitles[i]);
        TabCtrl_InsertItem(m_hTabControl, i, &tie);
    }
}

void MainWindow::setControlFont(HWND hwndControl, HFONT hFont) {
    SendMessage(hwndControl, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void MainWindow::addListViewColumn(HWND hList, int colIndex, const char* text, int width) {
    LVCOLUMNA lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lvc.iSubItem = colIndex;
    lvc.pszText = const_cast<char*>(text);
    lvc.cx = width;
    ListView_InsertColumn(hList, colIndex, &lvc);
}

// -------------------------------------------------------------
// Panel 0: Dashboard & Overview
// -------------------------------------------------------------
void MainWindow::createDashboardPanel() {
    HINSTANCE hInst = GetModuleHandle(NULL);
    HWND p = CreateWindowExA(0, "SCAPMS_PanelClass", "", WS_CHILD, 0, 0, 800, 500, m_hwndMain, NULL, hInst, NULL);
    m_panels[0] = p;

    // KPI Cards (Labels displaying statistics)
    for (int i = 0; i < 5; ++i) {
        m_hDashKpiCards[i] = CreateWindowExA(
            WS_EX_CLIENTEDGE, "STATIC", "",
            WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
            0, 0, 100, 100, p, NULL, hInst, NULL
        );
        setControlFont(m_hDashKpiCards[i], m_hBoldFont);
    }

    // Recent Patients Cache (Mandatory Req 3.d: PatientRecord* recentPatients[10])
    HWND hCacheLabel = CreateWindowExA(0, "STATIC", "Frequently Accessed Patient Records (Fixed-Size C++ Array Cache [10]):",
                                       WS_CHILD | WS_VISIBLE, 0, 0, 100, 20, p, NULL, hInst, NULL);
    setControlFont(hCacheLabel, m_hBoldFont);

    m_hDashRecentList = CreateWindowExA(
        WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        0, 0, 100, 100, p, (HMENU)(UINT_PTR)IDC_RECENT_CACHE_LIST, hInst, NULL
    );
    ListView_SetExtendedListViewStyle(m_hDashRecentList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    setControlFont(m_hDashRecentList, m_hFont);

    addListViewColumn(m_hDashRecentList, 0, "Cache Slot", 80);
    addListViewColumn(m_hDashRecentList, 1, "Patient ID", 100);
    addListViewColumn(m_hDashRecentList, 2, "Full Name", 180);
    addListViewColumn(m_hDashRecentList, 3, "Category", 120);
    addListViewColumn(m_hDashRecentList, 4, "Age", 60);
    addListViewColumn(m_hDashRecentList, 5, "Contact", 140);
    addListViewColumn(m_hDashRecentList, 6, "Fee Rate", 90);

    // Alert Banner
    m_hDashAlertText = CreateWindowExA(
        WS_EX_STATICEDGE, "STATIC", " Alerts: Clinic operation normal. No emergency backlog.",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
        0, 0, 100, 30, p, NULL, hInst, NULL
    );
    setControlFont(m_hDashAlertText, m_hBoldFont);

    // Quick Action Buttons
    m_hDashBtnAddPat = CreateWindowExA(0, "BUTTON", "+ Register Patient", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       0, 0, 120, 32, p, (HMENU)(UINT_PTR)IDC_BTN_DASH_ADD_PAT, hInst, NULL);
    m_hDashBtnAddApt = CreateWindowExA(0, "BUTTON", "+ Book Appointment", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       0, 0, 120, 32, p, (HMENU)(UINT_PTR)IDC_BTN_DASH_ADD_APT, hInst, NULL);
    m_hDashBtnCallNext = CreateWindowExA(0, "BUTTON", ">>> Call Next Patient", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                         0, 0, 140, 32, p, (HMENU)(UINT_PTR)IDC_BTN_DASH_CALL_NEXT, hInst, NULL);
    setControlFont(m_hDashBtnAddPat, m_hBoldFont);
    setControlFont(m_hDashBtnAddApt, m_hBoldFont);
    setControlFont(m_hDashBtnCallNext, m_hBoldFont);
}

// -------------------------------------------------------------
// Panel 1: Patient Records
// -------------------------------------------------------------
void MainWindow::createPatientPanel() {
    HINSTANCE hInst = GetModuleHandle(NULL);
    HWND p = CreateWindowExA(0, "SCAPMS_PanelClass", "", WS_CHILD, 0, 0, 800, 500, m_hwndMain, NULL, hInst, NULL);
    m_panels[1] = p;

    // Filter label and box
    HWND hLbl = CreateWindowExA(0, "STATIC", "Quick Filter:", WS_CHILD | WS_VISIBLE, 0, 0, 80, 24, p, NULL, hInst, NULL);
    setControlFont(hLbl, m_hFont);

    m_hPatientFilterEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        0, 0, 200, 24, p, (HMENU)(UINT_PTR)IDC_PATIENT_FILTER_EDIT, hInst, NULL
    );
    setControlFont(m_hPatientFilterEdit, m_hFont);

    // Table / List View
    m_hPatientList = CreateWindowExA(
        WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0, 100, 100, p, (HMENU)(UINT_PTR)IDC_PATIENT_LIST, hInst, NULL
    );
    ListView_SetExtendedListViewStyle(m_hPatientList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    setControlFont(m_hPatientList, m_hFont);

    addListViewColumn(m_hPatientList, 0, "ID", 90);
    addListViewColumn(m_hPatientList, 1, "Patient Name", 170);
    addListViewColumn(m_hPatientList, 2, "Category (Type)", 150);
    addListViewColumn(m_hPatientList, 3, "Age", 55);
    addListViewColumn(m_hPatientList, 4, "Contact Phone", 130);
    addListViewColumn(m_hPatientList, 5, "Registration", 100);
    addListViewColumn(m_hPatientList, 6, "Fee", 75);
    addListViewColumn(m_hPatientList, 7, "Clinical Details & Attributes", 280);

    // Action Buttons
    m_hBtnAddPatient = CreateWindowExA(0, "BUTTON", "+ Add Patient", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                       0, 0, 110, 30, p, (HMENU)(UINT_PTR)IDC_BTN_ADD_PATIENT, hInst, NULL);
    m_hBtnEditPatient = CreateWindowExA(0, "BUTTON", "Edit Patient", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                        0, 0, 110, 30, p, (HMENU)(UINT_PTR)IDC_BTN_EDIT_PATIENT, hInst, NULL);
    m_hBtnDeletePatient = CreateWindowExA(0, "BUTTON", "Delete Patient", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                          0, 0, 110, 30, p, (HMENU)(UINT_PTR)IDC_BTN_DELETE_PATIENT, hInst, NULL);
    m_hBtnQueuePatient = CreateWindowExA(0, "BUTTON", "Add to Queue", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                         0, 0, 110, 30, p, (HMENU)(UINT_PTR)IDC_BTN_QUEUE_PATIENT, hInst, NULL);
    m_hBtnUndoPatient = CreateWindowExA(0, "BUTTON", "Undo Action", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                        0, 0, 110, 30, p, (HMENU)(UINT_PTR)IDC_BTN_UNDO_PATIENT, hInst, NULL);

    setControlFont(m_hBtnAddPatient, m_hBoldFont);
    setControlFont(m_hBtnEditPatient, m_hFont);
    setControlFont(m_hBtnDeletePatient, m_hFont);
    setControlFont(m_hBtnQueuePatient, m_hFont);
    setControlFont(m_hBtnUndoPatient, m_hFont);
}

// -------------------------------------------------------------
// Panel 2: Appointments
// -------------------------------------------------------------
void MainWindow::createAppointmentPanel() {
    HINSTANCE hInst = GetModuleHandle(NULL);
    HWND p = CreateWindowExA(0, "SCAPMS_PanelClass", "", WS_CHILD, 0, 0, 800, 500, m_hwndMain, NULL, hInst, NULL);
    m_panels[2] = p;

    m_hAppointmentList = CreateWindowExA(
        WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0, 100, 100, p, (HMENU)(UINT_PTR)IDC_APPOINTMENT_LIST, hInst, NULL
    );
    ListView_SetExtendedListViewStyle(m_hAppointmentList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    setControlFont(m_hAppointmentList, m_hFont);

    addListViewColumn(m_hAppointmentList, 0, "Apt ID", 90);
    addListViewColumn(m_hAppointmentList, 1, "Patient ID", 90);
    addListViewColumn(m_hAppointmentList, 2, "Patient Name", 170);
    addListViewColumn(m_hAppointmentList, 3, "Date", 95);
    addListViewColumn(m_hAppointmentList, 4, "Time", 70);
    addListViewColumn(m_hAppointmentList, 5, "Department", 130);
    addListViewColumn(m_hAppointmentList, 6, "Doctor", 120);
    addListViewColumn(m_hAppointmentList, 7, "Status", 100);
    addListViewColumn(m_hAppointmentList, 8, "Fee", 70);
    addListViewColumn(m_hAppointmentList, 9, "Clinical Notes", 220);

    m_hBtnScheduleApt = CreateWindowExA(0, "BUTTON", "+ Schedule Appointment", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                        0, 0, 150, 30, p, (HMENU)(UINT_PTR)IDC_BTN_SCHEDULE_APPOINTMENT, hInst, NULL);
    m_hBtnCompleteApt = CreateWindowExA(0, "BUTTON", "Mark Completed", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                        0, 0, 130, 30, p, (HMENU)(UINT_PTR)IDC_BTN_COMPLETE_APPOINTMENT, hInst, NULL);
    m_hBtnCancelApt = CreateWindowExA(0, "BUTTON", "Cancel Appointment", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                      0, 0, 140, 30, p, (HMENU)(UINT_PTR)IDC_BTN_CANCEL_APPOINTMENT, hInst, NULL);
    m_hBtnDeleteApt = CreateWindowExA(0, "BUTTON", "Delete Appointment", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                      0, 0, 140, 30, p, (HMENU)(UINT_PTR)IDC_BTN_DELETE_APPOINTMENT, hInst, NULL);
    m_hBtnUndoApt = CreateWindowExA(0, "BUTTON", "Undo", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    0, 0, 90, 30, p, (HMENU)(UINT_PTR)IDC_BTN_UNDO_APPOINTMENT, hInst, NULL);

    setControlFont(m_hBtnScheduleApt, m_hBoldFont);
    setControlFont(m_hBtnCompleteApt, m_hFont);
    setControlFont(m_hBtnCancelApt, m_hFont);
    setControlFont(m_hBtnDeleteApt, m_hFont);
    setControlFont(m_hBtnUndoApt, m_hFont);
}

// -------------------------------------------------------------
// Panel 3: Waiting Queue (std::queue)
// -------------------------------------------------------------
void MainWindow::createQueuePanel() {
    HINSTANCE hInst = GetModuleHandle(NULL);
    HWND p = CreateWindowExA(0, "SCAPMS_PanelClass", "", WS_CHILD, 0, 0, 800, 500, m_hwndMain, NULL, hInst, NULL);
    m_panels[3] = p;

    // Currently Serving Banner
    m_hQueueBanner = CreateWindowExA(
        WS_EX_CLIENTEDGE, "STATIC", " CURRENTLY SERVING: None (Clinic In Consultation Room 1)",
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
        0, 0, 100, 50, p, (HMENU)(UINT_PTR)IDC_STATIC_NOW_SERVING, hInst, NULL
    );
    setControlFont(m_hQueueBanner, m_hTitleFont);

    // Waiting queue table
    m_hQueueList = CreateWindowExA(
        WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        0, 0, 100, 100, p, (HMENU)(UINT_PTR)IDC_QUEUE_LIST, hInst, NULL
    );
    ListView_SetExtendedListViewStyle(m_hQueueList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    setControlFont(m_hQueueList, m_hFont);

    addListViewColumn(m_hQueueList, 0, "Position", 80);
    addListViewColumn(m_hQueueList, 1, "Patient ID", 110);
    addListViewColumn(m_hQueueList, 2, "Patient Name", 200);
    addListViewColumn(m_hQueueList, 3, "Category", 140);
    addListViewColumn(m_hQueueList, 4, "Age", 60);
    addListViewColumn(m_hQueueList, 5, "Contact", 150);
    addListViewColumn(m_hQueueList, 6, "Status / Triage Priority", 200);

    m_hBtnCallNextQueue = CreateWindowExA(0, "BUTTON", ">>> CALL NEXT PATIENT (FIFO) <<<",
                                          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                          0, 0, 240, 36, p, (HMENU)(UINT_PTR)IDC_BTN_CALL_NEXT, hInst, NULL);
    m_hBtnAddSelectedToQueue = CreateWindowExA(0, "BUTTON", "+ Enqueue Registered Patient",
                                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                               0, 0, 200, 36, p, (HMENU)(UINT_PTR)IDC_BTN_ENQUEUE_SELECTED, hInst, NULL);

    setControlFont(m_hBtnCallNextQueue, m_hBoldFont);
    setControlFont(m_hBtnAddSelectedToQueue, m_hFont);
}

// -------------------------------------------------------------
// Panel 4: Search & Sorting (Student Algorithms)
// -------------------------------------------------------------
void MainWindow::createSearchPanel() {
    HINSTANCE hInst = GetModuleHandle(NULL);
    HWND p = CreateWindowExA(0, "SCAPMS_PanelClass", "", WS_CHILD, 0, 0, 800, 500, m_hwndMain, NULL, hInst, NULL);
    m_panels[4] = p;

    // Search controls
    HWND hLblQuery = CreateWindowExA(0, "STATIC", "Search Query:", WS_CHILD | WS_VISIBLE, 0, 0, 90, 22, p, NULL, hInst, NULL);
    setControlFont(hLblQuery, m_hFont);

    m_hSearchEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "PAT-1002", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                    0, 0, 140, 24, p, (HMENU)(UINT_PTR)IDC_SEARCH_EDIT, hInst, NULL);
    setControlFont(m_hSearchEdit, m_hFont);

    HWND hLblAlgo = CreateWindowExA(0, "STATIC", "Algorithm:", WS_CHILD | WS_VISIBLE, 0, 0, 70, 22, p, NULL, hInst, NULL);
    setControlFont(hLblAlgo, m_hFont);

    m_hComboSearchAlgo = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                                         0, 0, 180, 150, p, (HMENU)(UINT_PTR)IDC_COMBO_SEARCH_ALGO, hInst, NULL);
    setControlFont(m_hComboSearchAlgo, m_hFont);
    SendMessageA(m_hComboSearchAlgo, CB_ADDSTRING, 0, (LPARAM)"Linear Search (by ID)");
    SendMessageA(m_hComboSearchAlgo, CB_ADDSTRING, 0, (LPARAM)"Binary Search (by ID/Name)");
    SendMessageA(m_hComboSearchAlgo, CB_ADDSTRING, 0, (LPARAM)"Recursive Search (by Name)");
    SendMessageA(m_hComboSearchAlgo, CB_SETCURSEL, 0, 0);

    HWND hLblField = CreateWindowExA(0, "STATIC", "Field:", WS_CHILD | WS_VISIBLE, 0, 0, 40, 22, p, NULL, hInst, NULL);
    setControlFont(hLblField, m_hFont);

    m_hComboSearchField = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                                          0, 0, 90, 120, p, (HMENU)(UINT_PTR)IDC_COMBO_SEARCH_FIELD, hInst, NULL);
    setControlFont(m_hComboSearchField, m_hFont);
    SendMessageA(m_hComboSearchField, CB_ADDSTRING, 0, (LPARAM)"ID");
    SendMessageA(m_hComboSearchField, CB_ADDSTRING, 0, (LPARAM)"Name");
    SendMessageA(m_hComboSearchField, CB_SETCURSEL, 0, 0);

    m_hBtnExecuteSearch = CreateWindowExA(0, "BUTTON", "Execute Search", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                          0, 0, 110, 26, p, (HMENU)(UINT_PTR)IDC_BTN_DO_SEARCH, hInst, NULL);
    setControlFont(m_hBtnExecuteSearch, m_hBoldFont);

    // Sorting controls
    HWND hLblSortAlgo = CreateWindowExA(0, "STATIC", "Sort Algorithm:", WS_CHILD | WS_VISIBLE, 0, 0, 95, 22, p, NULL, hInst, NULL);
    setControlFont(hLblSortAlgo, m_hFont);

    m_hComboSortAlgo = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                                       0, 0, 160, 150, p, (HMENU)(UINT_PTR)IDC_COMBO_SORT_ALGO, hInst, NULL);
    setControlFont(m_hComboSortAlgo, m_hFont);
    SendMessageA(m_hComboSortAlgo, CB_ADDSTRING, 0, (LPARAM)"Quick Sort");
    SendMessageA(m_hComboSortAlgo, CB_ADDSTRING, 0, (LPARAM)"Selection Sort");
    SendMessageA(m_hComboSortAlgo, CB_ADDSTRING, 0, (LPARAM)"Merge Sort");
    SendMessageA(m_hComboSortAlgo, CB_SETCURSEL, 0, 0);

    HWND hLblSortField = CreateWindowExA(0, "STATIC", "Sort By:", WS_CHILD | WS_VISIBLE, 0, 0, 55, 22, p, NULL, hInst, NULL);
    setControlFont(hLblSortField, m_hFont);

    m_hComboSortField = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                                        0, 0, 130, 150, p, (HMENU)(UINT_PTR)IDC_COMBO_SORT_FIELD, hInst, NULL);
    setControlFont(m_hComboSortField, m_hFont);
    SendMessageA(m_hComboSortField, CB_ADDSTRING, 0, (LPARAM)"Patient Name");
    SendMessageA(m_hComboSortField, CB_ADDSTRING, 0, (LPARAM)"Patient ID");
    SendMessageA(m_hComboSortField, CB_ADDSTRING, 0, (LPARAM)"Age");
    SendMessageA(m_hComboSortField, CB_ADDSTRING, 0, (LPARAM)"Registration Date");
    SendMessageA(m_hComboSortField, CB_SETCURSEL, 0, 0);

    m_hComboSortOrder = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                                        0, 0, 100, 100, p, (HMENU)(UINT_PTR)IDC_COMBO_SORT_ORDER, hInst, NULL);
    setControlFont(m_hComboSortOrder, m_hFont);
    SendMessageA(m_hComboSortOrder, CB_ADDSTRING, 0, (LPARAM)"Ascending");
    SendMessageA(m_hComboSortOrder, CB_ADDSTRING, 0, (LPARAM)"Descending");
    SendMessageA(m_hComboSortOrder, CB_SETCURSEL, 0, 0);

    m_hBtnExecuteSort = CreateWindowExA(0, "BUTTON", "Execute Sort", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                        0, 0, 110, 26, p, (HMENU)(UINT_PTR)IDC_BTN_DO_SORT, hInst, NULL);
    setControlFont(m_hBtnExecuteSort, m_hBoldFont);

    // Performance Metrics Display
    m_hSearchMetricsText = CreateWindowExA(
        WS_EX_STATICEDGE, "STATIC", " Algorithm Diagnostics: Ready to benchmark searching and sorting.",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
        0, 0, 100, 28, p, (HMENU)(UINT_PTR)IDC_STATIC_METRICS, hInst, NULL
    );
    setControlFont(m_hSearchMetricsText, m_hBoldFont);

    // Results Table
    m_hSearchResultsList = CreateWindowExA(
        WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        0, 0, 100, 100, p, (HMENU)(UINT_PTR)IDC_SEARCH_RESULTS_LIST, hInst, NULL
    );
    ListView_SetExtendedListViewStyle(m_hSearchResultsList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    setControlFont(m_hSearchResultsList, m_hFont);

    addListViewColumn(m_hSearchResultsList, 0, "ID", 90);
    addListViewColumn(m_hSearchResultsList, 1, "Patient Name", 180);
    addListViewColumn(m_hSearchResultsList, 2, "Category", 130);
    addListViewColumn(m_hSearchResultsList, 3, "Age", 55);
    addListViewColumn(m_hSearchResultsList, 4, "Contact", 130);
    addListViewColumn(m_hSearchResultsList, 5, "Registration", 100);
    addListViewColumn(m_hSearchResultsList, 6, "Fee Rate", 80);
    addListViewColumn(m_hSearchResultsList, 7, "Details", 240);
}

// -------------------------------------------------------------
// Panel 5: Clinical Reports
// -------------------------------------------------------------
void MainWindow::createReportPanel() {
    HINSTANCE hInst = GetModuleHandle(NULL);
    HWND p = CreateWindowExA(0, "SCAPMS_PanelClass", "", WS_CHILD, 0, 0, 800, 500, m_hwndMain, NULL, hInst, NULL);
    m_panels[5] = p;

    HWND hLbl = CreateWindowExA(0, "STATIC", "Select Clinical Report:", WS_CHILD | WS_VISIBLE, 0, 0, 140, 24, p, NULL, hInst, NULL);
    setControlFont(hLbl, m_hBoldFont);

    m_hComboReportType = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                                         0, 0, 320, 180, p, (HMENU)(UINT_PTR)IDC_COMBO_REPORT_TYPE, hInst, NULL);
    setControlFont(m_hComboReportType, m_hFont);
    SendMessageA(m_hComboReportType, CB_ADDSTRING, 0, (LPARAM)"1. Most Frequently Attended Patients");
    SendMessageA(m_hComboReportType, CB_ADDSTRING, 0, (LPARAM)"2. Patients with Pending Appointments");
    SendMessageA(m_hComboReportType, CB_ADDSTRING, 0, (LPARAM)"3. Patient Category Distribution");
    SendMessageA(m_hComboReportType, CB_ADDSTRING, 0, (LPARAM)"4. Daily Appointment Summary");
    SendMessageA(m_hComboReportType, CB_ADDSTRING, 0, (LPARAM)"5. Full Executive Clinic Summary Report");
    SendMessageA(m_hComboReportType, CB_SETCURSEL, 0, 0);

    m_hBtnGenerateReport = CreateWindowExA(0, "BUTTON", "Generate Report", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                           0, 0, 130, 26, p, (HMENU)(UINT_PTR)IDC_BTN_GENERATE_REPORT, hInst, NULL);
    m_hBtnExportReport = CreateWindowExA(0, "BUTTON", "Export to File...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                         0, 0, 120, 26, p, (HMENU)(UINT_PTR)IDC_BTN_EXPORT_REPORT, hInst, NULL);

    setControlFont(m_hBtnGenerateReport, m_hBoldFont);
    setControlFont(m_hBtnExportReport, m_hFont);

    m_hReportTextView = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY | WS_VSCROLL,
        0, 0, 100, 100, p, (HMENU)(UINT_PTR)IDC_REPORT_TEXT_VIEW, hInst, NULL
    );
    // Fixed pitch font for formatted text report tables
    HFONT hFixedFont = CreateFontA(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                   ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    SendMessage(m_hReportTextView, WM_SETFONT, (WPARAM)hFixedFont, TRUE);
}

// -------------------------------------------------------------
// Layout Management Resizing (Mandatory Section 8)
// -------------------------------------------------------------
void MainWindow::onResize(int width, int height) {
    if (!m_hTabControl || !m_hStatusBar) return;

    // Send resize to status bar
    SendMessage(m_hStatusBar, WM_SIZE, 0, 0);

    RECT rcStatus;
    GetWindowRect(m_hStatusBar, &rcStatus);
    int statusHeight = rcStatus.bottom - rcStatus.top;

    int tabH = height - statusHeight;
    MoveWindow(m_hTabControl, 0, 0, width, tabH, TRUE);

    // Compute display area inside tab control
    RECT rcTab;
    SetRect(&rcTab, 0, 0, width, tabH);
    TabCtrl_AdjustRect(m_hTabControl, FALSE, &rcTab);

    int panelX = rcTab.left;
    int panelY = rcTab.top;
    int panelW = rcTab.right - rcTab.left;
    int panelH = rcTab.bottom - rcTab.top;

    // Reposition active panels
    for (int i = 0; i < 6; ++i) {
        if (m_panels[i]) {
            MoveWindow(m_panels[i], panelX, panelY, panelW, panelH, TRUE);
        }
    }

    // Call individual panel layout managers
    layoutDashboardPanel(panelW, panelH);
    layoutPatientPanel(panelW, panelH);
    layoutAppointmentPanel(panelW, panelH);
    layoutQueuePanel(panelW, panelH);
    layoutSearchPanel(panelW, panelH);
    layoutReportPanel(panelW, panelH);
}

void MainWindow::layoutDashboardPanel(int w, int h) {
    // Row 1: KPI Cards across top (GridLayout 1x5)
    int cardH = 70;
    int cardSpacing = 10;
    int cardW = (w - 20 - 4 * cardSpacing) / 5;
    for (int i = 0; i < 5; ++i) {
        MoveWindow(m_hDashKpiCards[i], 10 + i * (cardW + cardSpacing), 10, cardW, cardH, TRUE);
    }

    // Row 2: Alerts banner and Quick Action buttons (HBoxLayout)
    int row2Y = 10 + cardH + 12;
    MoveWindow(m_hDashAlertText, 10, row2Y, w - 460, 32, TRUE);
    MoveWindow(m_hDashBtnAddPat, w - 440, row2Y, 130, 32, TRUE);
    MoveWindow(m_hDashBtnAddApt, w - 300, row2Y, 130, 32, TRUE);
    MoveWindow(m_hDashBtnCallNext, w - 160, row2Y, 150, 32, TRUE);

    // Row 3: Cache Label and ListView (VBoxLayout)
    int listY = row2Y + 44;
    int listH = std::max(100, h - listY - 10);
    MoveWindow(m_hDashRecentList, 10, listY, w - 20, listH, TRUE);
}

void MainWindow::layoutPatientPanel(int w, int h) {
    // Top Bar: Filter Edit
    MoveWindow(GetDlgItem(m_panels[1], 0), 10, 10, 80, 24, TRUE);
    MoveWindow(m_hPatientFilterEdit, 95, 10, 220, 24, TRUE);

    // Bottom Bar: Action buttons
    int btnY = h - 42;
    int btnW = 120;
    int btnSpacing = 8;
    MoveWindow(m_hBtnAddPatient, 10, btnY, btnW, 32, TRUE);
    MoveWindow(m_hBtnEditPatient, 10 + (btnW + btnSpacing), btnY, btnW, 32, TRUE);
    MoveWindow(m_hBtnDeletePatient, 10 + 2 * (btnW + btnSpacing), btnY, btnW, 32, TRUE);
    MoveWindow(m_hBtnQueuePatient, 10 + 3 * (btnW + btnSpacing), btnY, btnW + 10, 32, TRUE);
    MoveWindow(m_hBtnUndoPatient, 10 + 4 * (btnW + btnSpacing) + 10, btnY, btnW, 32, TRUE);

    // Center: List View
    int listH = btnY - 46;
    MoveWindow(m_hPatientList, 10, 42, w - 20, listH, TRUE);
}

void MainWindow::layoutAppointmentPanel(int w, int h) {
    int btnY = h - 42;
    int btnW = 150;
    int spacing = 8;
    MoveWindow(m_hBtnScheduleApt, 10, btnY, 160, 32, TRUE);
    MoveWindow(m_hBtnCompleteApt, 10 + 160 + spacing, btnY, 140, 32, TRUE);
    MoveWindow(m_hBtnCancelApt, 10 + 300 + 2 * spacing, btnY, 140, 32, TRUE);
    MoveWindow(m_hBtnDeleteApt, 10 + 440 + 3 * spacing, btnY, 140, 32, TRUE);
    MoveWindow(m_hBtnUndoApt, 10 + 580 + 4 * spacing, btnY, 100, 32, TRUE);

    int listH = btnY - 20;
    MoveWindow(m_hAppointmentList, 10, 10, w - 20, listH, TRUE);
}

void MainWindow::layoutQueuePanel(int w, int h) {
    MoveWindow(m_hQueueBanner, 10, 10, w - 20, 50, TRUE);

    int btnY = h - 46;
    MoveWindow(m_hBtnCallNextQueue, 10, btnY, 280, 36, TRUE);
    MoveWindow(m_hBtnAddSelectedToQueue, 300, btnY, 220, 36, TRUE);

    int listY = 70;
    int listH = btnY - listY - 10;
    MoveWindow(m_hQueueList, 10, listY, w - 20, listH, TRUE);
}

void MainWindow::layoutSearchPanel(int w, int h) {
    // Row 1: Search controls
    MoveWindow(m_hSearchEdit, 100, 10, 150, 24, TRUE);
    MoveWindow(m_hComboSearchAlgo, 330, 10, 200, 24, TRUE);
    MoveWindow(m_hComboSearchField, 580, 10, 90, 24, TRUE);
    MoveWindow(m_hBtnExecuteSearch, 680, 9, 130, 26, TRUE);

    // Row 2: Sort controls
    MoveWindow(m_hComboSortAlgo, 100, 42, 160, 24, TRUE);
    MoveWindow(m_hComboSortField, 320, 42, 140, 24, TRUE);
    MoveWindow(m_hComboSortOrder, 510, 42, 100, 24, TRUE);
    MoveWindow(m_hBtnExecuteSort, 620, 41, 120, 26, TRUE);

    // Row 3: Diagnostics strip
    MoveWindow(m_hSearchMetricsText, 10, 74, w - 20, 28, TRUE);

    // Results table
    int listY = 110;
    int listH = h - listY - 10;
    MoveWindow(m_hSearchResultsList, 10, listY, w - 20, listH, TRUE);
}

void MainWindow::layoutReportPanel(int w, int h) {
    MoveWindow(m_hComboReportType, 160, 10, 340, 24, TRUE);
    MoveWindow(m_hBtnGenerateReport, 510, 9, 130, 26, TRUE);
    MoveWindow(m_hBtnExportReport, 650, 9, 130, 26, TRUE);

    int textY = 46;
    int textH = h - textY - 10;
    MoveWindow(m_hReportTextView, 10, textY, w - 20, textH, TRUE);
}

// -------------------------------------------------------------
// View Switching & Updates
// -------------------------------------------------------------
void MainWindow::onTabChanged(int newIndex) {
    m_currentTabIndex = newIndex;
    for (int i = 0; i < 6; ++i) {
        if (m_panels[i]) {
            ShowWindow(m_panels[i], (i == newIndex) ? SW_SHOW : SW_HIDE);
        }
    }

    if (newIndex == 0) refreshDashboard();
    else if (newIndex == 1) refreshPatientList();
    else if (newIndex == 2) refreshAppointmentList();
    else if (newIndex == 3) refreshQueueList();
    else if (newIndex == 4) onSearchClicked();
    else if (newIndex == 5) onGenerateReportClicked();

    refreshStatusBar();
}

void MainWindow::refreshAllViews() {
    refreshDashboard();
    refreshPatientList();
    refreshAppointmentList();
    refreshQueueList();
    refreshCacheList();
    refreshStatusBar();
}

void MainWindow::refreshDashboard() {
    auto& db = m_manager.getDatabase();
    auto& aptCtrl = m_manager.getAppointmentController();
    auto daily = aptCtrl.getDailySummary();

    std::ostringstream s0, s1, s2, s3, s4;
    s0 << "TOTAL PATIENTS\n" << db.getPatientCount();
    s1 << "PENDING APPOINTMENTS\n" << daily.pending;
    s2 << "WAITING QUEUE\n" << db.getQueueSize() << " Patients";
    s3 << "COMPLETED TODAY\n" << daily.completed;
    s4 << "REVENUE COLLECTED\n$" << std::fixed << std::setprecision(2) << daily.totalRevenue;

    SetWindowTextA(m_hDashKpiCards[0], s0.str().c_str());
    SetWindowTextA(m_hDashKpiCards[1], s1.str().c_str());
    SetWindowTextA(m_hDashKpiCards[2], s2.str().c_str());
    SetWindowTextA(m_hDashKpiCards[3], s3.str().c_str());
    SetWindowTextA(m_hDashKpiCards[4], s4.str().c_str());

    refreshCacheList();
}

void MainWindow::refreshCacheList() {
    ListView_DeleteAllItems(m_hDashRecentList);
    auto cachedPatients = m_manager.getRecentlyAccessedPatients();

    for (size_t i = 0; i < cachedPatients.size(); ++i) {
        const auto* p = cachedPatients[i];
        if (!p) continue;

        LVITEMA lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;

        std::string slotStr = "[" + std::to_string(i + 1) + "]";
        lvi.pszText = const_cast<char*>(slotStr.c_str());
        ListView_InsertItem(m_hDashRecentList, &lvi);

        ListView_SetItemText(m_hDashRecentList, (int)i, 1, const_cast<char*>(p->getId().c_str()));
        ListView_SetItemText(m_hDashRecentList, (int)i, 2, const_cast<char*>(p->getName().c_str()));
        ListView_SetItemText(m_hDashRecentList, (int)i, 3, const_cast<char*>(p->getPatientType().c_str()));
        std::string ageStr = std::to_string(p->getAge());
        ListView_SetItemText(m_hDashRecentList, (int)i, 4, const_cast<char*>(ageStr.c_str()));
        ListView_SetItemText(m_hDashRecentList, (int)i, 5, const_cast<char*>(p->getContact().c_str()));

        std::ostringstream feeOss;
        feeOss << "$" << std::fixed << std::setprecision(2) << p->calculateConsultationFee();
        ListView_SetItemText(m_hDashRecentList, (int)i, 6, const_cast<char*>(feeOss.str().c_str()));
    }
}

void MainWindow::refreshPatientList(const std::string& filter) {
    const auto& all = m_manager.getDatabase().getAllPatients();
    std::vector<std::shared_ptr<model::PatientRecord>> list;

    if (filter.empty()) {
        list = all;
    } else {
        list = controller::SearchEngine::filterByKeyword(all, filter);
    }

    populatePatientListView(m_hPatientList, list);
}

void MainWindow::populatePatientListView(HWND hList, const std::vector<std::shared_ptr<model::PatientRecord>>& patients) {
    ListView_DeleteAllItems(hList);

    for (size_t i = 0; i < patients.size(); ++i) {
        const auto& p = patients[i];
        if (!p) continue;

        LVITEMA lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;
        lvi.pszText = const_cast<char*>(p->getId().c_str());
        ListView_InsertItem(hList, &lvi);

        ListView_SetItemText(hList, (int)i, 1, const_cast<char*>(p->getName().c_str()));

        // Advanced formatting tag (Mandatory Section 9.a: Custom presentation)
        std::string categoryTag;
        if (p->getPatientType() == "Emergency") {
            categoryTag = "[EMERGENCY - High Priority]";
        } else if (p->getPatientType() == "Follow-Up") {
            categoryTag = "[Follow-Up - Scheduled]";
        } else {
            categoryTag = "[General - Normal]";
        }
        ListView_SetItemText(hList, (int)i, 2, const_cast<char*>(categoryTag.c_str()));

        std::string ageStr = std::to_string(p->getAge());
        ListView_SetItemText(hList, (int)i, 3, const_cast<char*>(ageStr.c_str()));
        ListView_SetItemText(hList, (int)i, 4, const_cast<char*>(p->getContact().c_str()));
        ListView_SetItemText(hList, (int)i, 5, const_cast<char*>(p->getRegistrationDate().c_str()));

        std::ostringstream feeOss;
        feeOss << "$" << std::fixed << std::setprecision(2) << p->calculateConsultationFee();
        ListView_SetItemText(hList, (int)i, 6, const_cast<char*>(feeOss.str().c_str()));

        std::string details = p->getDetails();
        ListView_SetItemText(hList, (int)i, 7, const_cast<char*>(details.c_str()));
    }
}

void MainWindow::refreshAppointmentList() {
    ListView_DeleteAllItems(m_hAppointmentList);
    const auto& appointments = m_manager.getDatabase().getAllAppointments();

    for (size_t i = 0; i < appointments.size(); ++i) {
        const auto& a = appointments[i];

        LVITEMA lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;
        lvi.pszText = const_cast<char*>(a.getAppointmentId().c_str());
        ListView_InsertItem(m_hAppointmentList, &lvi);

        ListView_SetItemText(m_hAppointmentList, (int)i, 1, const_cast<char*>(a.getPatientId().c_str()));
        ListView_SetItemText(m_hAppointmentList, (int)i, 2, const_cast<char*>(a.getPatientName().c_str()));
        ListView_SetItemText(m_hAppointmentList, (int)i, 3, const_cast<char*>(a.getDate().c_str()));
        ListView_SetItemText(m_hAppointmentList, (int)i, 4, const_cast<char*>(a.getTime().c_str()));
        ListView_SetItemText(m_hAppointmentList, (int)i, 5, const_cast<char*>(a.getDepartment().c_str()));
        ListView_SetItemText(m_hAppointmentList, (int)i, 6, const_cast<char*>(a.getDoctorName().c_str()));

        std::string statusTag = a.getStatusString();
        ListView_SetItemText(m_hAppointmentList, (int)i, 7, const_cast<char*>(statusTag.c_str()));

        std::ostringstream feeOss;
        feeOss << "$" << std::fixed << std::setprecision(2) << a.getFee();
        ListView_SetItemText(m_hAppointmentList, (int)i, 8, const_cast<char*>(feeOss.str().c_str()));
        ListView_SetItemText(m_hAppointmentList, (int)i, 9, const_cast<char*>(a.getNotes().c_str()));
    }
}

void MainWindow::refreshQueueList() {
    ListView_DeleteAllItems(m_hQueueList);
    auto queuePatients = m_manager.getWaitingPatientsList();

    for (size_t i = 0; i < queuePatients.size(); ++i) {
        const auto& p = queuePatients[i];
        if (!p) continue;

        LVITEMA lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;
        std::string posStr = "# " + std::to_string(i + 1);
        lvi.pszText = const_cast<char*>(posStr.c_str());
        ListView_InsertItem(m_hQueueList, &lvi);

        ListView_SetItemText(m_hQueueList, (int)i, 1, const_cast<char*>(p->getId().c_str()));
        ListView_SetItemText(m_hQueueList, (int)i, 2, const_cast<char*>(p->getName().c_str()));
        ListView_SetItemText(m_hQueueList, (int)i, 3, const_cast<char*>(p->getPatientType().c_str()));
        std::string ageStr = std::to_string(p->getAge());
        ListView_SetItemText(m_hQueueList, (int)i, 4, const_cast<char*>(ageStr.c_str()));
        ListView_SetItemText(m_hQueueList, (int)i, 5, const_cast<char*>(p->getContact().c_str()));

        std::string triage = (p->getPatientType() == "Emergency") ? "HIGH PRIORITY / IMMEDIATE" : "Standard Waiting";
        ListView_SetItemText(m_hQueueList, (int)i, 6, const_cast<char*>(triage.c_str()));
    }
}

void MainWindow::refreshStatusBar() {
    if (!m_hStatusBar) return;

    std::string undoDesc = m_manager.canUndo()
        ? " Undo Ready: " + m_manager.getUndoActionDescription()
        : " Undo: No pending operation";
    SendMessageA(m_hStatusBar, SB_SETTEXTA, 1, (LPARAM)undoDesc.c_str());

    std::string queueDesc = " Queue: " + std::to_string(m_manager.getQueueLength()) + " waiting";
    SendMessageA(m_hStatusBar, SB_SETTEXTA, 2, (LPARAM)queueDesc.c_str());
}

// -------------------------------------------------------------
// Actions & Event Handling (Mandatory Section 6)
// -------------------------------------------------------------
void MainWindow::onAddPatientClicked() {
    auto res = PatientDialog::showAddDialog(m_hwndMain);
    if (!res.confirmed) return;

    try {
        std::string newId;
        if (res.category == "General") {
            newId = m_manager.registerGeneralPatient(
                res.name, res.age, res.contact, res.symptoms, res.routineCheckup);
        } else if (res.category == "Emergency") {
            newId = m_manager.registerEmergencyPatient(
                res.name, res.age, res.contact, res.severityLevel, res.emergencyContact);
        } else if (res.category == "Follow-Up") {
            newId = m_manager.registerFollowUpPatient(
                res.name, res.age, res.contact, res.previousVisitDate, res.followUpReason, res.attendingDoctor);
        }

        refreshAllViews();
        std::string msg = "Patient Registered Successfully!\n\n"
                          "Assigned Patient ID: " + newId + "\n"
                          "Full Name: " + res.name + "\n"
                          "Category: " + res.category + "\n"
                          "Age: " + std::to_string(res.age) + "\n"
                          "Contact: " + res.contact + "\n\n"
                          "The patient record has been saved to the database and cached.";
        MessageBoxA(m_hwndMain, msg.c_str(), "Registration Successful", MB_OK | MB_ICONINFORMATION);
    } catch (const utils::ClinicException& ex) {
        MessageBoxA(m_hwndMain, ex.what(), "Registration Error", MB_ICONERROR);
    }
}

void MainWindow::onEditPatientClicked() {
    int sel = getListViewSelectedIndex(m_hPatientList);
    if (sel < 0) {
        MessageBoxA(m_hwndMain, "Please select a patient from the patient records table to edit.", "Selection Required", MB_ICONWARNING);
        return;
    }

    std::string id = getListViewItemText(m_hPatientList, sel, 0);
    auto p = m_manager.getPatient(id);
    if (!p) {
        MessageBoxA(m_hwndMain, "Selected patient record could not be found.", "Error", MB_ICONERROR);
        return;
    }

    auto res = PatientDialog::showEditDialog(m_hwndMain, p);
    if (!res.confirmed) return;

    try {
        // Update basic record in database (pushes previous state to Undo stack!)
        m_manager.updatePatient(id, res.name, res.age, res.contact);

        // Update category-specific attributes
        if (auto gp = dynamic_cast<model::GeneralPatient*>(p.get())) {
            gp->setSymptoms(res.symptoms);
            gp->setRoutineCheckup(res.routineCheckup);
        } else if (auto ep = dynamic_cast<model::EmergencyPatient*>(p.get())) {
            ep->setSeverityLevel(res.severityLevel);
            ep->setEmergencyContact(res.emergencyContact);
        } else if (auto fp = dynamic_cast<model::FollowUpPatient*>(p.get())) {
            fp->setPreviousVisitDate(res.previousVisitDate);
            fp->setFollowUpReason(res.followUpReason);
            fp->setAttendingDoctor(res.attendingDoctor);
        }

        refreshAllViews();
        std::string msg = "Patient record [" + id + "] updated successfully.\n"
                          "Previous state saved to the administrative Undo stack (Ctrl+Z).";
        MessageBoxA(m_hwndMain, msg.c_str(), "Update Successful", MB_OK | MB_ICONINFORMATION);
    } catch (const utils::ClinicException& ex) {
        MessageBoxA(m_hwndMain, ex.what(), "Error Updating Patient", MB_ICONERROR);
    }
}

void MainWindow::onDeletePatientClicked() {
    int sel = getListViewSelectedIndex(m_hPatientList);
    if (sel < 0) {
        MessageBoxA(m_hwndMain, "Please select a patient from the records table to delete.", "Selection Required", MB_ICONWARNING);
        return;
    }

    std::string id = getListViewItemText(m_hPatientList, sel, 0);
    std::string name = getListViewItemText(m_hPatientList, sel, 1);

    std::string confirmMsg = "Are you sure you want to permanently delete patient:\n\n"
                             "'" + name + "' (" + id + ")\n\n"
                             "Note: All scheduled appointments associated with this patient will also be removed.\n"
                             "This operation can be reversed via the Undo feature (Ctrl+Z).";
    if (MessageBoxA(m_hwndMain, confirmMsg.c_str(), "Confirm Patient Deletion", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        try {
            m_manager.deletePatient(id);
            refreshAllViews();
            MessageBoxA(m_hwndMain, "Patient record deleted successfully.\nOperation recorded in the Undo stack.", "Deleted", MB_OK | MB_ICONINFORMATION);
        } catch (const utils::ClinicException& ex) {
            MessageBoxA(m_hwndMain, ex.what(), "Error Deleting Patient", MB_ICONERROR);
        }
    }
}

void MainWindow::onQueuePatientClicked() {
    int sel = getListViewSelectedIndex(m_hPatientList);
    if (sel < 0) {
        MessageBoxA(m_hwndMain, "Please select a patient from the records table to add to the waiting queue.", "Selection Required", MB_ICONWARNING);
        return;
    }

    std::string id = getListViewItemText(m_hPatientList, sel, 0);
    std::string name = getListViewItemText(m_hPatientList, sel, 1);

    try {
        m_manager.addPatientToQueue(id);
        refreshAllViews();

        std::string msg = "Patient '" + name + "' (" + id + ") has been queued for doctor consultation.\n"
                          "Total patients in waiting queue: " + std::to_string(m_manager.getQueueLength());
        MessageBoxA(m_hwndMain, msg.c_str(), "Patient Queued (FIFO)", MB_OK | MB_ICONINFORMATION);
    } catch (const utils::ClinicException& ex) {
        MessageBoxA(m_hwndMain, ex.what(), "Queue Error", MB_ICONERROR);
    }
}

void MainWindow::onUndoClicked() {
    if (!m_manager.canUndo()) {
        MessageBoxA(m_hwndMain, "No administrative operations to undo in the stack.", "Undo Stack Empty", MB_ICONINFORMATION);
        return;
    }

    std::string desc = m_manager.getUndoActionDescription();
    try {
        if (m_manager.undoLastAction()) {
            refreshAllViews();
            std::string msg = "Successfully reversed last administrative operation:\n\n" + desc;
            MessageBoxA(m_hwndMain, msg.c_str(), "Undo Successful", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBoxA(m_hwndMain, "Undo operation could not be completed.", "Undo Notice", MB_ICONWARNING);
        }
    } catch (const utils::ClinicException& ex) {
        MessageBoxA(m_hwndMain, ex.what(), "Undo Error", MB_ICONERROR);
    }
}

void MainWindow::onScheduleAppointmentClicked() {
    int sel = getListViewSelectedIndex(m_hPatientList);
    std::string preselectedId = "";
    if (sel >= 0) {
        preselectedId = getListViewItemText(m_hPatientList, sel, 0);
    }

    auto allPatients = m_manager.getDatabase().getAllPatients();
    if (allPatients.empty()) {
        MessageBoxA(m_hwndMain, "No patients found in the database. Please register a patient first.", "Notice", MB_ICONINFORMATION);
        return;
    }

    auto res = AppointmentDialog::showScheduleDialog(m_hwndMain, allPatients, preselectedId);
    if (!res.confirmed) return;

    try {
        std::string aptId = m_manager.getAppointmentController().scheduleNewAppointment(
            res.patientId, res.date, res.time, res.department, res.doctor, res.notes);
        refreshAllViews();

        std::string msg = "Clinical Appointment Scheduled Successfully!\n\n"
                          "Appointment ID: " + aptId + "\n"
                          "Patient: " + res.patientName + " (" + res.patientId + ")\n"
                          "Date & Time: " + res.date + " at " + res.time + "\n"
                          "Department: " + res.department + "\n"
                          "Attending Doctor: " + res.doctor + "\n\n"
                          "Action recorded in database and Undo stack.";
        MessageBoxA(m_hwndMain, msg.c_str(), "Appointment Confirmed", MB_OK | MB_ICONINFORMATION);
    } catch (const utils::ClinicException& ex) {
        MessageBoxA(m_hwndMain, ex.what(), "Appointment Error", MB_ICONERROR);
    }
}

void MainWindow::onCancelAppointmentClicked() {
    int sel = getListViewSelectedIndex(m_hAppointmentList);
    if (sel < 0) {
        MessageBoxA(m_hwndMain, "Please select an appointment from the list to cancel.", "Selection Required", MB_ICONWARNING);
        return;
    }

    std::string aptId = getListViewItemText(m_hAppointmentList, sel, 0);
    std::string patName = getListViewItemText(m_hAppointmentList, sel, 2);

    std::string confirmMsg = "Are you sure you want to cancel appointment " + aptId + " (" + patName + ")?\n"
                             "This cancellation can be reversed via Undo (Ctrl+Z).";
    if (MessageBoxA(m_hwndMain, confirmMsg.c_str(), "Confirm Cancellation", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        try {
            if (m_manager.getAppointmentController().cancelExistingAppointment(aptId)) {
                refreshAllViews();
                MessageBoxA(m_hwndMain, "Appointment marked as Cancelled. Action recorded in Undo stack.", "Cancelled", MB_OK | MB_ICONINFORMATION);
            } else {
                MessageBoxA(m_hwndMain, "Failed to cancel appointment. Appointment ID not found.", "Error", MB_ICONERROR);
            }
        } catch (const utils::ClinicException& ex) {
            MessageBoxA(m_hwndMain, ex.what(), "Error", MB_ICONERROR);
        }
    }
}

void MainWindow::onCompleteAppointmentClicked() {
    int sel = getListViewSelectedIndex(m_hAppointmentList);
    if (sel < 0) {
        MessageBoxA(m_hwndMain, "Please select an appointment from the list to mark as completed.", "Selection Required", MB_ICONWARNING);
        return;
    }

    std::string aptId = getListViewItemText(m_hAppointmentList, sel, 0);
    std::string patName = getListViewItemText(m_hAppointmentList, sel, 2);

    try {
        if (m_manager.getAppointmentController().completeAppointment(aptId)) {
            refreshAllViews();
            std::string msg = "Appointment " + aptId + " for '" + patName + "' has been marked as Completed.\n"
                              "Consultation fee has been credited to realized revenue.";
            MessageBoxA(m_hwndMain, msg.c_str(), "Appointment Completed", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBoxA(m_hwndMain, "Failed to complete appointment. Appointment ID not found.", "Error", MB_ICONERROR);
        }
    } catch (const utils::ClinicException& ex) {
        MessageBoxA(m_hwndMain, ex.what(), "Error", MB_ICONERROR);
    }
}

void MainWindow::onDeleteAppointmentClicked() {
    int sel = getListViewSelectedIndex(m_hAppointmentList);
    if (sel < 0) {
        MessageBoxA(m_hwndMain, "Please select an appointment from the list to delete.", "Selection Required", MB_ICONWARNING);
        return;
    }

    std::string aptId = getListViewItemText(m_hAppointmentList, sel, 0);

    std::string confirmMsg = "Are you sure you want to permanently delete appointment " + aptId + " from the database?\n"
                             "This action can be reversed via Undo (Ctrl+Z).";
    if (MessageBoxA(m_hwndMain, confirmMsg.c_str(), "Confirm Deletion", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        try {
            if (m_manager.getDatabase().deleteAppointment(aptId, true)) {
                refreshAllViews();
                MessageBoxA(m_hwndMain, "Appointment deleted from database.", "Deleted", MB_OK | MB_ICONINFORMATION);
            } else {
                MessageBoxA(m_hwndMain, "Appointment not found in database.", "Error", MB_ICONERROR);
            }
        } catch (const utils::ClinicException& ex) {
            MessageBoxA(m_hwndMain, ex.what(), "Error", MB_ICONERROR);
        }
    }
}

void MainWindow::onCallNextPatientClicked() {
    try {
        if (m_manager.getQueueLength() == 0) {
            MessageBoxA(m_hwndMain, "The waiting queue is currently empty. No patients waiting.", "Queue Empty", MB_OK | MB_ICONINFORMATION);
            return;
        }

        auto nextPatient = m_manager.callNextPatient();
        if (nextPatient) {
            m_currentlyServingPatientName = nextPatient->getName() + " (" + nextPatient->getId() +
                                           ") - " + nextPatient->getPatientType();
            std::string banner = " NOW CALLING / SERVING: " + m_currentlyServingPatientName + " [Consultation Room 1]";
            SetWindowTextA(m_hQueueBanner, banner.c_str());

            std::string alertMsg = " Doctor Consultation Alert - Next Patient Called (FIFO):\n\n"
                                   "Patient Name: " + nextPatient->getName() + "\n"
                                   "Patient ID: " + nextPatient->getId() + "\n"
                                   "Category: " + nextPatient->getPatientType() + "\n"
                                   "Age: " + std::to_string(nextPatient->getAge()) + "\n"
                                   "Contact: " + nextPatient->getContact() + "\n\n"
                                   "Clinical Notes: " + nextPatient->getDetails() + "\n\n"
                                   "Assigned to: Consultation Room 1";
            MessageBoxA(m_hwndMain, alertMsg.c_str(), "Patient Called for Consultation", MB_OK | MB_ICONINFORMATION);
            refreshAllViews();
        }
    } catch (const utils::QueueEmptyException& ex) {
        MessageBoxA(m_hwndMain, ex.what(), "Queue Notice", MB_OK | MB_ICONINFORMATION);
    } catch (const utils::ClinicException& ex) {
        MessageBoxA(m_hwndMain, ex.what(), "Queue Error", MB_ICONERROR);
    }
}

// -------------------------------------------------------------
// Search & Sort Event Handlers (Mandatory Sections 4 & 5)
// -------------------------------------------------------------
void MainWindow::onSearchClicked() {
    char bufQuery[256];
    GetWindowTextA(m_hSearchEdit, bufQuery, sizeof(bufQuery));
    std::string query = bufQuery;

    int algoSel = (int)SendMessage(m_hComboSearchAlgo, CB_GETCURSEL, 0, 0);
    int fieldSel = (int)SendMessage(m_hComboSearchField, CB_GETCURSEL, 0, 0);

    controller::SearchAlgorithmType algo = controller::SearchAlgorithmType::LINEAR_SEARCH;
    if (algoSel == 1) algo = controller::SearchAlgorithmType::BINARY_SEARCH;
    else if (algoSel == 2) algo = controller::SearchAlgorithmType::RECURSIVE_SEARCH;

    std::string fieldName = (fieldSel == 1) ? "Name" : "ID";

    auto res = m_manager.search(query, algo, fieldName);

    std::vector<std::shared_ptr<model::PatientRecord>> displayList;
    if (res.found && res.match) {
        displayList = res.multipleMatches;
    } else if (query.empty()) {
        displayList = m_manager.getDatabase().getAllPatients();
    }

    populatePatientListView(m_hSearchResultsList, displayList);

    std::ostringstream oss;
    oss << " Search Diagnostics: " << res.algorithmUsed
        << " | Comparisons: " << res.comparisons
        << " | Execution Time: " << std::fixed << std::setprecision(2) << res.durationMicroseconds << " \xC2\xB5s"
        << " | Result: " << (res.found ? "MATCH FOUND" : "NO EXACT MATCH");
    SetWindowTextA(m_hSearchMetricsText, oss.str().c_str());
}

void MainWindow::onSortClicked() {
    int algoSel = (int)SendMessage(m_hComboSortAlgo, CB_GETCURSEL, 0, 0);
    int fieldSel = (int)SendMessage(m_hComboSortField, CB_GETCURSEL, 0, 0);
    int orderSel = (int)SendMessage(m_hComboSortOrder, CB_GETCURSEL, 0, 0);

    controller::SortAlgorithmType algo = controller::SortAlgorithmType::QUICK_SORT;
    if (algoSel == 1) algo = controller::SortAlgorithmType::SELECTION_SORT;
    else if (algoSel == 2) algo = controller::SortAlgorithmType::MERGE_SORT;

    controller::SortField field = controller::SortField::BY_NAME;
    if (fieldSel == 1) field = controller::SortField::BY_ID;
    else if (fieldSel == 2) field = controller::SortField::BY_AGE;
    else if (fieldSel == 3) field = controller::SortField::BY_DATE;

    bool ascending = (orderSel == 0);

    auto res = m_manager.sort(algo, field, ascending);
    m_lastSortReason = res.reasonWhyAppropriate;

    // Display sorted records in Results view
    populatePatientListView(m_hSearchResultsList, m_manager.getDatabase().getAllPatients());
    refreshPatientList(); // also sync main patient list

    std::ostringstream oss;
    oss << " Sort Diagnostics: " << res.algorithmName
        << " on [" << res.fieldName << "] (" << (ascending ? "Asc" : "Desc") << ")"
        << " | Comparisons: " << res.comparisons
        << " | Swaps: " << res.swaps
        << " | Duration: " << std::fixed << std::setprecision(2) << res.durationMicroseconds << " \xC2\xB5s"
        << " | Optimal Use: " << res.reasonWhyAppropriate;
    SetWindowTextA(m_hSearchMetricsText, oss.str().c_str());
}

// -------------------------------------------------------------
// Report Event Handlers (Mandatory Section "Simple Reports")
// -------------------------------------------------------------
void MainWindow::onGenerateReportClicked() {
    int sel = (int)SendMessage(m_hComboReportType, CB_GETCURSEL, 0, 0);
    auto& aptCtrl = m_manager.getAppointmentController();

    std::ostringstream oss;
    if (sel == 0) { // 1. Most Frequently Attended Patients
        auto top = aptCtrl.getMostFrequentlyAttendedPatients(10);
        oss << "======================================================================\n"
            << "              REPORT 1: MOST FREQUENTLY ATTENDED PATIENTS\n"
            << "         (Patients with the highest number of completed visits)\n"
            << "======================================================================\n\n"
            << std::left << std::setw(6) << "Rank"
            << std::setw(12) << "Patient ID"
            << std::setw(24) << "Patient Name"
            << std::setw(18) << "Category"
            << std::setw(15) << "Completed Visits" << "\n"
            << "----------------------------------------------------------------------\n";

        for (size_t i = 0; i < top.size(); ++i) {
            oss << std::left << std::setw(6) << (i + 1)
                << std::setw(12) << top[i].patientId
                << std::setw(24) << top[i].patientName
                << std::setw(18) << top[i].patientType
                << std::setw(15) << top[i].completedCount << "\n";
        }
    } else if (sel == 1) { // 2. Patients with Pending Appointments
        auto pending = aptCtrl.getPendingAppointments();
        oss << "======================================================================\n"
            << "             REPORT 2: PATIENTS WITH PENDING APPOINTMENTS\n"
            << "     (Patients who have active appointments not yet completed)\n"
            << "======================================================================\n\n"
            << std::left << std::setw(10) << "Apt ID"
            << std::setw(12) << "Patient ID"
            << std::setw(22) << "Patient Name"
            << std::setw(12) << "Date"
            << std::setw(8) << "Time"
            << std::setw(16) << "Department"
            << std::setw(14) << "Attending Dr." << "\n"
            << "----------------------------------------------------------------------\n";

        for (const auto& a : pending) {
            oss << std::left << std::setw(10) << a.getAppointmentId()
                << std::setw(12) << a.getPatientId()
                << std::setw(22) << a.getPatientName()
                << std::setw(12) << a.getDate()
                << std::setw(8) << a.getTime()
                << std::setw(16) << a.getDepartment()
                << std::setw(14) << a.getDoctorName() << "\n";
        }
        oss << "\nTotal Pending Consultations: " << pending.size() << "\n";
    } else if (sel == 2) { // 3. Patient Category Distribution
        auto dist = aptCtrl.getCategoryDistribution();
        oss << "======================================================================\n"
            << "              REPORT 3: PATIENT CATEGORY DISTRIBUTION\n"
            << "                 (Outpatient Demographics & Triage)\n"
            << "======================================================================\n\n"
            << "Category Type              Total Count      Percentage (%)\n"
            << "----------------------------------------------------------------------\n";

        double total = dist.total > 0 ? static_cast<double>(dist.total) : 1.0;
        oss << "General Patients           " << std::setw(12) << dist.generalCount
            << std::fixed << std::setprecision(1) << (dist.generalCount * 100.0 / total) << " %\n";
        oss << "Emergency Patients (Acute) " << std::setw(12) << dist.emergencyCount
            << std::fixed << std::setprecision(1) << (dist.emergencyCount * 100.0 / total) << " %\n";
        oss << "Follow-Up Patients         " << std::setw(12) << dist.followUpCount
            << std::fixed << std::setprecision(1) << (dist.followUpCount * 100.0 / total) << " %\n";
        oss << "----------------------------------------------------------------------\n"
            << "TOTAL REGISTERED PATIENTS: " << dist.total << "\n";
    } else if (sel == 3) { // 4. Daily Appointment Summary
        auto daily = aptCtrl.getDailySummary("2026-11-06");
        oss << "======================================================================\n"
            << "                REPORT 4: DAILY APPOINTMENT SUMMARY\n"
            << "                 (Clinic Operational Day: 2026-11-06)\n"
            << "======================================================================\n\n"
            << "Total Appointments Scheduled: " << daily.totalAppointments << "\n"
            << "Completed Consultations:      " << daily.completed << "\n"
            << "Pending Patients to be Seen:  " << daily.pending << "\n"
            << "Cancelled Appointments:       " << daily.cancelled << "\n"
            << "Total Daily Revenue:          $" << std::fixed << std::setprecision(2) << daily.totalRevenue << "\n";
    } else { // 5. Full Executive Report
        oss << aptCtrl.generateTextReportSummary();
    }

    SetWindowTextA(m_hReportTextView, oss.str().c_str());
}

void MainWindow::onExportReportClicked() {
    char szFileName[MAX_PATH] = "Clinic_Executive_Report.txt";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = m_hwndMain;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "txt";

    if (GetSaveFileNameA(&ofn)) {
        int len = GetWindowTextLengthA(m_hReportTextView);
        std::vector<char> buf(len + 1);
        GetWindowTextA(m_hReportTextView, buf.data(), len + 1);

        FILE* fp = fopen(szFileName, "w");
        if (fp) {
            fputs(buf.data(), fp);
            fclose(fp);
            MessageBoxA(m_hwndMain, "Report successfully exported to disk.", "Export Complete", MB_OK);
        } else {
            MessageBoxA(m_hwndMain, "Failed to write report file.", "File Error", MB_ICONERROR);
        }
    }
}

// -------------------------------------------------------------
// File Persistence (Mandatory Section 9.c & 11.d)
// -------------------------------------------------------------
void MainWindow::onFileNew() {
    if (MessageBoxA(m_hwndMain, "Reset database and initialize sample records?", "New Database", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        m_manager.resetWithSampleData();
        refreshAllViews();
        MessageBoxA(m_hwndMain, "New database initialized with baseline clinic records.", "Initialized", MB_OK);
    }
}

void MainWindow::onFileOpen() {
    char szFileName[MAX_PATH] = "";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = m_hwndMain;
    ofn.lpstrFilter = "CSV Data Files (*.csv)\0*.csv\0Unified Export (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        try {
            utils::FileHandler::importFullDatabase(m_manager.getDatabase(), szFileName);
            refreshAllViews();
            MessageBoxA(m_hwndMain, "Clinic database imported successfully.", "Import Complete", MB_OK);
        } catch (const utils::ClinicException& ex) {
            MessageBoxA(m_hwndMain, ex.what(), "Import Failure", MB_ICONERROR);
        }
    }
}

void MainWindow::onFileSave() {
    char szFileName[MAX_PATH] = "clinic_database_export.txt";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = m_hwndMain;
    ofn.lpstrFilter = "Unified Clinic Export (*.txt)\0*.txt\0CSV Files (*.csv)\0*.csv\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "txt";

    if (GetSaveFileNameA(&ofn)) {
        try {
            utils::FileHandler::exportFullDatabase(m_manager.getDatabase(), szFileName);
            MessageBoxA(m_hwndMain, "Clinic database successfully saved.", "Save Complete", MB_OK);
        } catch (const utils::ClinicException& ex) {
            MessageBoxA(m_hwndMain, ex.what(), "Save Failure", MB_ICONERROR);
        }
    }
}

void MainWindow::onFileExport() {
    onExportReportClicked();
}

// -------------------------------------------------------------
// Timers (Mandatory Section 6.d, 9.d)
// -------------------------------------------------------------
void MainWindow::onTimerTick(UINT_PTR timerId) {
    if (timerId == TIMER_CLOCK_ID) {
        // Update live time on status bar
        time_t now = time(nullptr);
        tm localTm;
        localtime_s(&localTm, &now);
        char timeBuf[64];
        strftime(timeBuf, sizeof(timeBuf), " %Y-%m-%d  %H:%M:%S", &localTm);
        SendMessageA(m_hStatusBar, SB_SETTEXTA, 3, (LPARAM)timeBuf);

        // Check for waiting patients reminder (>3 patients alert)
        if (m_manager.getQueueLength() > 3) {
            SetWindowTextA(m_hDashAlertText,
                " Alert: Waiting queue has >3 patients. Please call next patient for doctor consultation.");
        }
    } else if (timerId == TIMER_DASHBOARD_ID) {
        // Periodic statistics refresh
        refreshStatusBar();
        if (m_currentTabIndex == 0) {
            refreshDashboard();
        }
    }
}

// -------------------------------------------------------------
// Panel Subclass Procedure (Forwards Child Control Messages)
// -------------------------------------------------------------
LRESULT CALLBACK MainWindow::PanelSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            // Forward button clicks, combobox selections, edit box changes to MainWindow
            return SendMessageA(GetParent(hwnd), WM_COMMAND, wParam, lParam);

        case WM_NOTIFY:
            // Forward ListView notifications, column clicks, double-clicks to MainWindow
            return SendMessageA(GetParent(hwnd), WM_NOTIFY, wParam, lParam);
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

// -------------------------------------------------------------
// Window Procedure
// -------------------------------------------------------------
LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (!g_pMainWindow) return DefWindowProcA(hwnd, uMsg, wParam, lParam);

    switch (uMsg) {
        case WM_SIZE:
            g_pMainWindow->onResize(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_NOTIFY: {
            LPNMHDR pnm = (LPNMHDR)lParam;
            if (pnm->idFrom == ID_MAIN_TAB && pnm->code == TCN_SELCHANGE) {
                int sel = TabCtrl_GetCurSel(g_pMainWindow->m_hTabControl);
                g_pMainWindow->onTabChanged(sel);
                return 0;
            }
            if (pnm->code == NM_DBLCLK) {
                if (pnm->idFrom == IDC_PATIENT_LIST) {
                    g_pMainWindow->onEditPatientClicked();
                    return 0;
                }
                if (pnm->idFrom == IDC_APPOINTMENT_LIST) {
                    g_pMainWindow->onCompleteAppointmentClicked();
                    return 0;
                }
            }
            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            // Menu and Accelerators
            if (wmId == IDM_FILE_NEW) { g_pMainWindow->onFileNew(); return 0; }
            if (wmId == IDM_FILE_OPEN) { g_pMainWindow->onFileOpen(); return 0; }
            if (wmId == IDM_FILE_SAVE || wmId == IDA_SAVE) { g_pMainWindow->onFileSave(); return 0; }
            if (wmId == IDM_FILE_EXPORT) { g_pMainWindow->onFileExport(); return 0; }
            if (wmId == IDM_FILE_EXIT || wmId == IDA_EXIT) { PostQuitMessage(0); return 0; }
            if (wmId == IDM_EDIT_UNDO || wmId == IDA_UNDO) { g_pMainWindow->onUndoClicked(); return 0; }
            if (wmId == IDA_SEARCH) { TabCtrl_SetCurSel(g_pMainWindow->m_hTabControl, 4); g_pMainWindow->onTabChanged(4); return 0; }
            if (wmId == IDM_VIEW_REFRESH) { g_pMainWindow->refreshAllViews(); return 0; }
            if (wmId == IDM_HELP_ABOUT) {
                MessageBoxA(hwnd,
                    "Smart Clinic Appointment & Patient Management System (SCAPMS)\n"
                    "COS202 Programming II - Lab Assessment Project\n\n"
                    "Features:\n"
                    "- Advanced OOP (Abstract PatientRecord, Appointable interface, 3 Subclasses)\n"
                    "- Data Structures (std::vector, std::queue, std::stack, fixed-size cache)\n"
                    "- Student-implemented Quick Sort, Selection Sort, Merge Sort\n"
                    "- Student-implemented Linear Search, Binary Search, Recursive Search\n"
                    "- Real-time Event-driven GUI, Custom Layouts, Input Validation, Exception Handling",
                    "About SCAPMS", MB_OK | MB_ICONINFORMATION);
                return 0;
            }

            // Dashboard buttons
            if (wmId == IDC_BTN_DASH_ADD_PAT) { g_pMainWindow->onAddPatientClicked(); return 0; }
            if (wmId == IDC_BTN_DASH_ADD_APT) { g_pMainWindow->onScheduleAppointmentClicked(); return 0; }
            if (wmId == IDC_BTN_DASH_CALL_NEXT) { g_pMainWindow->onCallNextPatientClicked(); return 0; }

            // Patient buttons
            if (wmId == IDC_BTN_ADD_PATIENT) { g_pMainWindow->onAddPatientClicked(); return 0; }
            if (wmId == IDC_BTN_EDIT_PATIENT) { g_pMainWindow->onEditPatientClicked(); return 0; }
            if (wmId == IDC_BTN_DELETE_PATIENT) { g_pMainWindow->onDeletePatientClicked(); return 0; }
            if (wmId == IDC_BTN_QUEUE_PATIENT) { g_pMainWindow->onQueuePatientClicked(); return 0; }
            if (wmId == IDC_BTN_UNDO_PATIENT) { g_pMainWindow->onUndoClicked(); return 0; }

            // Live filter edit
            if (wmId == IDC_PATIENT_FILTER_EDIT && wmEvent == EN_CHANGE) {
                char buf[256];
                GetWindowTextA(g_pMainWindow->m_hPatientFilterEdit, buf, sizeof(buf));
                g_pMainWindow->refreshPatientList(buf);
                return 0;
            }

            // Appointment buttons
            if (wmId == IDC_BTN_SCHEDULE_APPOINTMENT) { g_pMainWindow->onScheduleAppointmentClicked(); return 0; }
            if (wmId == IDC_BTN_CANCEL_APPOINTMENT) { g_pMainWindow->onCancelAppointmentClicked(); return 0; }
            if (wmId == IDC_BTN_COMPLETE_APPOINTMENT) { g_pMainWindow->onCompleteAppointmentClicked(); return 0; }
            if (wmId == IDC_BTN_DELETE_APPOINTMENT) { g_pMainWindow->onDeleteAppointmentClicked(); return 0; }
            if (wmId == IDC_BTN_UNDO_APPOINTMENT) { g_pMainWindow->onUndoClicked(); return 0; }

            // Queue buttons
            if (wmId == IDC_BTN_CALL_NEXT) { g_pMainWindow->onCallNextPatientClicked(); return 0; }
            if (wmId == IDC_BTN_ENQUEUE_SELECTED) {
                int sel = getListViewSelectedIndex(g_pMainWindow->m_hPatientList);
                if (sel >= 0) {
                    g_pMainWindow->onQueuePatientClicked();
                } else {
                    TabCtrl_SetCurSel(g_pMainWindow->m_hTabControl, 1);
                    g_pMainWindow->onTabChanged(1);
                    MessageBoxA(hwnd, "Please select a patient from the records table and click 'Add to Queue'.", "Select Patient to Queue", MB_OK | MB_ICONINFORMATION);
                }
                return 0;
            }

            // Search & Sort buttons
            if (wmId == IDC_BTN_DO_SEARCH) { g_pMainWindow->onSearchClicked(); return 0; }
            if (wmId == IDC_BTN_DO_SORT) { g_pMainWindow->onSortClicked(); return 0; }

            // Report buttons
            if (wmId == IDC_BTN_GENERATE_REPORT) { g_pMainWindow->onGenerateReportClicked(); return 0; }
            if (wmId == IDC_BTN_EXPORT_REPORT) { g_pMainWindow->onExportReportClicked(); return 0; }
            if (wmId == IDC_COMBO_REPORT_TYPE && wmEvent == CBN_SELCHANGE) {
                g_pMainWindow->onGenerateReportClicked();
                return 0;
            }

            break;
        }

        case WM_TIMER:
            g_pMainWindow->onTimerTick(wParam);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

} // namespace gui
} // namespace scapms
