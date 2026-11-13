#include "AppointmentDialog.h"
#include "../model/Appointable.h"
#include <commctrl.h>
#include <ctime>
#include <sstream>

namespace scapms {
namespace gui {

namespace {

// Control IDs
constexpr int IDC_APT_DLG_TITLE = 300;
constexpr int IDC_APT_DLG_PAT_LBL = 301;
constexpr int IDC_APT_DLG_PAT_COMBO = 302;
constexpr int IDC_APT_DLG_NOTICE_LBL = 303;
constexpr int IDC_APT_DLG_DATE_LBL = 304;
constexpr int IDC_APT_DLG_DATE_EDIT = 305;
constexpr int IDC_APT_DLG_TIME_LBL = 306;
constexpr int IDC_APT_DLG_TIME_COMBO = 307;
constexpr int IDC_APT_DLG_DEPT_LBL = 308;
constexpr int IDC_APT_DLG_DEPT_COMBO = 309;
constexpr int IDC_APT_DLG_DOC_LBL = 310;
constexpr int IDC_APT_DLG_DOC_COMBO = 311;
constexpr int IDC_APT_DLG_NOTES_LBL = 312;
constexpr int IDC_APT_DLG_NOTES_EDIT = 313;
constexpr int IDC_APT_DLG_BTN_OK = IDOK;
constexpr int IDC_APT_DLG_BTN_CANCEL = IDCANCEL;

// Dialog state
AppointmentDialogResult g_aptDialogResult;
std::vector<std::shared_ptr<model::PatientRecord>> g_patientsList;
std::string g_preselectedId;
bool g_aptDialogActive = false;
HFONT g_hAptFont = nullptr;
HFONT g_hAptBoldFont = nullptr;
HFONT g_hAptTitleFont = nullptr;

std::string getTomorrowDateStr() {
    time_t t = time(nullptr);
    t += 24 * 60 * 60; // +1 day
    tm localTm;
    localtime_s(&localTm, &t);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &localTm);
    return std::string(buf);
}

std::string trimStr(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void updatePatientNotice(HWND hwnd) {
    HWND hCombo = GetDlgItem(hwnd, IDC_APT_DLG_PAT_COMBO);
    int sel = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
    HWND hNotice = GetDlgItem(hwnd, IDC_APT_DLG_NOTICE_LBL);

    if (sel >= 0 && sel < static_cast<int>(g_patientsList.size())) {
        const auto& p = g_patientsList[sel];
        if (p->getPatientType() == "Emergency") {
            SetWindowTextA(hNotice, "(!) WARNING: Emergency patients require immediate triage rather than routine booking.");
        } else {
            std::string feeStr = "Standard Consultation Fee: $" + std::to_string((int)p->calculateConsultationFee()) + ".00";
            SetWindowTextA(hNotice, feeStr.c_str());
        }
    } else {
        SetWindowTextA(hNotice, "");
    }
}

LRESULT CALLBACK AppointmentDialogWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

            // Title
            HWND hTitle = CreateWindowExA(0, "STATIC", "Schedule New Clinical Appointment",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                20, 15, 440, 24, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_TITLE, hInst, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)g_hAptTitleFont, TRUE);

            // Patient Selection
            HWND hPatLbl = CreateWindowExA(0, "STATIC", "Select Patient (*):",
                WS_CHILD | WS_VISIBLE, 20, 52, 140, 20, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_PAT_LBL, hInst, NULL);
            SendMessage(hPatLbl, WM_SETFONT, (WPARAM)g_hAptBoldFont, TRUE);

            HWND hPatCombo = CreateWindowExA(0, "COMBOBOX", "",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                160, 50, 300, 240, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_PAT_COMBO, hInst, NULL);
            SendMessage(hPatCombo, WM_SETFONT, (WPARAM)g_hAptFont, TRUE);

