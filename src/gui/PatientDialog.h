#ifndef SCAPMS_GUI_PATIENTDIALOG_H
#define SCAPMS_GUI_PATIENTDIALOG_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <string>
#include <memory>
#include "../model/PatientRecord.h"

namespace scapms {
namespace gui {

struct PatientDialogResult {
    bool confirmed{false};
    std::string category;
    std::string name;
    int age{0};
    std::string contact;
    // General
    std::string symptoms;
    bool routineCheckup{true};
    // Emergency
    int severityLevel{2};
    std::string emergencyContact;
    // Follow-up
    std::string previousVisitDate;
    std::string followUpReason;
    std::string attendingDoctor;
};

class PatientDialog {
public:
    static PatientDialogResult showAddDialog(HWND parentHwnd);
    static PatientDialogResult showEditDialog(HWND parentHwnd, const std::shared_ptr<model::PatientRecord>& patient);
};

} // namespace gui
} // namespace scapms

#endif // SCAPMS_GUI_PATIENTDIALOG_H
