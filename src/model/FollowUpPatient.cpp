#include "FollowUpPatient.h"
#include <sstream>
#include <algorithm>

namespace scapms {
namespace model {

FollowUpPatient::FollowUpPatient(std::string id, std::string name, int age,
                                 std::string contact, std::string previousVisitDate,
                                 std::string followUpReason, std::string attendingDoctor,
                                 std::string registrationDate)
    : PatientRecord(std::move(id), std::move(name), age, std::move(contact), std::move(registrationDate)),
      m_previousVisitDate(std::move(previousVisitDate)),
      m_followUpReason(std::move(followUpReason)),
      m_attendingDoctor(std::move(attendingDoctor)) {}

std::string FollowUpPatient::getDetails() const {
    std::ostringstream oss;
    oss << "Prior Visit: " << m_previousVisitDate
        << " | Reason: " << m_followUpReason
        << " | Attending: " << m_attendingDoctor
        << " | Scheduled Appointments: " << m_appointmentIds.size();
    return oss.str();
}

double FollowUpPatient::calculateConsultationFee() const {
    // Discounted fee for continuity of care
    return 25.00;
}

std::string FollowUpPatient::serialize() const {
    std::ostringstream oss;
    oss << "Follow-Up," << m_id << "," << m_name << "," << m_age << "," << m_contact << ","
        << m_registrationDate << "," << m_previousVisitDate << ","
        << m_followUpReason << "," << m_attendingDoctor;
    return oss.str();
}

std::unique_ptr<PatientRecord> FollowUpPatient::clone() const {
    auto p = std::make_unique<FollowUpPatient>(m_id, m_name, m_age, m_contact,
                                               m_previousVisitDate, m_followUpReason,
                                               m_attendingDoctor, m_registrationDate);
    p->m_appointmentIds = this->m_appointmentIds;
    return p;
}

void FollowUpPatient::scheduleAppointment(const std::string& appointmentId,
                                          const std::string& /*date*/,
                                          const std::string& /*time*/,
                                          const std::string& /*reason*/) {
    m_appointmentIds.push_back(appointmentId);
}

bool FollowUpPatient::cancelAppointment(const std::string& appointmentId) {
    auto it = std::find(m_appointmentIds.begin(), m_appointmentIds.end(), appointmentId);
    if (it != m_appointmentIds.end()) {
        m_appointmentIds.erase(it);
        return true;
    }
    return false;
}

bool FollowUpPatient::hasActiveAppointment() const {
    return !m_appointmentIds.empty();
}

} // namespace model
} // namespace scapms
