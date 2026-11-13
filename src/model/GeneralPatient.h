#ifndef SCAPMS_MODEL_GENERALPATIENT_H
#define SCAPMS_MODEL_GENERALPATIENT_H

#include "PatientRecord.h"
#include "Appointable.h"
#include <vector>
#include <string>

namespace scapms {
namespace model {

/**
 * @brief Represents a regular outpatient visiting for general consultation or wellness checkups.
 *        Implements Appointable interface for scheduled doctor appointments.
 */
class GeneralPatient : public PatientRecord, public Appointable {
private:
    std::string m_generalSymptoms;
    bool m_hasRoutineCheckup;
    std::vector<std::string> m_appointmentIds;

public:
    GeneralPatient(std::string id, std::string name, int age,
                   std::string contact, std::string generalSymptoms = "None",
                   bool hasRoutineCheckup = true,
                   std::string registrationDate = "");

    // Getters and Setters
    const std::string& getSymptoms() const { return m_generalSymptoms; }
    void setSymptoms(const std::string& symptoms) { m_generalSymptoms = symptoms; }

    bool isRoutineCheckup() const { return m_hasRoutineCheckup; }
    void setRoutineCheckup(bool val) { m_hasRoutineCheckup = val; }

    // PatientRecord pure virtual implementations
    std::string getPatientType() const override { return "General"; }
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

#endif // SCAPMS_MODEL_GENERALPATIENT_H
