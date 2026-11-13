#ifndef SCAPMS_MODEL_APPOINTMENT_H
#define SCAPMS_MODEL_APPOINTMENT_H

#include <string>
#include <iostream>

namespace scapms {
namespace model {

enum class AppointmentStatus {
    SCHEDULED,
    COMPLETED,
    CANCELLED
};

inline std::string statusToString(AppointmentStatus status) {
    switch (status) {
        case AppointmentStatus::SCHEDULED: return "Scheduled";
        case AppointmentStatus::COMPLETED: return "Completed";
        case AppointmentStatus::CANCELLED: return "Cancelled";
        default: return "Unknown";
    }
}

inline AppointmentStatus stringToStatus(const std::string& str) {
    if (str == "Completed" || str == "COMPLETED") return AppointmentStatus::COMPLETED;
    if (str == "Cancelled" || str == "CANCELLED") return AppointmentStatus::CANCELLED;
    return AppointmentStatus::SCHEDULED;
}

/**
 * @brief Represents a scheduled or recorded clinical consultation session.
 */
class Appointment {
private:
    std::string m_appointmentId;
    std::string m_patientId;
    std::string m_patientName;
    std::string m_date;
    std::string m_time;
    std::string m_department;
    std::string m_doctorName;
    AppointmentStatus m_status;
    std::string m_notes;
    double m_fee;

public:
    Appointment(std::string appointmentId,
                std::string patientId,
                std::string patientName,
                std::string date,
                std::string time,
                std::string department,
                std::string doctorName,
                AppointmentStatus status = AppointmentStatus::SCHEDULED,
                std::string notes = "",
                double fee = 40.00);

    // Getters
    const std::string& getAppointmentId() const { return m_appointmentId; }
    const std::string& getPatientId() const { return m_patientId; }
    const std::string& getPatientName() const { return m_patientName; }
    const std::string& getDate() const { return m_date; }
    const std::string& getTime() const { return m_time; }
    const std::string& getDepartment() const { return m_department; }
    const std::string& getDoctorName() const { return m_doctorName; }
    AppointmentStatus getStatus() const { return m_status; }
    std::string getStatusString() const { return statusToString(m_status); }
    const std::string& getNotes() const { return m_notes; }
    double getFee() const { return m_fee; }

    // Setters
    void setPatientName(const std::string& name) { m_patientName = name; }
    void setDate(const std::string& date) { m_date = date; }
    void setTime(const std::string& time) { m_time = time; }
    void setDepartment(const std::string& dept) { m_department = dept; }
    void setDoctorName(const std::string& doc) { m_doctorName = doc; }
    void setStatus(AppointmentStatus status) { m_status = status; }
    void setNotes(const std::string& notes) { m_notes = notes; }
    void setFee(double fee) { m_fee = fee; }

    std::string serialize() const;
    void print(std::ostream& os = std::cout) const;
};

} // namespace model
} // namespace scapms

#endif // SCAPMS_MODEL_APPOINTMENT_H
