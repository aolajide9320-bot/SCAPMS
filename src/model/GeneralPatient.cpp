#include "GeneralPatient.h"
#include <sstream>
#include <algorithm>

namespace scapms {
namespace model {

GeneralPatient::GeneralPatient(std::string id, std::string name, int age,
                               std::string contact, std::string generalSymptoms,
                               bool hasRoutineCheckup, std::string registrationDate)
    : PatientRecord(std::move(id), std::move(name), age, std::move(contact), std::move(registrationDate)),
      m_generalSymptoms(std::move(generalSymptoms)),
      m_hasRoutineCheckup(hasRoutineCheckup) {}

std::string GeneralPatient::getDetails() const {
    std::ostringstream oss;
    oss << "Symptoms: " << m_generalSymptoms
        << " | Routine Checkup: " << (m_hasRoutineCheckup ? "Yes" : "No")
        << " | Scheduled Appointments: " << m_appointmentIds.size();
    return oss.str();
}

double GeneralPatient::calculateConsultationFee() const {
    // Standard baseline fee: $40.00, + $15.00 if detailed symptom evaluation needed
    double base = 40.0;
    if (!m_hasRoutineCheckup || m_generalSymptoms != "None") {
        base += 15.0;
    }
    return base;
}

std::string GeneralPatient::serialize() const {
    std::ostringstream oss;
    oss << "General," << m_id << "," << m_name << "," << m_age << "," << m_contact << ","
        << m_registrationDate << "," << m_generalSymptoms << ","
        << (m_hasRoutineCheckup ? "1" : "0");
    return oss.str();
}

std::unique_ptr<PatientRecord> GeneralPatient::clone() const {
    auto p = std::make_unique<GeneralPatient>(m_id, m_name, m_age, m_contact,
                                              m_generalSymptoms, m_hasRoutineCheckup,
                                              m_registrationDate);
    p->m_appointmentIds = this->m_appointmentIds;
    return p;
}

void GeneralPatient::scheduleAppointment(const std::string& appointmentId,
                                         const std::string& /*date*/,
                                         const std::string& /*time*/,
                                         const std::string& /*reason*/) {
    m_appointmentIds.push_back(appointmentId);
}

bool GeneralPatient::cancelAppointment(const std::string& appointmentId) {
    auto it = std::find(m_appointmentIds.begin(), m_appointmentIds.end(), appointmentId);
    if (it != m_appointmentIds.end()) {
        m_appointmentIds.erase(it);
        return true;
    }
    return false;
}

bool GeneralPatient::hasActiveAppointment() const {
    return !m_appointmentIds.empty();
}

} // namespace model
} // namespace scapms
