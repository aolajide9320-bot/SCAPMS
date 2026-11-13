#ifndef SCAPMS_MODEL_CLINICDATABASE_H
#define SCAPMS_MODEL_CLINICDATABASE_H

#include "PatientRecord.h"
#include "GeneralPatient.h"
#include "EmergencyPatient.h"
#include "FollowUpPatient.h"
#include "Appointment.h"
#include "../utils/Exceptions.h"

#include <vector>
#include <queue>
#include <stack>
#include <memory>
#include <string>
#include <array>
#include <functional>

namespace scapms {
namespace model {

// Forward declaration of database for undo commands
class ClinicDatabase;

/**
 * @brief Command pattern base class for undoable administrative operations stored in std::stack.
 */
class AdminOperation {
public:
    virtual ~AdminOperation() = default;
    virtual void undo(ClinicDatabase& db) = 0;
    virtual std::string getDescription() const = 0;
};

/**
 * @brief Central repository class demonstrating Encapsulation and Composition (Mandatory Req 1.e).
 *        Holds std::vector for patients, std::queue for waiting queue, std::stack for undo,
 *        and fixed-size array PatientRecord* recentPatients[10] for cache.
 */
class ClinicDatabase {
public:
    static constexpr size_t RECENT_CACHE_SIZE = 10;

private:
    // Core Data Structures (Mandatory Requirements 3.a, 3.b, 3.c, 3.d)
    std::vector<std::shared_ptr<PatientRecord>> m_patients;       // 3.a: std::vector
    std::queue<std::string> m_waitingQueue;                       // 3.b: std::queue
    std::stack<std::shared_ptr<AdminOperation>> m_undoStack;      // 3.c: std::stack
    PatientRecord* m_recentPatients[RECENT_CACHE_SIZE];           // 3.d: C++ fixed-size array cache
    size_t m_recentCount;

    // Composition with appointments
    std::vector<Appointment> m_appointments;

public:
    ClinicDatabase();
    ~ClinicDatabase() = default;

    // Prevent direct copying to protect raw pointer cache integrity
    ClinicDatabase(const ClinicDatabase&) = delete;
    ClinicDatabase& operator=(const ClinicDatabase&) = delete;

    // --- Patient Operations ---
    void addPatient(std::shared_ptr<PatientRecord> patient, bool trackUndo = true);
    bool updatePatient(const std::string& id, const std::string& name, int age,
                       const std::string& contact, bool trackUndo = true);
    bool deletePatient(const std::string& id, bool trackUndo = true);
    std::shared_ptr<PatientRecord> findPatientById(const std::string& id);
    const std::vector<std::shared_ptr<PatientRecord>>& getAllPatients() const { return m_patients; }
    size_t getPatientCount() const { return m_patients.size(); }

    // --- Appointment Operations ---
    void addAppointment(const Appointment& apt, bool trackUndo = true);
    bool cancelAppointment(const std::string& appointmentId, bool trackUndo = true);
    bool completeAppointment(const std::string& appointmentId);
    bool deleteAppointment(const std::string& appointmentId, bool trackUndo = true);
    const Appointment* findAppointmentById(const std::string& appointmentId) const;
    Appointment* findAppointmentById(const std::string& appointmentId);
    std::vector<Appointment> getAppointmentsForPatient(const std::string& patientId) const;
    const std::vector<Appointment>& getAllAppointments() const { return m_appointments; }
    size_t getAppointmentCount() const { return m_appointments.size(); }

    // --- Waiting Queue Operations (std::queue) ---
    void enqueuePatient(const std::string& patientId);
    std::string dequeuePatient();
    std::string peekNextPatient() const;
    bool isQueueEmpty() const { return m_waitingQueue.empty(); }
    size_t getQueueSize() const { return m_waitingQueue.size(); }
    std::vector<std::string> getWaitingQueueSnapshot() const;

    // --- Undo Stack Operations (std::stack) ---
    void pushUndoOperation(std::shared_ptr<AdminOperation> op);
    bool canUndo() const { return !m_undoStack.empty(); }
    std::string getNextUndoDescription() const;
    bool undoLastOperation();
    size_t getUndoStackSize() const { return m_undoStack.size(); }

    // --- Cache Operations (Fixed Array) ---
    void recordPatientAccess(PatientRecord* patient);
    void invalidateCachedPointer(PatientRecord* patient);
    size_t getRecentCacheCount() const { return m_recentCount; }
    const PatientRecord* getCachedPatient(size_t index) const {
        if (index < m_recentCount) return m_recentPatients[index];
        return nullptr;
    }

    // Direct access for testing/demonstration
    PatientRecord** getRawRecentCache() { return m_recentPatients; }

    // Utility
    void clearAll();
};

// --- Undo Operations Concrete Implementations ---

class UndoDeletePatientOperation : public AdminOperation {
private:
    std::shared_ptr<PatientRecord> m_deletedPatient;
    std::vector<Appointment> m_associatedAppointments;
public:
    UndoDeletePatientOperation(std::shared_ptr<PatientRecord> patient,
                                std::vector<Appointment> appointments)
        : m_deletedPatient(std::move(patient)),
          m_associatedAppointments(std::move(appointments)) {}

    void undo(ClinicDatabase& db) override;
    std::string getDescription() const override {
        return "Undo Delete Patient: " + (m_deletedPatient ? m_deletedPatient->getName() : "Unknown");
    }
};

class UndoAddPatientOperation : public AdminOperation {
private:
    std::string m_patientId;
public:
    explicit UndoAddPatientOperation(std::string patientId)
        : m_patientId(std::move(patientId)) {}

    void undo(ClinicDatabase& db) override;
    std::string getDescription() const override {
        return "Undo Add Patient ID: " + m_patientId;
    }
};

class UndoModifyPatientOperation : public AdminOperation {
private:
    std::shared_ptr<PatientRecord> m_previousState;
public:
    explicit UndoModifyPatientOperation(std::shared_ptr<PatientRecord> previousState)
        : m_previousState(std::move(previousState)) {}

    void undo(ClinicDatabase& db) override;
    std::string getDescription() const override {
        return "Undo Modify Patient: " + (m_previousState ? m_previousState->getName() : "");
    }
};

class UndoRemoveAppointmentOperation : public AdminOperation {
private:
    Appointment m_removedAppointment;
public:
    explicit UndoRemoveAppointmentOperation(Appointment apt)
        : m_removedAppointment(std::move(apt)) {}

    void undo(ClinicDatabase& db) override;
    std::string getDescription() const override {
        return "Undo Cancel/Remove Appointment: " + m_removedAppointment.getAppointmentId();
    }
};

class UndoScheduleAppointmentOperation : public AdminOperation {
private:
    std::string m_appointmentId;
public:
    explicit UndoScheduleAppointmentOperation(std::string appointmentId)
        : m_appointmentId(std::move(appointmentId)) {}

    void undo(ClinicDatabase& db) override;
    std::string getDescription() const override {
        return "Undo Schedule Appointment: " + m_appointmentId;
    }
};

} // namespace model
} // namespace scapms

#endif // SCAPMS_MODEL_CLINICDATABASE_H
