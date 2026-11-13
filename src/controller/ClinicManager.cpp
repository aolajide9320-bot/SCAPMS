#include "ClinicManager.h"
#include "../utils/IDGenerator.h"
#include "../utils/Exceptions.h"
#include "../utils/FileHandler.h"
#include <regex>

namespace scapms {
namespace controller {

ClinicManager::ClinicManager()
    : m_appointmentController(m_database) {
    // Load initial clinic sample data
    utils::FileHandler::loadSampleData(m_database);
}

void ClinicManager::validatePatientInput(const std::string& name, int age, const std::string& contact) {
    if (name.empty() || name.find_first_not_of(" \t\n\r") == std::string::npos) {
        throw utils::InvalidInputException("Name", "Patient name cannot be blank.");
    }
    if (age < 0 || age > 120) {
        throw utils::InvalidInputException("Age", "Patient age must be between 0 and 120 years.");
    }
    if (contact.empty() || contact.find_first_not_of(" \t\n\r") == std::string::npos) {
        throw utils::InvalidInputException("Contact", "Contact phone number cannot be blank.");
    }
}

std::string ClinicManager::registerGeneralPatient(
    const std::string& name, int age,
    const std::string& contact,
    const std::string& symptoms,
    bool routineCheckup) {

    validatePatientInput(name, age, contact);
    std::string id = utils::IDGenerator::generatePatientID("GEN");

    auto p = std::make_shared<model::GeneralPatient>(id, name, age, contact, symptoms, routineCheckup);
    m_database.addPatient(p, true);
    return id;
}

std::string ClinicManager::registerEmergencyPatient(
    const std::string& name, int age,
    const std::string& contact,
    int severityLevel,
    const std::string& emergencyContact) {

    validatePatientInput(name, age, contact);
    if (severityLevel < 1 || severityLevel > 5) {
        throw utils::InvalidInputException("Severity Level", "Triage severity must be between 1 (Critical) and 5 (Non-urgent).");
    }
    if (emergencyContact.empty()) {
        throw utils::InvalidInputException("Emergency Contact", "Emergency contact details are required.");
    }

    std::string id = utils::IDGenerator::generatePatientID("EMG");
    auto p = std::make_shared<model::EmergencyPatient>(id, name, age, contact, severityLevel, emergencyContact);
    m_database.addPatient(p, true);

    // Emergency patients are automatically added to the front of the queue or waiting queue
    m_database.enqueuePatient(id);

    return id;
}

std::string ClinicManager::registerFollowUpPatient(
    const std::string& name, int age,
    const std::string& contact,
    const std::string& previousVisitDate,
    const std::string& followUpReason,
    const std::string& attendingDoctor) {

    validatePatientInput(name, age, contact);
    if (previousVisitDate.empty()) {
        throw utils::InvalidInputException("Previous Visit Date", "Previous visit date is required for follow-up patients.");
    }

    std::string id = utils::IDGenerator::generatePatientID("FLP");
    auto p = std::make_shared<model::FollowUpPatient>(
        id, name, age, contact, previousVisitDate, followUpReason, attendingDoctor);
    m_database.addPatient(p, true);
    return id;
}

bool ClinicManager::updatePatient(const std::string& id, const std::string& name, int age, const std::string& contact) {
    validatePatientInput(name, age, contact);
    return m_database.updatePatient(id, name, age, contact, true);
}

bool ClinicManager::deletePatient(const std::string& id) {
    return m_database.deletePatient(id, true);
}

std::shared_ptr<model::PatientRecord> ClinicManager::getPatient(const std::string& id) {
    return m_database.findPatientById(id);
}

// Waiting Queue
void ClinicManager::addPatientToQueue(const std::string& patientId) {
    auto p = m_database.findPatientById(patientId);
    if (!p) {
        throw utils::PatientNotFoundException(patientId);
    }
    m_database.enqueuePatient(patientId);
}

std::shared_ptr<model::PatientRecord> ClinicManager::callNextPatient() {
    std::string nextId = m_database.dequeuePatient();
    return m_database.findPatientById(nextId);
}

std::vector<std::shared_ptr<model::PatientRecord>> ClinicManager::getWaitingPatientsList() {
    auto snapshot = m_database.getWaitingQueueSnapshot();
    std::vector<std::shared_ptr<model::PatientRecord>> result;
    for (const auto& id : snapshot) {
        auto p = m_database.findPatientById(id);
        if (p) result.push_back(p);
    }
    return result;
}

// Undo
bool ClinicManager::undoLastAction() {
    return m_database.undoLastOperation();
}

std::string ClinicManager::getUndoActionDescription() const {
    return m_database.getNextUndoDescription();
}

// Cache
std::vector<const model::PatientRecord*> ClinicManager::getRecentlyAccessedPatients() const {
    std::vector<const model::PatientRecord*> list;
    for (size_t i = 0; i < m_database.getRecentCacheCount(); ++i) {
        const auto* ptr = m_database.getCachedPatient(i);
        if (ptr) list.push_back(ptr);
    }
    return list;
}

// Search
SearchResult ClinicManager::search(const std::string& query, SearchAlgorithmType algo, const std::string& field) {
    const auto& list = m_database.getAllPatients();

    if (algo == SearchAlgorithmType::LINEAR_SEARCH) {
        return SearchEngine::linearSearchByID(list, query);
    } else if (algo == SearchAlgorithmType::BINARY_SEARCH) {
        // Prepare sorted copy for binary search
        auto sortedList = list;
        if (field == "Name") {
            SortEngine::quickSort(sortedList, SortField::BY_NAME, true);
            return SearchEngine::binarySearchByName(sortedList, query);
        } else {
            SortEngine::quickSort(sortedList, SortField::BY_ID, true);
            return SearchEngine::binarySearchByID(sortedList, query);
        }
    } else { // RECURSIVE_SEARCH
        return SearchEngine::recursiveSearchByName(list, query);
    }
}

// Sort
SortResult ClinicManager::sort(SortAlgorithmType algo, SortField field, bool ascending) {
    // We sort the internal vector or an extracted vector
    // Let's sort the database's internal patient list directly
    // Wait, m_database returns const reference for getAllPatients, so let's allow sorting
    // We can cast away const for internal in-place sort
    auto& list = const_cast<std::vector<std::shared_ptr<model::PatientRecord>>&>(m_database.getAllPatients());
    return SortEngine::sort(list, algo, field, ascending);
}

// Persistence
bool ClinicManager::saveToDisk(const std::string& pPath, const std::string& aPath) {
    return utils::FileHandler::saveDatabase(m_database, pPath, aPath);
}

bool ClinicManager::loadFromDisk(const std::string& pPath, const std::string& aPath) {
    return utils::FileHandler::loadDatabase(m_database, pPath, aPath);
}

void ClinicManager::resetWithSampleData() {
    utils::FileHandler::loadSampleData(m_database);
}

} // namespace controller
} // namespace scapms
