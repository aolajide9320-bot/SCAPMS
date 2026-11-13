#include "PatientDialog.h"
#include "../model/GeneralPatient.h"
#include "../model/EmergencyPatient.h"
#include "../model/FollowUpPatient.h"
#include <commctrl.h>
#include <algorithm>
#include <sstream>

namespace scapms {
namespace gui {

namespace {

// Control IDs
constexpr int IDC_DLG_TITLE = 200;
constexpr int IDC_DLG_CAT_LBL = 201;
constexpr int IDC_DLG_CAT_COMBO = 202;
constexpr int IDC_DLG_NAME_LBL = 203;
constexpr int IDC_DLG_NAME_EDIT = 204;
constexpr int IDC_DLG_AGE_LBL = 205;
constexpr int IDC_DLG_AGE_EDIT = 206;
constexpr int IDC_DLG_CONTACT_LBL = 207;
constexpr int IDC_DLG_CONTACT_EDIT = 208;

// GroupBox
constexpr int IDC_DLG_GROUP_DETAILS = 209;

// General controls
constexpr int IDC_DLG_GEN_SYMP_LBL = 210;
constexpr int IDC_DLG_GEN_SYMP_EDIT = 211;
constexpr int IDC_DLG_GEN_ROUTINE_CHK = 212;

// Emergency controls
constexpr int IDC_DLG_EMERG_SEV_LBL = 213;
constexpr int IDC_DLG_EMERG_SEV_COMBO = 214;
constexpr int IDC_DLG_EMERG_CONTACT_LBL = 215;
constexpr int IDC_DLG_EMERG_CONTACT_EDIT = 216;

// Follow-up controls
constexpr int IDC_DLG_FLP_DATE_LBL = 217;
constexpr int IDC_DLG_FLP_DATE_EDIT = 218;
constexpr int IDC_DLG_FLP_DOC_LBL = 219;
constexpr int IDC_DLG_FLP_DOC_EDIT = 220;
constexpr int IDC_DLG_FLP_REASON_LBL = 221;
constexpr int IDC_DLG_FLP_REASON_EDIT = 222;

// Action Buttons
constexpr int IDC_DLG_BTN_OK = IDOK;
constexpr int IDC_DLG_BTN_CANCEL = IDCANCEL;

// Dialog state
PatientDialogResult g_dialogResult;
bool g_isEditMode = false;
std::shared_ptr<model::PatientRecord> g_editPatient = nullptr;
bool g_dialogActive = false;
HFONT g_hDlgFont = nullptr;
HFONT g_hDlgBoldFont = nullptr;
HFONT g_hDlgTitleFont = nullptr;

void updateCategoryVisibility(HWND hwnd) {
    HWND hCombo = GetDlgItem(hwnd, IDC_DLG_CAT_COMBO);
    int sel = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);

    // 0 = General, 1 = Emergency, 2 = Follow-Up
    int showGeneral = (sel == 0) ? SW_SHOW : SW_HIDE;
    int showEmergency = (sel == 1) ? SW_SHOW : SW_HIDE;
    int showFollowUp = (sel == 2) ? SW_SHOW : SW_HIDE;

    // General
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_GEN_SYMP_LBL), showGeneral);
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_GEN_SYMP_EDIT), showGeneral);
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_GEN_ROUTINE_CHK), showGeneral);

    // Emergency
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_EMERG_SEV_LBL), showEmergency);
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_EMERG_SEV_COMBO), showEmergency);
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_EMERG_CONTACT_LBL), showEmergency);
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_EMERG_CONTACT_EDIT), showEmergency);

    // Follow-up
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_FLP_DATE_LBL), showFollowUp);
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_FLP_DATE_EDIT), showFollowUp);
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_FLP_DOC_LBL), showFollowUp);
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_FLP_DOC_EDIT), showFollowUp);
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_FLP_REASON_LBL), showFollowUp);
    ShowWindow(GetDlgItem(hwnd, IDC_DLG_FLP_REASON_EDIT), showFollowUp);
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

