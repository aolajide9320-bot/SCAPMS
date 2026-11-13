#ifndef SCAPMS_GUI_GUICONSTANTS_H
#define SCAPMS_GUI_GUICONSTANTS_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>

namespace scapms {
namespace gui {

// Window and Control IDs
constexpr int ID_MAIN_TAB = 1001;
constexpr int ID_STATUS_BAR = 1002;

// Menu IDs
constexpr int IDM_FILE_NEW = 2001;
constexpr int IDM_FILE_OPEN = 2002;
constexpr int IDM_FILE_SAVE = 2003;
constexpr int IDM_FILE_EXPORT = 2004;
constexpr int IDM_FILE_EXIT = 2005;
constexpr int IDM_EDIT_UNDO = 2006;
constexpr int IDM_VIEW_REFRESH = 2007;
constexpr int IDM_HELP_ABOUT = 2008;

// Accelerator IDs
constexpr int IDA_SAVE = 3001;
constexpr int IDA_SEARCH = 3002;
constexpr int IDA_EXIT = 3003;
constexpr int IDA_UNDO = 3004;

// Timer IDs
constexpr UINT_PTR TIMER_CLOCK_ID = 4001;
constexpr UINT_PTR TIMER_DASHBOARD_ID = 4002;

// Patient Tab Controls
constexpr int IDC_PATIENT_LIST = 5001;
constexpr int IDC_BTN_ADD_PATIENT = 5002;
constexpr int IDC_BTN_EDIT_PATIENT = 5003;
constexpr int IDC_BTN_DELETE_PATIENT = 5004;
constexpr int IDC_BTN_QUEUE_PATIENT = 5005;
constexpr int IDC_BTN_UNDO_PATIENT = 5006;
constexpr int IDC_PATIENT_FILTER_EDIT = 5007;

// Appointment Tab Controls
constexpr int IDC_APPOINTMENT_LIST = 6001;
constexpr int IDC_BTN_SCHEDULE_APPOINTMENT = 6002;
constexpr int IDC_BTN_CANCEL_APPOINTMENT = 6003;
constexpr int IDC_BTN_COMPLETE_APPOINTMENT = 6004;
constexpr int IDC_BTN_DELETE_APPOINTMENT = 6005;
constexpr int IDC_BTN_UNDO_APPOINTMENT = 6006;

// Queue Tab Controls
constexpr int IDC_QUEUE_LIST = 7001;
constexpr int IDC_BTN_CALL_NEXT = 7002;
constexpr int IDC_BTN_ENQUEUE_SELECTED = 7003;
constexpr int IDC_STATIC_NOW_SERVING = 7004;

// Search & Sort Tab Controls
constexpr int IDC_SEARCH_EDIT = 8001;
constexpr int IDC_COMBO_SEARCH_ALGO = 8002;
constexpr int IDC_COMBO_SEARCH_FIELD = 8003;
constexpr int IDC_BTN_DO_SEARCH = 8004;
constexpr int IDC_COMBO_SORT_ALGO = 8005;
constexpr int IDC_COMBO_SORT_FIELD = 8006;
constexpr int IDC_COMBO_SORT_ORDER = 8007;
constexpr int IDC_BTN_DO_SORT = 8008;
constexpr int IDC_SEARCH_RESULTS_LIST = 8009;
constexpr int IDC_STATIC_METRICS = 8010;

// Reports Tab Controls
constexpr int IDC_COMBO_REPORT_TYPE = 9001;
constexpr int IDC_BTN_GENERATE_REPORT = 9002;
constexpr int IDC_BTN_EXPORT_REPORT = 9003;
constexpr int IDC_REPORT_TEXT_VIEW = 9004;

// Dashboard Controls
constexpr int IDC_DASHBOARD_TEXT = 10001;
constexpr int IDC_RECENT_CACHE_LIST = 10002;
constexpr int IDC_BTN_DASH_ADD_PAT = 10003;
constexpr int IDC_BTN_DASH_ADD_APT = 10004;
constexpr int IDC_BTN_DASH_CALL_NEXT = 10005;

} // namespace gui
} // namespace scapms

#endif // SCAPMS_GUI_GUICONSTANTS_H