            int selectedIndex = 0;
            for (size_t i = 0; i < g_patientsList.size(); ++i) {
                const auto& p = g_patientsList[i];
                std::string itemText = "[" + p->getId() + "] " + p->getName() + " (" + p->getPatientType() + ")";
                SendMessageA(hPatCombo, CB_ADDSTRING, 0, (LPARAM)itemText.c_str());
                if (!g_preselectedId.empty() && p->getId() == g_preselectedId) {
                    selectedIndex = (int)i;
                }
            }
            if (!g_patientsList.empty()) {
                SendMessageA(hPatCombo, CB_SETCURSEL, selectedIndex, 0);
            }

            // Notice / Warning label
            HWND hNotice = CreateWindowExA(0, "STATIC", "",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                160, 80, 300, 32, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_NOTICE_LBL, hInst, NULL);
            SendMessage(hNotice, WM_SETFONT, (WPARAM)g_hAptFont, TRUE);

            // Date
            HWND hDateLbl = CreateWindowExA(0, "STATIC", "Date (YYYY-MM-DD):",
                WS_CHILD | WS_VISIBLE, 20, 120, 140, 20, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_DATE_LBL, hInst, NULL);
            SendMessage(hDateLbl, WM_SETFONT, (WPARAM)g_hAptBoldFont, TRUE);

            std::string defaultDate = getTomorrowDateStr();
            HWND hDateEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", defaultDate.c_str(),
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
                160, 118, 140, 24, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_DATE_EDIT, hInst, NULL);
            SendMessage(hDateEdit, WM_SETFONT, (WPARAM)g_hAptFont, TRUE);

            // Time
            HWND hTimeLbl = CreateWindowExA(0, "STATIC", "Time Slot (*):",
                WS_CHILD | WS_VISIBLE, 20, 155, 140, 20, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_TIME_LBL, hInst, NULL);
            SendMessage(hTimeLbl, WM_SETFONT, (WPARAM)g_hAptBoldFont, TRUE);

            HWND hTimeCombo = CreateWindowExA(0, "COMBOBOX", "",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_VSCROLL | WS_TABSTOP,
                160, 153, 140, 180, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_TIME_COMBO, hInst, NULL);
            SendMessage(hTimeCombo, WM_SETFONT, (WPARAM)g_hAptFont, TRUE);
            const char* timeSlots[] = { "09:00", "09:30", "10:00", "10:30", "11:00", "11:30",
                                        "13:00", "13:30", "14:00", "14:30", "15:00", "15:30", "16:00" };
            for (const char* t : timeSlots) {
                SendMessageA(hTimeCombo, CB_ADDSTRING, 0, (LPARAM)t);
            }
            SendMessageA(hTimeCombo, CB_SETCURSEL, 3, 0); // 10:30 default

            // Department
            HWND hDeptLbl = CreateWindowExA(0, "STATIC", "Department (*):",
                WS_CHILD | WS_VISIBLE, 20, 190, 140, 20, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_DEPT_LBL, hInst, NULL);
            SendMessage(hDeptLbl, WM_SETFONT, (WPARAM)g_hAptBoldFont, TRUE);

            HWND hDeptCombo = CreateWindowExA(0, "COMBOBOX", "",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                160, 188, 300, 180, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_DEPT_COMBO, hInst, NULL);
            SendMessage(hDeptCombo, WM_SETFONT, (WPARAM)g_hAptFont, TRUE);
            const char* depts[] = {
                "General Practice", "Cardiology", "Pediatrics",
                "Internal Medicine", "General Surgery", "Orthopedics",
                "ENT (Ear, Nose & Throat)", "Ophthalmology"
            };
            for (const char* d : depts) {
                SendMessageA(hDeptCombo, CB_ADDSTRING, 0, (LPARAM)d);
            }
            SendMessageA(hDeptCombo, CB_SETCURSEL, 0, 0);

            // Attending Doctor
            HWND hDocLbl = CreateWindowExA(0, "STATIC", "Attending Doctor (*):",
                WS_CHILD | WS_VISIBLE, 20, 225, 140, 20, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_DOC_LBL, hInst, NULL);
            SendMessage(hDocLbl, WM_SETFONT, (WPARAM)g_hAptBoldFont, TRUE);

