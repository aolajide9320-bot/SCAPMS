#include "ClinicDatabase.h"
#include <algorithm>

namespace scapms {
namespace model {

ClinicDatabase::ClinicDatabase() : m_recentCount(0) {
    for (size_t i = 0; i < RECENT_CACHE_SIZE; ++i) {
        m_recentPatients[i] = nullptr;
    }
}

// --- Patient Operations ---

void ClinicDatabase::addPatient(std::shared_ptr<PatientRecord> patient, bool trackUndo) {
    if (!patient) return;

    // Check for duplicate ID
    for (const auto& p : m_patients) {
        if (p->getId() == patient->getId()) {
            throw utils::DuplicateIDException(patient->getId());
        }
    }

    m_patients.push_back(patient);
    recordPatientAccess(patient.get());

    if (trackUndo) {
        m_undoStack.push(std::make_shared<UndoAddPatientOperation>(patient->getId()));
    }
}

bool ClinicDatabase::updatePatient(const std::string& id, const std::string& name,
                                  int age, const std::string& contact, bool trackUndo) {
    for (auto& p : m_patients) {
        if (p->getId() == id) {
            if (trackUndo) {
                m_undoStack.push(std::make_shared<UndoModifyPatientOperation>(p->clone()));
            }

            p->setName(name);
            p->setAge(age);
            p->setContact(contact);

            // Also update appointment patient names if any
            for (auto& apt : m_appointments) {
                if (apt.getPatientId() == id) {
                    apt.setPatientName(name);
                }
            }

            recordPatientAccess(p.get());
            return true;
        }
    }
    return false;
}

bool ClinicDatabase::deletePatient(const std::string& id, bool trackUndo) {
    auto it = std::find_if(m_patients.begin(), m_patients.end(),
                           [&id](const std::shared_ptr<PatientRecord>& p) {
                               return p->getId() == id;
                           });

    if (it != m_patients.end()) {
        std::shared_ptr<PatientRecord> patientToDelete = *it;
        invalidateCachedPointer(patientToDelete.get());

        // Extract associated appointments for undo
        std::vector<Appointment> associatedApts;
        for (const auto& apt : m_appointments) {
            if (apt.getPatientId() == id) {
                associatedApts.push_back(apt);
            }
        }

        if (trackUndo) {
            m_undoStack.push(std::make_shared<UndoDeletePatientOperation>(patientToDelete, associatedApts));
        }

        m_patients.erase(it);

        // Remove associated appointments
        m_appointments.erase(
            std::remove_if(m_appointments.begin(), m_appointments.end(),
                           [&id](const Appointment& a) { return a.getPatientId() == id; }),
            m_appointments.end()
        );

        return true;
    }
    return false;
}

std::shared_ptr<PatientRecord> ClinicDatabase::findPatientById(const std::string& id) {
    for (auto& p : m_patients) {
        if (p->getId() == id) {
            recordPatientAccess(p.get());
            return p;
        }
    }
    return nullptr;
}

// --- Appointment Operations ---

void ClinicDatabase::addAppointment(const Appointment& apt, bool trackUndo) {
    m_appointments.push_back(apt);

    // Link with patient if Appointable
    auto patient = findPatientById(apt.getPatientId());
    if (patient) {
        if (auto appointable = dynamic_cast<Appointable*>(patient.get())) {
            appointable->scheduleAppointment(apt.getAppointmentId(), apt.getDate(), apt.getTime(), apt.getDepartment());
        }
    }

    if (trackUndo) {
        m_undoStack.push(std::make_shared<UndoScheduleAppointmentOperation>(apt.getAppointmentId()));
    }
}

bool ClinicDatabase::cancelAppointment(const std::string& appointmentId, bool trackUndo) {
    for (auto& a : m_appointments) {
        if (a.getAppointmentId() == appointmentId) {
            if (trackUndo) {
                m_undoStack.push(std::make_shared<UndoRemoveAppointmentOperation>(a));
            }
            a.setStatus(AppointmentStatus::CANCELLED);

            auto patient = findPatientById(a.getPatientId());
            if (patient) {
                if (auto appointable = dynamic_cast<Appointable*>(patient.get())) {
                    appointable->cancelAppointment(appointmentId);
                }
            }
            return true;
        }
    }
    return false;
}

bool ClinicDatabase::completeAppointment(const std::string& appointmentId) {
    for (auto& a : m_appointments) {
        if (a.getAppointmentId() == appointmentId) {
            a.setStatus(AppointmentStatus::COMPLETED);
            return true;
        }
    }
    return false;
}

bool ClinicDatabase::deleteAppointment(const std::string& appointmentId, bool trackUndo) {
    auto it = std::find_if(m_appointments.begin(), m_appointments.end(),
                           [&appointmentId](const Appointment& a) {
                               return a.getAppointmentId() == appointmentId;
                           });
    if (it != m_appointments.end()) {
        if (trackUndo) {
            m_undoStack.push(std::make_shared<UndoRemoveAppointmentOperation>(*it));
        }
        m_appointments.erase(it);
        return true;
    }
    return false;
}

const Appointment* ClinicDatabase::findAppointmentById(const std::string& appointmentId) const {
    for (const auto& a : m_appointments) {
        if (a.getAppointmentId() == appointmentId) return &a;
    }
    return nullptr;
}

Appointment* ClinicDatabase::findAppointmentById(const std::string& appointmentId) {
    for (auto& a : m_appointments) {
        if (a.getAppointmentId() == appointmentId) return &a;
    }
    return nullptr;
}

std::vector<Appointment> ClinicDatabase::getAppointmentsForPatient(const std::string& patientId) const {
    std::vector<Appointment> result;
    for (const auto& a : m_appointments) {
        if (a.getPatientId() == patientId) {
            result.push_back(a);
        }
    }
    return result;
}

// --- Waiting Queue Operations (std::queue) ---

void ClinicDatabase::enqueuePatient(const std::string& patientId) {
    m_waitingQueue.push(patientId);
}

std::string ClinicDatabase::dequeuePatient() {
    if (m_waitingQueue.empty()) {
        throw utils::QueueEmptyException();
    }
    std::string nextId = m_waitingQueue.front();
    m_waitingQueue.pop();
    return nextId;
}

std::string ClinicDatabase::peekNextPatient() const {
    if (m_waitingQueue.empty()) return "";
    return m_waitingQueue.front();
}

std::vector<std::string> ClinicDatabase::getWaitingQueueSnapshot() const {
    // std::queue does not provide iterator, copy queue to snapshot
    std::queue<std::string> tempQueue = m_waitingQueue;
    std::vector<std::string> snapshot;
    while (!tempQueue.empty()) {
        snapshot.push_back(tempQueue.front());
        tempQueue.pop();
    }
    return snapshot;
}

// --- Undo Stack Operations (std::stack) ---

void ClinicDatabase::pushUndoOperation(std::shared_ptr<AdminOperation> op) {
    m_undoStack.push(op);
}

std::string ClinicDatabase::getNextUndoDescription() const {
    if (m_undoStack.empty()) return "None";
    return m_undoStack.top()->getDescription();
}

bool ClinicDatabase::undoLastOperation() {
    if (m_undoStack.empty()) return false;
    auto lastOp = m_undoStack.top();
    m_undoStack.pop();
    lastOp->undo(*this);
    return true;
}

// --- Cache Operations (Fixed Array) ---

void ClinicDatabase::recordPatientAccess(PatientRecord* patient) {
    if (!patient) return;

    // Check if already in cache
    int existingIndex = -1;
    for (size_t i = 0; i < m_recentCount; ++i) {
        if (m_recentPatients[i] == patient) {
            existingIndex = static_cast<int>(i);
            break;
        }
    }

    if (existingIndex >= 0) {
        // Move to front (index 0)
        for (int i = existingIndex; i > 0; --i) {
            m_recentPatients[i] = m_recentPatients[i - 1];
        }
        m_recentPatients[0] = patient;
    } else {
        // Shift right, inserting at front
        size_t limit = std::min(m_recentCount, RECENT_CACHE_SIZE - 1);
        for (size_t i = limit; i > 0; --i) {
            m_recentPatients[i] = m_recentPatients[i - 1];
        }
        m_recentPatients[0] = patient;
        if (m_recentCount < RECENT_CACHE_SIZE) {
            m_recentCount++;
        }
    }
}

void ClinicDatabase::invalidateCachedPointer(PatientRecord* patient) {
    if (!patient) return;
    for (size_t i = 0; i < m_recentCount; ++i) {
        if (m_recentPatients[i] == patient) {
            // Shift left to fill gap
            for (size_t j = i; j < m_recentCount - 1; ++j) {
                m_recentPatients[j] = m_recentPatients[j + 1];
            }
            m_recentPatients[m_recentCount - 1] = nullptr;
            m_recentCount--;
            break;
        }
    }
}

void ClinicDatabase::clearAll() {
    m_patients.clear();
    m_appointments.clear();
    while (!m_waitingQueue.empty()) m_waitingQueue.pop();
    while (!m_undoStack.empty()) m_undoStack.pop();
    for (size_t i = 0; i < RECENT_CACHE_SIZE; ++i) {
        m_recentPatients[i] = nullptr;
    }
    m_recentCount = 0;
}

// --- Undo Command Implementations ---

void UndoDeletePatientOperation::undo(ClinicDatabase& db) {
    if (m_deletedPatient) {
        // Re-insert patient without tracking another undo operation
        db.addPatient(m_deletedPatient, false);
        for (const auto& apt : m_associatedAppointments) {
            db.addAppointment(apt, false);
        }
    }
}

void UndoAddPatientOperation::undo(ClinicDatabase& db) {
    db.deletePatient(m_patientId, false);
}

void UndoModifyPatientOperation::undo(ClinicDatabase& db) {
    if (m_previousState) {
        db.updatePatient(m_previousState->getId(),
                         m_previousState->getName(),
                         m_previousState->getAge(),
                         m_previousState->getContact(),
                         false);
    }
}

void UndoRemoveAppointmentOperation::undo(ClinicDatabase& db) {
    // If appointment still exists, revert its status
    Appointment* existing = db.findAppointmentById(m_removedAppointment.getAppointmentId());
    if (existing) {
        existing->setStatus(m_removedAppointment.getStatus());
    } else {
        db.addAppointment(m_removedAppointment, false);
    }
}

void UndoScheduleAppointmentOperation::undo(ClinicDatabase& db) {
    db.deleteAppointment(m_appointmentId, false);
}

} // namespace model
} // namespace scapms
