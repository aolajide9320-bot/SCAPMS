#ifndef SCAPMS_GUI_APPOINTMENTDIALOG_H
#define SCAPMS_GUI_APPOINTMENTDIALOG_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include "../model/PatientRecord.h"

namespace scapms {
namespace gui {

struct AppointmentDialogResult {
    bool confirmed{false};
    std::string patientId;
    std::string patientName;
    std::string date;
    std::string time;
    std::string department;
    std::string doctor;
    std::string notes;
};

class AppointmentDialog {
public:
    static AppointmentDialogResult showScheduleDialog(
        HWND parentHwnd,
        const std::vector<std::shared_ptr<model::PatientRecord>>& patients,
        const std::string& preselectedPatientId = ""
    );
};

} // namespace gui
} // namespace scapms

#endif // SCAPMS_GUI_APPOINTMENTDIALOG_H