            HWND hDocCombo = CreateWindowExA(0, "COMBOBOX", "",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                160, 223, 300, 180, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_DOC_COMBO, hInst, NULL);
            SendMessage(hDocCombo, WM_SETFONT, (WPARAM)g_hAptFont, TRUE);
            const char* doctors[] = {
                "Dr. Adebayo", "Dr. Eze", "Dr. Williams",
                "Dr. Okon", "Dr. Bello", "Dr. Fatimah"
            };
            for (const char* doc : doctors) {
                SendMessageA(hDocCombo, CB_ADDSTRING, 0, (LPARAM)doc);
            }
            SendMessageA(hDocCombo, CB_SETCURSEL, 0, 0);

            // Notes
            HWND hNotesLbl = CreateWindowExA(0, "STATIC", "Clinical Notes / Reason:",
                WS_CHILD | WS_VISIBLE, 20, 260, 140, 20, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_NOTES_LBL, hInst, NULL);
            SendMessage(hNotesLbl, WM_SETFONT, (WPARAM)g_hAptBoldFont, TRUE);

            HWND hNotesEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Routine consultation and health review",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
                160, 258, 300, 24, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_NOTES_EDIT, hInst, NULL);
            SendMessage(hNotesEdit, WM_SETFONT, (WPARAM)g_hAptFont, TRUE);

            // Action Buttons
            HWND hBtnOk = CreateWindowExA(0, "BUTTON", "Book Appointment",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
                210, 310, 140, 32, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_BTN_OK, hInst, NULL);
            SendMessage(hBtnOk, WM_SETFONT, (WPARAM)g_hAptBoldFont, TRUE);

            HWND hBtnCancel = CreateWindowExA(0, "BUTTON", "Cancel",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                360, 310, 100, 32, hwnd, (HMENU)(UINT_PTR)IDC_APT_DLG_BTN_CANCEL, hInst, NULL);
            SendMessage(hBtnCancel, WM_SETFONT, (WPARAM)g_hAptFont, TRUE);

            updatePatientNotice(hwnd);
            return 0;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);

            if (id == IDC_APT_DLG_PAT_COMBO && code == CBN_SELCHANGE) {
                updatePatientNotice(hwnd);
                return 0;
            }

            if (id == IDC_APT_DLG_BTN_OK) {
                HWND hCombo = GetDlgItem(hwnd, IDC_APT_DLG_PAT_COMBO);
                int sel = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                if (sel < 0 || sel >= static_cast<int>(g_patientsList.size())) {
                    MessageBoxA(hwnd, "Validation Error: Please select a valid patient.", "Input Validation", MB_ICONERROR);
                    SetFocus(hCombo);
                    return 0;
                }

                const auto& pat = g_patientsList[sel];

                // Check Appointable capability
                if (pat->getPatientType() == "Emergency") {
                    std::string warnMsg = "Clinical Rule Violation:\n\n"
                                          "Patient '" + pat->getName() + "' is classified as an Emergency Patient.\n"
                                          "Emergency patients bypass advance scheduled appointments and must be admitted\n"
                                          "directly to the Waiting Queue for immediate triage.";
                    MessageBoxA(hwnd, warnMsg.c_str(), "Appointment Scheduling Restricted", MB_ICONWARNING);
                    return 0;
                }

                char bufDate[64] = {0};
                char bufTime[64] = {0};
                char bufDept[128] = {0};
                char bufDoc[128] = {0};
                char bufNotes[256] = {0};

                GetWindowTextA(GetDlgItem(hwnd, IDC_APT_DLG_DATE_EDIT), bufDate, sizeof(bufDate));
                GetWindowTextA(GetDlgItem(hwnd, IDC_APT_DLG_TIME_COMBO), bufTime, sizeof(bufTime));
                GetWindowTextA(GetDlgItem(hwnd, IDC_APT_DLG_DEPT_COMBO), bufDept, sizeof(bufDept));
                GetWindowTextA(GetDlgItem(hwnd, IDC_APT_DLG_DOC_COMBO), bufDoc, sizeof(bufDoc));
                GetWindowTextA(GetDlgItem(hwnd, IDC_APT_DLG_NOTES_EDIT), bufNotes, sizeof(bufNotes));

                std::string date = trimStr(bufDate);
                std::string time = trimStr(bufTime);
                std::string dept = trimStr(bufDept);
                std::string doc = trimStr(bufDoc);
                std::string notes = trimStr(bufNotes);

                if (date.empty()) {
                    MessageBoxA(hwnd, "Validation Error: Appointment date cannot be blank.", "Input Validation", MB_ICONERROR);
                    SetFocus(GetDlgItem(hwnd, IDC_APT_DLG_DATE_EDIT));
                    return 0;
                }

                if (time.empty()) {
                    MessageBoxA(hwnd, "Validation Error: Appointment time cannot be blank.", "Input Validation", MB_ICONERROR);
                    SetFocus(GetDlgItem(hwnd, IDC_APT_DLG_TIME_COMBO));
                    return 0;
                }

                g_aptDialogResult.confirmed = true;
                g_aptDialogResult.patientId = pat->getId();
                g_aptDialogResult.patientName = pat->getName();
                g_aptDialogResult.date = date;
                g_aptDialogResult.time = time;
                g_aptDialogResult.department = dept.empty() ? "General Practice" : dept;
                g_aptDialogResult.doctor = doc.empty() ? "Dr. Adebayo" : doc;
                g_aptDialogResult.notes = notes.empty() ? "Routine consultation" : notes;

                g_aptDialogActive = false;
                DestroyWindow(hwnd);
                return 0;
            }

            if (id == IDC_APT_DLG_BTN_CANCEL) {
                g_aptDialogResult.confirmed = false;
                g_aptDialogActive = false;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }

        case WM_CLOSE: {
            g_aptDialogResult.confirmed = false;
            g_aptDialogActive = false;
            DestroyWindow(hwnd);
            return 0;
        }

        case WM_DESTROY: {
            g_aptDialogActive = false;
            return 0;
        }
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void registerAptDialogClass(HINSTANCE hInst) {
    static bool registered = false;
    if (registered) return;

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = AppointmentDialogWndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "SCAPMS_AppointmentDialogClass";

    RegisterClassExA(&wc);
    registered = true;
}

} // namespace

