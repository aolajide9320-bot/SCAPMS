#ifndef SCAPMS_CONTROLLER_APPOINTMENTCONTROLLER_H
#define SCAPMS_CONTROLLER_APPOINTMENTCONTROLLER_H

#include "../model/ClinicDatabase.h"
#include "../model/Appointment.h"
#include <string>
#include <vector>
#include <map>

namespace scapms {
namespace controller {

struct FrequentlyAttendedPatient {
    std::string patientId;
    std::string patientName;
    std::string patientType;
    int completedCount;
};

struct DailySummaryReport {
    int totalAppointments{0};
    int completed{0};
    int pending{0};
    int cancelled{0};
    double totalRevenue{0.0};
};

struct CategoryDistributionReport {
    int generalCount{0};
    int emergencyCount{0};
    int followUpCount{0};
    int total{0};
};

/**
 * @brief Manages appointment lifecycle, validation, and analytics reporting (Mandatory Features).
 */
class AppointmentController {
private:
    model::ClinicDatabase& m_db;

public:
    explicit AppointmentController(model::ClinicDatabase& db);

    // Core Appointment Actions
    std::string scheduleNewAppointment(const std::string& patientId,
                                      const std::string& date,
                                      const std::string& time,
                                      const std::string& department,
                                      const std::string& doctor,
                                      const std::string& notes = "");

    bool cancelExistingAppointment(const std::string& appointmentId);
    bool completeAppointment(const std::string& appointmentId);

    // Reporting Features (Mandatory Section "Simple Reports")
    // Report 1: Most Frequently Attended Patients
    std::vector<FrequentlyAttendedPatient> getMostFrequentlyAttendedPatients(int limit = 5) const;

    // Report 2: Patients with Pending Appointments
    std::vector<model::Appointment> getPendingAppointments() const;

    // Report 3: Patient Category Distribution
    CategoryDistributionReport getCategoryDistribution() const;

    // Report 4: Daily Appointment Summary
    DailySummaryReport getDailySummary(const std::string& date = "") const;

    // Formatting utilities
    std::string generateTextReportSummary() const;
};

} // namespace controller
} // namespace scapms

#endif // SCAPMS_CONTROLLER_APPOINTMENTCONTROLLER_H
