#include "EmergencyPatient.h"
#include <sstream>

namespace scapms {
namespace model {

EmergencyPatient::EmergencyPatient(std::string id, std::string name, int age,
                                   std::string contact, int severityLevel,
                                   std::string emergencyContact,
                                   std::string registrationDate)
    : PatientRecord(std::move(id), std::move(name), age, std::move(contact), std::move(registrationDate)),
      m_severityLevel(severityLevel),
      m_emergencyContact(std::move(emergencyContact)) {
    updateTriageCategory();
}

void EmergencyPatient::setSeverityLevel(int level) {
    if (level < 1) level = 1;
    if (level > 5) level = 5;
    m_severityLevel = level;
    updateTriageCategory();
}

void EmergencyPatient::updateTriageCategory() {
    switch (m_severityLevel) {
        case 1: m_triageCategory = "Critical / Resuscitation"; break;
        case 2: m_triageCategory = "Emergent / High Priority"; break;
        case 3: m_triageCategory = "Urgent"; break;
        case 4: m_triageCategory = "Semi-Urgent"; break;
        default: m_triageCategory = "Non-Urgent"; break;
    }
}

std::string EmergencyPatient::getDetails() const {
    std::ostringstream oss;
    oss << "Triage: " << m_triageCategory
        << " (Severity " << m_severityLevel << ")"
        << " | Emergency Contact: " << m_emergencyContact;
    return oss.str();
}

double EmergencyPatient::calculateConsultationFee() const {
    // Base emergency intake is $100.00 + additional charge based on severity level
    double fee = 120.00;
    if (m_severityLevel == 1) fee += 80.00; // Critical care surcharge
    else if (m_severityLevel == 2) fee += 40.00;
    return fee;
}

std::string EmergencyPatient::serialize() const {
    std::ostringstream oss;
    oss << "Emergency," << m_id << "," << m_name << "," << m_age << "," << m_contact << ","
        << m_registrationDate << "," << m_severityLevel << "," << m_emergencyContact;
    return oss.str();
}

std::unique_ptr<PatientRecord> EmergencyPatient::clone() const {
    return std::make_unique<EmergencyPatient>(m_id, m_name, m_age, m_contact,
                                              m_severityLevel, m_emergencyContact,
                                              m_registrationDate);
}

} // namespace model
} // namespace scapms