AppointmentDialogResult AppointmentDialog::showScheduleDialog(
    HWND parentHwnd,
    const std::vector<std::shared_ptr<model::PatientRecord>>& patients,
    const std::string& preselectedPatientId) {

    g_aptDialogResult = AppointmentDialogResult();
    g_patientsList = patients;
    g_preselectedId = preselectedPatientId;

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(parentHwnd, GWLP_HINSTANCE);
    if (!hInst) hInst = GetModuleHandle(NULL);

    registerAptDialogClass(hInst);

    if (!g_hAptFont) {
        g_hAptFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    }
    if (!g_hAptBoldFont) {
        g_hAptBoldFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                     ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                     CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    }
    if (!g_hAptTitleFont) {
        g_hAptTitleFont = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    }

    EnableWindow(parentHwnd, FALSE);

    RECT rcParent;
    GetWindowRect(parentHwnd, &rcParent);
    int dlgW = 500;
    int dlgH = 400;
    int x = rcParent.left + (rcParent.right - rcParent.left - dlgW) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - dlgH) / 2;
    if (x < 0) x = 100;
    if (y < 0) y = 100;

    HWND hDlg = CreateWindowExA(
        WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT,
        "SCAPMS_AppointmentDialogClass",
        "Book Clinical Appointment - SCAPMS",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, dlgW, dlgH,
        parentHwnd, NULL, hInst, NULL
    );

    if (!hDlg) {
        EnableWindow(parentHwnd, TRUE);
        return g_aptDialogResult;
    }

    g_aptDialogActive = true;
    ShowWindow(hDlg, SW_SHOW);
    UpdateWindow(hDlg);

    MSG msg;
    while (g_aptDialogActive && GetMessageA(&msg, NULL, 0, 0)) {
        if (!IsDialogMessageA(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    EnableWindow(parentHwnd, TRUE);
    SetForegroundWindow(parentHwnd);
    SetFocus(parentHwnd);

    return g_aptDialogResult;
}

} // namespace gui
} // namespace scapms

