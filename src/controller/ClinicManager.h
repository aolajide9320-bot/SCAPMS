#ifndef SCAPMS_CONTROLLER_CLINICMANAGER_H
#define SCAPMS_CONTROLLER_CLINICMANAGER_H

#include "../model/ClinicDatabase.h"
#include "AppointmentController.h"
#include "SearchEngine.h"
#include "SortEngine.h"
#include <string>
#include <vector>
#include <memory>

namespace scapms {
namespace controller {

/**
 * @brief High-level system facade and business logic coordinator (Mandatory Controller component).
 */
class ClinicManager {
private:
    model::ClinicDatabase m_database;
    AppointmentController m_appointmentController;

public:
    ClinicManager();
    ~ClinicManager() = default;

    // Database accessors
    model::ClinicDatabase& getDatabase() { return m_database; }
    const model::ClinicDatabase& getDatabase() const { return m_database; }
    AppointmentController& getAppointmentController() { return m_appointmentController; }

    // --- Patient Registration & Management ---
    std::string registerGeneralPatient(const std::string& name, int age,
                                       const std::string& contact,
                                       const std::string& symptoms,
                                       bool routineCheckup);

    std::string registerEmergencyPatient(const std::string& name, int age,
                                         const std::string& contact,
                                         int severityLevel,
                                         const std::string& emergencyContact);

    std::string registerFollowUpPatient(const std::string& name, int age,
                                        const std::string& contact,
                                        const std::string& previousVisitDate,
                                        const std::string& followUpReason,
                                        const std::string& attendingDoctor);

    bool updatePatient(const std::string& id, const std::string& name, int age, const std::string& contact);
    bool deletePatient(const std::string& id);
    std::shared_ptr<model::PatientRecord> getPatient(const std::string& id);

    // Input Validation (Mandatory Section 9.e)
    static void validatePatientInput(const std::string& name, int age, const std::string& contact);

    // --- Waiting Queue (std::queue) ---
    void addPatientToQueue(const std::string& patientId);
    std::shared_ptr<model::PatientRecord> callNextPatient();
    std::vector<std::shared_ptr<model::PatientRecord>> getWaitingPatientsList();
    size_t getQueueLength() const { return m_database.getQueueSize(); }

    // --- Undo Stack (std::stack) ---
    bool undoLastAction();
    std::string getUndoActionDescription() const;
    bool canUndo() const { return m_database.canUndo(); }

    // --- Cache Inspection (Fixed Array [10]) ---
    std::vector<const model::PatientRecord*> getRecentlyAccessedPatients() const;

    // --- Search & Sort Integration ---
    SearchResult search(const std::string& query, SearchAlgorithmType algo, const std::string& field);
    SortResult sort(SortAlgorithmType algo, SortField field, bool ascending = true);

    // --- Persistence ---
    bool saveToDisk(const std::string& pPath = "patients.csv", const std::string& aPath = "appointments.csv");
    bool loadFromDisk(const std::string& pPath = "patients.csv", const std::string& aPath = "appointments.csv");
    void resetWithSampleData();
};

} // namespace controller
} // namespace scapms

#endif // SCAPMS_CONTROLLER_CLINICMANAGER_H
