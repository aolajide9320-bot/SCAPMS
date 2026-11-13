#ifndef SCAPMS_MODEL_EMERGENCYPATIENT_H
#define SCAPMS_MODEL_EMERGENCYPATIENT_H

#include "PatientRecord.h"
#include <string>

namespace scapms {
namespace model {

/**
 * @brief Represents an acute emergency patient requiring immediate triage and priority care.
 *        Does NOT implement Appointable because emergency admissions bypass scheduled appointments.
 */
class EmergencyPatient : public PatientRecord {
private:
    int m_severityLevel;              // 1 (Critical) to 5 (Non-urgent)
    std::string m_emergencyContact;   // Next of kin contact name & phone
    std::string m_triageCategory;     // e.g. "Level 1: Resuscitation", "Level 2: Emergent"

public:
    EmergencyPatient(std::string id, std::string name, int age,
                     std::string contact, int severityLevel = 2,
                     std::string emergencyContact = "Emergency Services",
                     std::string registrationDate = "");

    // Getters and Setters
    int getSeverityLevel() const { return m_severityLevel; }
    void setSeverityLevel(int level);

    const std::string& getEmergencyContact() const { return m_emergencyContact; }
    void setEmergencyContact(const std::string& contact) { m_emergencyContact = contact; }

    const std::string& getTriageCategory() const { return m_triageCategory; }

    // PatientRecord pure virtual implementations
    std::string getPatientType() const override { return "Emergency"; }
    std::string getDetails() const override;
    double calculateConsultationFee() const override;
    std::string serialize() const override;
    std::unique_ptr<PatientRecord> clone() const override;

private:
    void updateTriageCategory();
};

} // namespace model
} // namespace scapms

#endif // SCAPMS_MODEL_EMERGENCYPATIENT_H
