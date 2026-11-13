#include "PatientRecord.h"
#include <iomanip>

namespace scapms {
namespace model {

PatientRecord::PatientRecord(std::string id, std::string name, int age,
                             std::string contact, std::string registrationDate)
    : m_id(std::move(id)),
      m_name(std::move(name)),
      m_age(age),
      m_contact(std::move(contact)),
      m_registrationDate(std::move(registrationDate)) {
    if (m_registrationDate.empty()) {
        m_registrationDate = "2026-11-06"; // Default clinic operation date
    }
}

void PatientRecord::printSummary(std::ostream& os) const {
    os << "[" << getPatientType() << "] ID: " << m_id
       << " | Name: " << m_name
       << " | Age: " << m_age
       << " | Contact: " << m_contact
       << " | Fee: $" << std::fixed << std::setprecision(2) << calculateConsultationFee()
       << " | Details: " << getDetails();
}

void processPatientRecord(const PatientRecord& record, std::ostream& os) {
    os << "=== Processing Patient Record via Base Reference ===" << std::endl;
    os << "Type: " << record.getPatientType() << std::endl;
    os << "Identification: " << record.getId() << " - " << record.getName() << std::endl;
    os << "Consultation Fee: $" << std::fixed << std::setprecision(2)
       << record.calculateConsultationFee() << std::endl;
    os << "Specific Details: " << record.getDetails() << std::endl;
    os << "===================================================" << std::endl;
}

} // namespace model
} // namespace scapms