LRESULT CALLBACK PatientDialogWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

            // Title
            HWND hTitle = CreateWindowExA(0, "STATIC",
                g_isEditMode ? "Modify Patient Clinical Profile" : "Register New Patient Record",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                20, 15, 440, 24, hwnd, (HMENU)(UINT_PTR)IDC_DLG_TITLE, hInst, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)g_hDlgTitleFont, TRUE);

            // Category
            HWND hCatLbl = CreateWindowExA(0, "STATIC", "Patient Category (*):",
                WS_CHILD | WS_VISIBLE, 20, 50, 140, 20, hwnd, (HMENU)(UINT_PTR)IDC_DLG_CAT_LBL, hInst, NULL);
            SendMessage(hCatLbl, WM_SETFONT, (WPARAM)g_hDlgBoldFont, TRUE);

            HWND hCatCombo = CreateWindowExA(0, "COMBOBOX", "",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
                170, 48, 280, 150, hwnd, (HMENU)(UINT_PTR)IDC_DLG_CAT_COMBO, hInst, NULL);
            SendMessage(hCatCombo, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);
            SendMessageA(hCatCombo, CB_ADDSTRING, 0, (LPARAM)"General Outpatient");
            SendMessageA(hCatCombo, CB_ADDSTRING, 0, (LPARAM)"Emergency Patient (High Priority)");
            SendMessageA(hCatCombo, CB_ADDSTRING, 0, (LPARAM)"Follow-Up Patient");
            SendMessageA(hCatCombo, CB_SETCURSEL, 0, 0);

            // Patient Name
            HWND hNameLbl = CreateWindowExA(0, "STATIC", "Full Name (*):",
                WS_CHILD | WS_VISIBLE, 20, 85, 140, 20, hwnd, (HMENU)(UINT_PTR)IDC_DLG_NAME_LBL, hInst, NULL);
            SendMessage(hNameLbl, WM_SETFONT, (WPARAM)g_hDlgBoldFont, TRUE);

            HWND hNameEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
                170, 83, 280, 24, hwnd, (HMENU)(UINT_PTR)IDC_DLG_NAME_EDIT, hInst, NULL);
            SendMessage(hNameEdit, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            // Age
            HWND hAgeLbl = CreateWindowExA(0, "STATIC", "Age (0 - 120) (*):",
                WS_CHILD | WS_VISIBLE, 20, 120, 140, 20, hwnd, (HMENU)(UINT_PTR)IDC_DLG_AGE_LBL, hInst, NULL);
            SendMessage(hAgeLbl, WM_SETFONT, (WPARAM)g_hDlgBoldFont, TRUE);

            HWND hAgeEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_NUMBER | WS_TABSTOP,
                170, 118, 100, 24, hwnd, (HMENU)(UINT_PTR)IDC_DLG_AGE_EDIT, hInst, NULL);
            SendMessage(hAgeEdit, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            // Contact
            HWND hContactLbl = CreateWindowExA(0, "STATIC", "Contact / Phone (*):",
                WS_CHILD | WS_VISIBLE, 20, 155, 140, 20, hwnd, (HMENU)(UINT_PTR)IDC_DLG_CONTACT_LBL, hInst, NULL);
            SendMessage(hContactLbl, WM_SETFONT, (WPARAM)g_hDlgBoldFont, TRUE);

            HWND hContactEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
                170, 153, 280, 24, hwnd, (HMENU)(UINT_PTR)IDC_DLG_CONTACT_EDIT, hInst, NULL);
            SendMessage(hContactEdit, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            // GroupBox for Category Specific Fields
            HWND hGrp = CreateWindowExA(0, "BUTTON", "Category-Specific Clinical Information",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                20, 195, 430, 200, hwnd, (HMENU)(UINT_PTR)IDC_DLG_GROUP_DETAILS, hInst, NULL);
            SendMessage(hGrp, WM_SETFONT, (WPARAM)g_hDlgBoldFont, TRUE);

            // --- General controls ---
            HWND hSymLbl = CreateWindowExA(0, "STATIC", "Symptoms / Notes:",
                WS_CHILD, 35, 230, 130, 20, hwnd, (HMENU)(UINT_PTR)IDC_DLG_GEN_SYMP_LBL, hInst, NULL);
            SendMessage(hSymLbl, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            HWND hSymEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | ES_AUTOHSCROLL | WS_TABSTOP,
                170, 228, 260, 24, hwnd, (HMENU)(UINT_PTR)IDC_DLG_GEN_SYMP_EDIT, hInst, NULL);
            SendMessage(hSymEdit, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            HWND hChkRoutine = CreateWindowExA(0, "BUTTON", "Routine Preventive Checkup",
                WS_CHILD | BS_AUTOCHECKBOX | WS_TABSTOP,
                170, 265, 260, 24, hwnd, (HMENU)(UINT_PTR)IDC_DLG_GEN_ROUTINE_CHK, hInst, NULL);
            SendMessage(hChkRoutine, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);
            SendMessage(hChkRoutine, BM_SETCHECK, BST_CHECKED, 0);

            // --- Emergency controls ---
            HWND hSevLbl = CreateWindowExA(0, "STATIC", "Triage Severity (*):",
                WS_CHILD, 35, 230, 130, 20, hwnd, (HMENU)(UINT_PTR)IDC_DLG_EMERG_SEV_LBL, hInst, NULL);
            SendMessage(hSevLbl, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            HWND hSevCombo = CreateWindowExA(0, "COMBOBOX", "",
                WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP,
                170, 228, 260, 150, hwnd, (HMENU)(UINT_PTR)IDC_DLG_EMERG_SEV_COMBO, hInst, NULL);
            SendMessage(hSevCombo, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);
            SendMessageA(hSevCombo, CB_ADDSTRING, 0, (LPARAM)"1 - Critical (Immediate Resuscitation)");
            SendMessageA(hSevCombo, CB_ADDSTRING, 0, (LPARAM)"2 - Emergent (High Priority Care)");
            SendMessageA(hSevCombo, CB_ADDSTRING, 0, (LPARAM)"3 - Urgent (Serious Assessment)");
            SendMessageA(hSevCombo, CB_ADDSTRING, 0, (LPARAM)"4 - Semi-Urgent (Standard Assessment)");
            SendMessageA(hSevCombo, CB_ADDSTRING, 0, (LPARAM)"5 - Non-Urgent (Minor Condition)");
            SendMessageA(hSevCombo, CB_SETCURSEL, 1, 0);

            HWND hEmContactLbl = CreateWindowExA(0, "STATIC", "Emergency Contact (*):",
                WS_CHILD, 35, 275, 130, 20, hwnd, (HMENU)(UINT_PTR)IDC_DLG_EMERG_CONTACT_LBL, hInst, NULL);
            SendMessage(hEmContactLbl, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            HWND hEmContactEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | ES_AUTOHSCROLL | WS_TABSTOP,
                170, 273, 260, 24, hwnd, (HMENU)(UINT_PTR)IDC_DLG_EMERG_CONTACT_EDIT, hInst, NULL);
            SendMessage(hEmContactEdit, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            // --- Follow-Up controls ---
            HWND hDateLbl = CreateWindowExA(0, "STATIC", "Previous Visit Date:",
                WS_CHILD, 35, 225, 130, 20, hwnd, (HMENU)(UINT_PTR)IDC_DLG_FLP_DATE_LBL, hInst, NULL);
            SendMessage(hDateLbl, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            HWND hDateEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "2026-10-25",
                WS_CHILD | ES_AUTOHSCROLL | WS_TABSTOP,
                170, 223, 260, 24, hwnd, (HMENU)(UINT_PTR)IDC_DLG_FLP_DATE_EDIT, hInst, NULL);
            SendMessage(hDateEdit, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            HWND hDocLbl = CreateWindowExA(0, "STATIC", "Attending Doctor:",
                WS_CHILD, 35, 260, 130, 20, hwnd, (HMENU)(UINT_PTR)IDC_DLG_FLP_DOC_LBL, hInst, NULL);
            SendMessage(hDocLbl, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            HWND hDocEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Dr. Adebayo",
                WS_CHILD | ES_AUTOHSCROLL | WS_TABSTOP,
                170, 258, 260, 24, hwnd, (HMENU)(UINT_PTR)IDC_DLG_FLP_DOC_EDIT, hInst, NULL);
            SendMessage(hDocEdit, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            HWND hReasLbl = CreateWindowExA(0, "STATIC", "Follow-Up Reason:",
                WS_CHILD, 35, 295, 130, 20, hwnd, (HMENU)(UINT_PTR)IDC_DLG_FLP_REASON_LBL, hInst, NULL);
            SendMessage(hReasLbl, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            HWND hReasEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Post-treatment review",
                WS_CHILD | ES_AUTOHSCROLL | WS_TABSTOP,
                170, 293, 260, 24, hwnd, (HMENU)(UINT_PTR)IDC_DLG_FLP_REASON_EDIT, hInst, NULL);
            SendMessage(hReasEdit, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            // --- Action Buttons ---
            HWND hBtnOk = CreateWindowExA(0, "BUTTON",
                g_isEditMode ? "Save Changes" : "Save Record",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
                220, 420, 120, 32, hwnd, (HMENU)(UINT_PTR)IDC_DLG_BTN_OK, hInst, NULL);
            SendMessage(hBtnOk, WM_SETFONT, (WPARAM)g_hDlgBoldFont, TRUE);

            HWND hBtnCancel = CreateWindowExA(0, "BUTTON", "Cancel",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                350, 420, 100, 32, hwnd, (HMENU)(UINT_PTR)IDC_DLG_BTN_CANCEL, hInst, NULL);
            SendMessage(hBtnCancel, WM_SETFONT, (WPARAM)g_hDlgFont, TRUE);

            // If Edit Mode, populate existing patient values
            if (g_isEditMode && g_editPatient) {
                SetWindowTextA(hNameEdit, g_editPatient->getName().c_str());
                SetWindowTextA(hAgeEdit, std::to_string(g_editPatient->getAge()).c_str());
                SetWindowTextA(hContactEdit, g_editPatient->getContact().c_str());

                std::string pType = g_editPatient->getPatientType();
                if (pType == "General") {
                    SendMessageA(hCatCombo, CB_SETCURSEL, 0, 0);
                    if (auto gp = dynamic_cast<model::GeneralPatient*>(g_editPatient.get())) {
                        SetWindowTextA(hSymEdit, gp->getSymptoms().c_str());
                        SendMessage(hChkRoutine, BM_SETCHECK, gp->isRoutineCheckup() ? BST_CHECKED : BST_UNCHECKED, 0);
                    }
                } else if (pType == "Emergency") {
                    SendMessageA(hCatCombo, CB_SETCURSEL, 1, 0);
                    if (auto ep = dynamic_cast<model::EmergencyPatient*>(g_editPatient.get())) {
                        SendMessageA(hSevCombo, CB_SETCURSEL, ep->getSeverityLevel() - 1, 0);
                        SetWindowTextA(hEmContactEdit, ep->getEmergencyContact().c_str());
                    }
                } else if (pType == "Follow-Up") {
                    SendMessageA(hCatCombo, CB_SETCURSEL, 2, 0);
                    if (auto fp = dynamic_cast<model::FollowUpPatient*>(g_editPatient.get())) {
                        SetWindowTextA(hDateEdit, fp->getPreviousVisitDate().c_str());
                        SetWindowTextA(hDocEdit, fp->getAttendingDoctor().c_str());
                        SetWindowTextA(hReasEdit, fp->getFollowUpReason().c_str());
                    }
                }
                // Lock category dropdown in edit mode to preserve concrete subclass type
                EnableWindow(hCatCombo, FALSE);
            }

            updateCategoryVisibility(hwnd);
            SetFocus(hNameEdit);
            return 0;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);

            if (id == IDC_DLG_CAT_COMBO && code == CBN_SELCHANGE) {
                updateCategoryVisibility(hwnd);
                return 0;
            }

            if (id == IDC_DLG_BTN_OK) {
                char bufName[256] = {0};
                char bufAge[32] = {0};
                char bufContact[256] = {0};

                GetWindowTextA(GetDlgItem(hwnd, IDC_DLG_NAME_EDIT), bufName, sizeof(bufName));
                GetWindowTextA(GetDlgItem(hwnd, IDC_DLG_AGE_EDIT), bufAge, sizeof(bufAge));
                GetWindowTextA(GetDlgItem(hwnd, IDC_DLG_CONTACT_EDIT), bufContact, sizeof(bufContact));

                std::string name = trim(bufName);
                std::string contact = trim(bufContact);

                // Name validation
                if (name.empty()) {
                    MessageBoxA(hwnd, "Validation Error: Patient Name cannot be blank.", "Input Validation", MB_ICONERROR);
                    SetFocus(GetDlgItem(hwnd, IDC_DLG_NAME_EDIT));
                    return 0;
                }

                // Age validation
                int age = -1;
                try {
                    age = std::stoi(trim(bufAge));
                } catch (...) {
                    MessageBoxA(hwnd, "Validation Error: Please enter a valid numeric age.", "Input Validation", MB_ICONERROR);
                    SetFocus(GetDlgItem(hwnd, IDC_DLG_AGE_EDIT));
                    return 0;
                }

                if (age < 0 || age > 120) {
                    MessageBoxA(hwnd, "Validation Error: Patient age must be between 0 and 120 years.", "Input Validation", MB_ICONERROR);
                    SetFocus(GetDlgItem(hwnd, IDC_DLG_AGE_EDIT));
                    return 0;
                }

                // Contact validation
                if (contact.empty()) {
                    MessageBoxA(hwnd, "Validation Error: Contact phone/email cannot be blank.", "Input Validation", MB_ICONERROR);
                    SetFocus(GetDlgItem(hwnd, IDC_DLG_CONTACT_EDIT));
                    return 0;
                }

                int catSel = (int)SendMessage(GetDlgItem(hwnd, IDC_DLG_CAT_COMBO), CB_GETCURSEL, 0, 0);

                g_dialogResult.name = name;
                g_dialogResult.age = age;
                g_dialogResult.contact = contact;

                if (catSel == 0) { // General
                    g_dialogResult.category = "General";
                    char bufSym[256] = {0};
                    GetWindowTextA(GetDlgItem(hwnd, IDC_DLG_GEN_SYMP_EDIT), bufSym, sizeof(bufSym));
                    std::string sym = trim(bufSym);
                    g_dialogResult.symptoms = sym.empty() ? "None (Routine Checkup)" : sym;
                    g_dialogResult.routineCheckup = (SendMessage(GetDlgItem(hwnd, IDC_DLG_GEN_ROUTINE_CHK), BM_GETCHECK, 0, 0) == BST_CHECKED);
                } else if (catSel == 1) { // Emergency
                    g_dialogResult.category = "Emergency";
                    int sevSel = (int)SendMessage(GetDlgItem(hwnd, IDC_DLG_EMERG_SEV_COMBO), CB_GETCURSEL, 0, 0);
                    g_dialogResult.severityLevel = (sevSel >= 0) ? (sevSel + 1) : 2;

                    char bufEm[256] = {0};
                    GetWindowTextA(GetDlgItem(hwnd, IDC_DLG_EMERG_CONTACT_EDIT), bufEm, sizeof(bufEm));
                    std::string emContact = trim(bufEm);
                    if (emContact.empty()) {
                        MessageBoxA(hwnd, "Validation Error: Emergency contact details are mandatory for Emergency patients.", "Input Validation", MB_ICONERROR);
                        SetFocus(GetDlgItem(hwnd, IDC_DLG_EMERG_CONTACT_EDIT));
                        return 0;
                    }
                    g_dialogResult.emergencyContact = emContact;
                } else { // Follow-up
                    g_dialogResult.category = "Follow-Up";
                    char bufDate[64] = {0};
                    char bufDoc[128] = {0};
                    char bufReas[256] = {0};

                    GetWindowTextA(GetDlgItem(hwnd, IDC_DLG_FLP_DATE_EDIT), bufDate, sizeof(bufDate));
                    GetWindowTextA(GetDlgItem(hwnd, IDC_DLG_FLP_DOC_EDIT), bufDoc, sizeof(bufDoc));
                    GetWindowTextA(GetDlgItem(hwnd, IDC_DLG_FLP_REASON_EDIT), bufReas, sizeof(bufReas));

                    std::string pDate = trim(bufDate);
                    std::string pDoc = trim(bufDoc);
                    std::string pReas = trim(bufReas);

                    if (pDate.empty()) {
                        MessageBoxA(hwnd, "Validation Error: Previous visit date is required for follow-up patients.", "Input Validation", MB_ICONERROR);
                        SetFocus(GetDlgItem(hwnd, IDC_DLG_FLP_DATE_EDIT));
                        return 0;
                    }

                    g_dialogResult.previousVisitDate = pDate;
                    g_dialogResult.attendingDoctor = pDoc.empty() ? "Dr. Adebayo" : pDoc;
                    g_dialogResult.followUpReason = pReas.empty() ? "Post-treatment review" : pReas;
                }

                g_dialogResult.confirmed = true;
                g_dialogActive = false;
                DestroyWindow(hwnd);
                return 0;
            }

            if (id == IDC_DLG_BTN_CANCEL) {
                g_dialogResult.confirmed = false;
                g_dialogActive = false;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }

        case WM_CLOSE: {
            g_dialogResult.confirmed = false;
            g_dialogActive = false;
            DestroyWindow(hwnd);
            return 0;
        }

        case WM_DESTROY: {
            g_dialogActive = false;
            return 0;
        }
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void registerDialogClass(HINSTANCE hInst) {
    static bool registered = false;
    if (registered) return;

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = PatientDialogWndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "SCAPMS_PatientDialogClass";

    RegisterClassExA(&wc);
    registered = true;
}

PatientDialogResult runModalDialog(HWND parentHwnd, const std::string& title, bool isEdit, const std::shared_ptr<model::PatientRecord>& patient) {
    g_dialogResult = PatientDialogResult();
    g_isEditMode = isEdit;
    g_editPatient = patient;

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(parentHwnd, GWLP_HINSTANCE);
    if (!hInst) hInst = GetModuleHandle(NULL);

    registerDialogClass(hInst);

    // Create fonts
    if (!g_hDlgFont) {
        g_hDlgFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    }
    if (!g_hDlgBoldFont) {
        g_hDlgBoldFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                    CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    }
    if (!g_hDlgTitleFont) {
        g_hDlgTitleFont = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                     ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                     CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    }

    // Disable parent window to enforce modality
    EnableWindow(parentHwnd, FALSE);

    // Center dialog over parent
    RECT rcParent;
    GetWindowRect(parentHwnd, &rcParent);
    int dlgW = 490;
    int dlgH = 510;
    int x = rcParent.left + (rcParent.right - rcParent.left - dlgW) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - dlgH) / 2;
    if (x < 0) x = 100;
    if (y < 0) y = 100;

    HWND hDlg = CreateWindowExA(
        WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT,
        "SCAPMS_PatientDialogClass",
        title.c_str(),
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, dlgW, dlgH,
        parentHwnd, NULL, hInst, NULL
    );

    if (!hDlg) {
        EnableWindow(parentHwnd, TRUE);
        return g_dialogResult;
    }

    g_dialogActive = true;
    ShowWindow(hDlg, SW_SHOW);
    UpdateWindow(hDlg);

    // Modal message loop
    MSG msg;
    while (g_dialogActive && GetMessageA(&msg, NULL, 0, 0)) {
        if (!IsDialogMessageA(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    // Re-enable parent and restore focus
    EnableWindow(parentHwnd, TRUE);
    SetForegroundWindow(parentHwnd);
    SetFocus(parentHwnd);

    return g_dialogResult;
}

} // namespace

PatientDialogResult PatientDialog::showAddDialog(HWND parentHwnd) {
    return runModalDialog(parentHwnd, "Register New Patient Record - SCAPMS", false, nullptr);
}

PatientDialogResult PatientDialog::showEditDialog(HWND parentHwnd, const std::shared_ptr<model::PatientRecord>& patient) {
    std::string title = "Edit Patient Record [" + (patient ? patient->getId() : "") + "] - SCAPMS";
    return runModalDialog(parentHwnd, title, true, patient);
}

} // namespace gui
} // namespace scapms
