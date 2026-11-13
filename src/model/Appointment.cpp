#include "Appointment.h"
#include <sstream>
#include <iomanip>

namespace scapms {
namespace model {

Appointment::Appointment(std::string appointmentId,
                         std::string patientId,
                         std::string patientName,
                         std::string date,
                         std::string time,
                         std::string department,
                         std::string doctorName,
                         AppointmentStatus status,
                         std::string notes,
                         double fee)
    : m_appointmentId(std::move(appointmentId)),
      m_patientId(std::move(patientId)),
      m_patientName(std::move(patientName)),
      m_date(std::move(date)),
      m_time(std::move(time)),
      m_department(std::move(department)),
      m_doctorName(std::move(doctorName)),
      m_status(status),
      m_notes(std::move(notes)),
      m_fee(fee) {}

std::string Appointment::serialize() const {
    std::ostringstream oss;
    oss << m_appointmentId << ","
        << m_patientId << ","
        << m_patientName << ","
        << m_date << ","
        << m_time << ","
        << m_department << ","
        << m_doctorName << ","
        << getStatusString() << ","
        << m_notes << ","
        << std::fixed << std::setprecision(2) << m_fee;
    return oss.str();
}

void Appointment::print(std::ostream& os) const {
    os << "Apt ID: " << m_appointmentId
       << " | Patient: " << m_patientName << " (" << m_patientId << ")"
       << " | Date/Time: " << m_date << " " << m_time
       << " | Dept: " << m_department
       << " | Doctor: " << m_doctorName
       << " | Status: " << getStatusString()
       << " | Fee: $" << std::fixed << std::setprecision(2) << m_fee;
}

} // namespace model
} // namespace scapms
