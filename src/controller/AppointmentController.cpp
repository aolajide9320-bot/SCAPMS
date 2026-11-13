#include "AppointmentController.h"
#include "../utils/IDGenerator.h"
#include "../utils/Exceptions.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace scapms {
namespace controller {

AppointmentController::AppointmentController(model::ClinicDatabase& db)
    : m_db(db) {}

std::string AppointmentController::scheduleNewAppointment(
    const std::string& patientId,
    const std::string& date,
    const std::string& time,
    const std::string& department,
    const std::string& doctor,
    const std::string& notes) {

    auto patient = m_db.findPatientById(patientId);
    if (!patient) {
        throw utils::PatientNotFoundException(patientId);
    }

    if (date.empty()) {
        throw utils::InvalidInputException("Appointment Date", "Date cannot be empty.");
    }
    if (time.empty()) {
        throw utils::InvalidInputException("Appointment Time", "Time cannot be empty.");
    }

    // Verify Appointable capability
    auto appointable = dynamic_cast<model::Appointable*>(patient.get());
    if (!appointable) {
        // Emergency patients bypass standard scheduled appointments
        throw utils::AppointmentException("Patient '" + patient->getName() +
                                           "' is an Emergency patient and cannot be booked for routine advance appointments.");
    }

    std::string aptId = utils::IDGenerator::generateAppointmentID();
    double fee = patient->calculateConsultationFee();

    model::Appointment apt(aptId, patientId, patient->getName(), date, time,
                           department, doctor, model::AppointmentStatus::SCHEDULED,
                           notes, fee);

    m_db.addAppointment(apt, true);
    return aptId;
}

bool AppointmentController::cancelExistingAppointment(const std::string& appointmentId) {
    return m_db.cancelAppointment(appointmentId, true);
}

bool AppointmentController::completeAppointment(const std::string& appointmentId) {
    return m_db.completeAppointment(appointmentId);
}

// Report 1: Most Frequently Attended Patients
std::vector<FrequentlyAttendedPatient> AppointmentController::getMostFrequentlyAttendedPatients(int limit) const {
    std::map<std::string, int> completionMap;
    for (const auto& apt : m_db.getAllAppointments()) {
        if (apt.getStatus() == model::AppointmentStatus::COMPLETED) {
            completionMap[apt.getPatientId()]++;
        }
    }

    std::vector<FrequentlyAttendedPatient> list;
    for (const auto& pair : completionMap) {
        auto pat = const_cast<model::ClinicDatabase&>(m_db).findPatientById(pair.first);
        if (pat) {
            list.push_back({pat->getId(), pat->getName(), pat->getPatientType(), pair.second});
        } else {
            list.push_back({pair.first, "Unknown", "General", pair.second});
        }
    }

    // Sort descending by completion count
    std::sort(list.begin(), list.end(), [](const FrequentlyAttendedPatient& a, const FrequentlyAttendedPatient& b) {
        return a.completedCount > b.completedCount;
    });

    if (static_cast<int>(list.size()) > limit) {
        list.resize(limit);
    }
    return list;
}

// Report 2: Patients with Pending Appointments
std::vector<model::Appointment> AppointmentController::getPendingAppointments() const {
    std::vector<model::Appointment> pending;
    for (const auto& apt : m_db.getAllAppointments()) {
        if (apt.getStatus() == model::AppointmentStatus::SCHEDULED) {
            pending.push_back(apt);
        }
    }
    return pending;
}

// Report 3: Patient Category Distribution
CategoryDistributionReport AppointmentController::getCategoryDistribution() const {
    CategoryDistributionReport r;
    r.total = static_cast<int>(m_db.getPatientCount());
    for (const auto& p : m_db.getAllPatients()) {
        if (p->getPatientType() == "General") {
            r.generalCount++;
        } else if (p->getPatientType() == "Emergency") {
            r.emergencyCount++;
        } else if (p->getPatientType() == "Follow-Up") {
            r.followUpCount++;
        }
    }
    return r;
}

// Report 4: Daily Appointment Summary
DailySummaryReport AppointmentController::getDailySummary(const std::string& date) const {
    DailySummaryReport r;
    for (const auto& apt : m_db.getAllAppointments()) {
        if (date.empty() || apt.getDate() == date) {
            r.totalAppointments++;
            if (apt.getStatus() == model::AppointmentStatus::COMPLETED) {
                r.completed++;
                r.totalRevenue += apt.getFee();
            } else if (apt.getStatus() == model::AppointmentStatus::SCHEDULED) {
                r.pending++;
            } else if (apt.getStatus() == model::AppointmentStatus::CANCELLED) {
                r.cancelled++;
            }
        }
    }
    return r;
}

std::string AppointmentController::generateTextReportSummary() const {
    std::ostringstream oss;
    auto dist = getCategoryDistribution();
    auto daily = getDailySummary();
    auto top = getMostFrequentlyAttendedPatients(5);
    auto pending = getPendingAppointments();

    oss << "========================================================\n"
        << "      SMART CLINIC APPOINTMENT & PATIENT MANAGEMENT\n"
        << "                 EXECUTIVE CLINIC REPORT\n"
        << "========================================================\n\n"
        << "1. PATIENT CATEGORY DISTRIBUTION\n"
        << "   - General Outpatients: " << dist.generalCount << "\n"
        << "   - Emergency Patients:  " << dist.emergencyCount << "\n"
        << "   - Follow-Up Patients:  " << dist.followUpCount << "\n"
        << "   - Total Registered:    " << dist.total << "\n\n"
        << "2. DAILY APPOINTMENT SUMMARY\n"
        << "   - Total Appointments:  " << daily.totalAppointments << "\n"
        << "   - Completed:           " << daily.completed << "\n"
        << "   - Pending (Scheduled): " << daily.pending << "\n"
        << "   - Cancelled:           " << daily.cancelled << "\n"
        << "   - Realized Revenue:    $" << std::fixed << std::setprecision(2) << daily.totalRevenue << "\n\n"
        << "3. TOP FREQUENTLY ATTENDED PATIENTS\n";

    if (top.empty()) {
        oss << "   (No completed appointments recorded yet)\n";
    } else {
        for (size_t i = 0; i < top.size(); ++i) {
            oss << "   " << (i + 1) << ". " << top[i].patientName
                << " [" << top[i].patientId << "] (" << top[i].patientType << ") - "
                << top[i].completedCount << " completed visits\n";
        }
    }

    oss << "\n4. PENDING APPOINTMENTS LIST (" << pending.size() << " active)\n";
    for (const auto& a : pending) {
        oss << "   - [" << a.getAppointmentId() << "] " << a.getDate() << " " << a.getTime()
            << " | " << a.getPatientName() << " | Dr. " << a.getDoctorName()
            << " (" << a.getDepartment() << ")\n";
    }
    oss << "\n========================================================\n";
    return oss.str();
}

} // namespace controller
} // namespace scapms
