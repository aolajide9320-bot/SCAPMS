#ifndef SCAPMS_MODEL_FOLLOWUPPATIENT_H
#define SCAPMS_MODEL_FOLLOWUPPATIENT_H

#include "PatientRecord.h"
#include "Appointable.h"
#include <vector>
#include <string>

namespace scapms {
namespace model {

/**
 * @brief Represents a returning patient scheduled for review or continuation of care.
 *        Implements Appointable interface.
 */
class FollowUpPatient : public PatientRecord, public Appointable {
private:
    std::string m_previousVisitDate;
    std::string m_followUpReason;
    std::string m_attendingDoctor;
    std::vector<std::string> m_appointmentIds;

public:
    FollowUpPatient(std::string id, std::string name, int age,
                    std::string contact, std::string previousVisitDate = "2026-10-15",
                    std::string followUpReason = "General Progress Review",
                    std::string attendingDoctor = "Dr. Adebayo",
                    std::string registrationDate = "");

    // Getters and Setters
    const std::string& getPreviousVisitDate() const { return m_previousVisitDate; }
    void setPreviousVisitDate(const std::string& date) { m_previousVisitDate = date; }

    const std::string& getFollowUpReason() const { return m_followUpReason; }
    void setFollowUpReason(const std::string& reason) { m_followUpReason = reason; }

    const std::string& getAttendingDoctor() const { return m_attendingDoctor; }
    void setAttendingDoctor(const std::string& doc) { m_attendingDoctor = doc; }

    // PatientRecord pure virtual implementations
    std::string getPatientType() const override { return "Follow-Up"; }
    std::string getDetails() const override;
    double calculateConsultationFee() const override;
    std::string serialize() const override;
    std::unique_ptr<PatientRecord> clone() const override;

    // Appointable interface implementations
    void scheduleAppointment(const std::string& appointmentId,
                            const std::string& date,
                            const std::string& time,
                            const std::string& reason) override;

    bool cancelAppointment(const std::string& appointmentId) override;
    bool hasActiveAppointment() const override;
    std::vector<std::string> getAppointmentIDs() const override { return m_appointmentIds; }
    void addAppointmentID(const std::string& aptId) { m_appointmentIds.push_back(aptId); }
};

} // namespace model
} // namespace scapms

#endif // SCAPMS_MODEL_FOLLOWUPPATIENT_H
