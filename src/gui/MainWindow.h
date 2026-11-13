#ifndef SCAPMS_GUI_MAINWINDOW_H
#define SCAPMS_GUI_MAINWINDOW_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>
#include "../controller/ClinicManager.h"
#include "LayoutManager.h"
#include "GuiConstants.h"

#include <string>
#include <vector>
#include <memory>

namespace scapms {
namespace gui {

/**
 * @brief Main Graphical User Interface Window (Mandatory Section 7).
 *        Orchestrates 6 multi-panel tabs, event-driven menus/buttons/text/timers,
 *        custom layouts, list views, and dialog boxes.
 */
class MainWindow {
private:
    HWND m_hwndMain{nullptr};
    HWND m_hTabControl{nullptr};
    HWND m_hStatusBar{nullptr};
    HFONT m_hFont{nullptr};
    HFONT m_hBoldFont{nullptr};
    HFONT m_hTitleFont{nullptr};
    HACCEL m_hAccel{nullptr};

    // Controller
    controller::ClinicManager m_manager;

    // Tab Containers (6 major panels)
    HWND m_panels[6]{nullptr};
    int m_currentTabIndex{0};

    // --- Tab 0: Dashboard Controls ---
    HWND m_hDashKpiCards[5]{nullptr};
    HWND m_hDashRecentList{nullptr};
    HWND m_hDashAlertText{nullptr};
    HWND m_hDashBtnAddPat{nullptr};
    HWND m_hDashBtnAddApt{nullptr};
    HWND m_hDashBtnCallNext{nullptr};

    // --- Tab 1: Patient Records Controls ---
    HWND m_hPatientList{nullptr};
    HWND m_hPatientFilterEdit{nullptr};
    HWND m_hBtnAddPatient{nullptr};
    HWND m_hBtnEditPatient{nullptr};
    HWND m_hBtnDeletePatient{nullptr};
    HWND m_hBtnQueuePatient{nullptr};
    HWND m_hBtnUndoPatient{nullptr};

    // --- Tab 2: Appointment Controls ---
    HWND m_hAppointmentList{nullptr};
    HWND m_hBtnScheduleApt{nullptr};
    HWND m_hBtnCompleteApt{nullptr};
    HWND m_hBtnCancelApt{nullptr};
    HWND m_hBtnDeleteApt{nullptr};
    HWND m_hBtnUndoApt{nullptr};

    // --- Tab 3: Waiting Queue Controls ---
    HWND m_hQueueBanner{nullptr};
    HWND m_hQueueList{nullptr};
    HWND m_hBtnCallNextQueue{nullptr};
    HWND m_hBtnAddSelectedToQueue{nullptr};

    // --- Tab 4: Search & Sort Controls ---
    HWND m_hSearchEdit{nullptr};
    HWND m_hComboSearchAlgo{nullptr};
    HWND m_hComboSearchField{nullptr};
    HWND m_hBtnExecuteSearch{nullptr};
    HWND m_hComboSortAlgo{nullptr};
    HWND m_hComboSortField{nullptr};
    HWND m_hComboSortOrder{nullptr};
    HWND m_hBtnExecuteSort{nullptr};
    HWND m_hSearchResultsList{nullptr};
    HWND m_hSearchMetricsText{nullptr};

    // --- Tab 5: Reports Controls ---
    HWND m_hComboReportType{nullptr};
    HWND m_hBtnGenerateReport{nullptr};
    HWND m_hBtnExportReport{nullptr};
    HWND m_hReportTextView{nullptr};

    // State
    std::string m_currentlyServingPatientName{"None (Waiting for Next Patient)"};
    std::string m_lastSortReason;

public:
    MainWindow();
    ~MainWindow();

    bool initialize(HINSTANCE hInstance, int nCmdShow);
    int runMessageLoop();

    HWND getHWND() const { return m_hwndMain; }
    controller::ClinicManager& getManager() { return m_manager; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK PanelSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void createMenuBar();
    void createStatusBar();
    void createTabs();

    // Panel creation
    void createDashboardPanel();
    void createPatientPanel();
    void createAppointmentPanel();
    void createQueuePanel();
    void createSearchPanel();
    void createReportPanel();

    // Layout resizing
    void onResize(int width, int height);
    void layoutDashboardPanel(int w, int h);
    void layoutPatientPanel(int w, int h);
    void layoutAppointmentPanel(int w, int h);
    void layoutQueuePanel(int w, int h);
    void layoutSearchPanel(int w, int h);
    void layoutReportPanel(int w, int h);

    // Event Handlers
    void onTabChanged(int newIndex);
    void onTimerTick(UINT_PTR timerId);

    // Refresh views
    void refreshAllViews();
    void refreshDashboard();
    void refreshPatientList(const std::string& filter = "");
    void refreshAppointmentList();
    void refreshQueueList();
    void refreshCacheList();
    void refreshStatusBar();

    // Patient Actions
    void onAddPatientClicked();
    void onEditPatientClicked();
    void onDeletePatientClicked();
    void onQueuePatientClicked();
    void onUndoClicked();

    // Appointment Actions
    void onScheduleAppointmentClicked();
    void onCancelAppointmentClicked();
    void onCompleteAppointmentClicked();
    void onDeleteAppointmentClicked();

    // Queue Actions
    void onCallNextPatientClicked();

    // Search & Sort Actions
    void onSearchClicked();
    void onSortClicked();

    // Report Actions
    void onGenerateReportClicked();
    void onExportReportClicked();

    // File Actions (Requirement 9.c: File Dialog)
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileExport();

    // Helpers
    void populatePatientListView(HWND hList, const std::vector<std::shared_ptr<model::PatientRecord>>& patients);
    void addListViewColumn(HWND hList, int colIndex, const char* text, int width);
    void setControlFont(HWND hwndControl, HFONT hFont);
};

} // namespace gui
} // namespace scapms

#endif // SCAPMS_GUI_MAINWINDOW_H
