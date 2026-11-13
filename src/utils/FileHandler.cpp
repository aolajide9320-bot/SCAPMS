#include "FileHandler.h"
#include "Exceptions.h"
#include "IDGenerator.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace scapms {
namespace utils {

namespace {
std::vector<std::string> splitCSVLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, ',')) {
        tokens.push_back(token);
    }
    return tokens;
}
} // namespace

bool FileHandler::saveDatabase(const model::ClinicDatabase& db,
                              const std::string& patientFilePath,
                              const std::string& appointmentFilePath) {
    // 1. Save Patients
    std::ofstream pOut(patientFilePath);
    if (!pOut.is_open()) {
        throw FileIOException(patientFilePath, "Unable to open file for writing patient records.");
    }

    pOut << "Category,ID,Name,Age,Contact,RegistrationDate,Extra1,Extra2,Extra3\n";
    for (const auto& p : db.getAllPatients()) {
        if (p) {
            pOut << p->serialize() << "\n";
        }
    }
    pOut.close();

    // 2. Save Appointments
    std::ofstream aOut(appointmentFilePath);
    if (!aOut.is_open()) {
        throw FileIOException(appointmentFilePath, "Unable to open file for writing appointments.");
    }

    aOut << "AppointmentID,PatientID,PatientName,Date,Time,Department,Doctor,Status,Notes,Fee\n";
    for (const auto& a : db.getAllAppointments()) {
        aOut << a.serialize() << "\n";
    }
    aOut.close();

    return true;
}

bool FileHandler::loadDatabase(model::ClinicDatabase& db,
                              const std::string& patientFilePath,
                              const std::string& appointmentFilePath) {
    // Clear existing records before loading
    db.clearAll();

    // 1. Load Patients
    std::ifstream pIn(patientFilePath);
    if (!pIn.is_open()) {
        throw FileIOException(patientFilePath, "Unable to open patient data file.");
    }

    std::string line;
    bool isFirstLine = true;
    while (std::getline(pIn, line)) {
        if (line.empty()) continue;
        if (isFirstLine) {
            isFirstLine = false;
            // Check if header line
            if (line.find("Category") != std::string::npos) continue;
        }

        auto tokens = splitCSVLine(line);
        if (tokens.size() < 6) continue;

        std::string cat = tokens[0];
        std::string id = tokens[1];
        std::string name = tokens[2];
        int age = std::stoi(tokens[3]);
        std::string contact = tokens[4];
        std::string regDate = tokens[5];

        if (cat == "General") {
            std::string symptoms = tokens.size() > 6 ? tokens[6] : "None";
            bool routine = tokens.size() > 7 ? (tokens[7] == "1") : true;
            db.addPatient(std::make_shared<model::GeneralPatient>(
                id, name, age, contact, symptoms, routine, regDate), false);
        } else if (cat == "Emergency") {
            int severity = tokens.size() > 6 ? std::stoi(tokens[6]) : 2;
            std::string emContact = tokens.size() > 7 ? tokens[7] : "Emergency Contacts";
            db.addPatient(std::make_shared<model::EmergencyPatient>(
                id, name, age, contact, severity, emContact, regDate), false);
        } else if (cat == "Follow-Up") {
            std::string prevDate = tokens.size() > 6 ? tokens[6] : "2026-10-01";
            std::string reason = tokens.size() > 7 ? tokens[7] : "Review";
            std::string doc = tokens.size() > 8 ? tokens[8] : "Dr. Adebayo";
            db.addPatient(std::make_shared<model::FollowUpPatient>(
                id, name, age, contact, prevDate, reason, doc, regDate), false);
        }
    }
    pIn.close();

    // 2. Load Appointments
    std::ifstream aIn(appointmentFilePath);
    if (aIn.is_open()) {
        isFirstLine = true;
        while (std::getline(aIn, line)) {
            if (line.empty()) continue;
            if (isFirstLine) {
                isFirstLine = false;
                if (line.find("AppointmentID") != std::string::npos) continue;
            }

            auto tokens = splitCSVLine(line);
            if (tokens.size() < 10) continue;

            std::string aptId = tokens[0];
            std::string patId = tokens[1];
            std::string patName = tokens[2];
            std::string date = tokens[3];
            std::string time = tokens[4];
            std::string dept = tokens[5];
            std::string doc = tokens[6];
            model::AppointmentStatus status = model::stringToStatus(tokens[7]);
            std::string notes = tokens[8];
            double fee = std::stod(tokens[9]);

            model::Appointment apt(aptId, patId, patName, date, time, dept, doc, status, notes, fee);
            db.addAppointment(apt, false);
        }
        aIn.close();
    }

    return true;
}

bool FileHandler::exportFullDatabase(const model::ClinicDatabase& db, const std::string& filePath) {
    std::ofstream out(filePath);
    if (!out.is_open()) {
        throw FileIOException(filePath, "Failed to open export file.");
    }

    out << "# SMART CLINIC UNIFIED EXPORT\n";
    out << "[PATIENTS]\n";
    for (const auto& p : db.getAllPatients()) {
        if (p) out << p->serialize() << "\n";
    }

    out << "[APPOINTMENTS]\n";
    for (const auto& a : db.getAllAppointments()) {
        out << a.serialize() << "\n";
    }

    out.close();
    return true;
}

bool FileHandler::importFullDatabase(model::ClinicDatabase& db, const std::string& filePath) {
    std::ifstream in(filePath);
    if (!in.is_open()) {
        throw FileIOException(filePath, "Failed to open import file.");
    }

    db.clearAll();
    std::string line;
    std::string currentSection;

    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line == "[PATIENTS]") {
            currentSection = "PATIENTS";
            continue;
        } else if (line == "[APPOINTMENTS]") {
            currentSection = "APPOINTMENTS";
            continue;
        }

        auto tokens = splitCSVLine(line);
        if (currentSection == "PATIENTS" && tokens.size() >= 6) {
            std::string cat = tokens[0];
            std::string id = tokens[1];
            std::string name = tokens[2];
            int age = std::stoi(tokens[3]);
            std::string contact = tokens[4];
            std::string regDate = tokens[5];

            if (cat == "General") {
                std::string symptoms = tokens.size() > 6 ? tokens[6] : "None";
                bool routine = tokens.size() > 7 ? (tokens[7] == "1") : true;
                db.addPatient(std::make_shared<model::GeneralPatient>(
                    id, name, age, contact, symptoms, routine, regDate), false);
            } else if (cat == "Emergency") {
                int severity = tokens.size() > 6 ? std::stoi(tokens[6]) : 2;
                std::string emContact = tokens.size() > 7 ? tokens[7] : "Emergency Contacts";
                db.addPatient(std::make_shared<model::EmergencyPatient>(
                    id, name, age, contact, severity, emContact, regDate), false);
            } else if (cat == "Follow-Up") {
                std::string prevDate = tokens.size() > 6 ? tokens[6] : "2026-10-01";
                std::string reason = tokens.size() > 7 ? tokens[7] : "Review";
                std::string doc = tokens.size() > 8 ? tokens[8] : "Dr. Adebayo";
                db.addPatient(std::make_shared<model::FollowUpPatient>(
                    id, name, age, contact, prevDate, reason, doc, regDate), false);
            }
        } else if (currentSection == "APPOINTMENTS" && tokens.size() >= 10) {
            std::string aptId = tokens[0];
            std::string patId = tokens[1];
            std::string patName = tokens[2];
            std::string date = tokens[3];
            std::string time = tokens[4];
            std::string dept = tokens[5];
            std::string doc = tokens[6];
            model::AppointmentStatus status = model::stringToStatus(tokens[7]);
            std::string notes = tokens[8];
            double fee = std::stod(tokens[9]);

            model::Appointment apt(aptId, patId, patName, date, time, dept, doc, status, notes, fee);
            db.addAppointment(apt, false);
        }
    }

    in.close();
    return true;
}

void FileHandler::loadSampleData(model::ClinicDatabase& db) {
    db.clearAll();

    // 1. General Patients
    db.addPatient(std::make_shared<model::GeneralPatient>(
        "PAT-1001", "Chinedu Okafor", 24, "0803-555-0192", "Mild headache and seasonal allergies", true, "2026-11-01"), false);
    db.addPatient(std::make_shared<model::GeneralPatient>(
        "PAT-1002", "Amina Mohammed", 29, "0802-123-4567", "Annual medical examination", true, "2026-11-02"), false);
    db.addPatient(std::make_shared<model::GeneralPatient>(
        "PAT-1003", "Babatunde Adeleke", 45, "0807-987-6543", "Persistent lower back pain", false, "2026-11-03"), false);
    db.addPatient(std::make_shared<model::GeneralPatient>(
        "PAT-1004", "Grace Danjuma", 19, "0814-222-3344", "Sore throat and mild fever", false, "2026-11-04"), false);
    db.addPatient(std::make_shared<model::GeneralPatient>(
        "PAT-1005", "Emeka Nwosu", 34, "0809-888-7711", "Pre-employment medical screening", true, "2026-11-05"), false);

    // 2. Emergency Patients
    db.addPatient(std::make_shared<model::EmergencyPatient>(
        "PAT-1006", "Fatima Bello", 21, "0805-444-1122", 1, "Alhaji Bello (0803-111-2233)", "2026-11-06"), false);
    db.addPatient(std::make_shared<model::EmergencyPatient>(
        "PAT-1007", "David Olatunji", 52, "0808-333-9988", 2, "Mrs. Olatunji (0808-333-9989)", "2026-11-06"), false);
    db.addPatient(std::make_shared<model::EmergencyPatient>(
        "PAT-1008", "Halima Suleiman", 12, "0812-555-7788", 3, "Mallam Suleiman (0812-555-7789)", "2026-11-06"), false);

    // 3. Follow-Up Patients
    db.addPatient(std::make_shared<model::FollowUpPatient>(
        "PAT-1009", "Samuel Kalu", 61, "0806-777-6655", "2026-10-20", "Hypertension Medication Review", "Dr. Adebayo", "2026-11-01"), false);
    db.addPatient(std::make_shared<model::FollowUpPatient>(
        "PAT-1010", "Zainab Abubakar", 38, "0803-888-9900", "2026-10-25", "Post-Surgical Wound Inspection", "Dr. Eze", "2026-11-02"), false);
    db.addPatient(std::make_shared<model::FollowUpPatient>(
        "PAT-1011", "Tunde Balogun", 48, "0815-999-1122", "2026-10-18", "Type 2 Diabetes Glycemic Monitoring", "Dr. Adebayo", "2026-11-03"), false);

    // Appointments
    db.addAppointment(model::Appointment("APT-2001", "PAT-1001", "Chinedu Okafor", "2026-11-06", "09:00", "General Practice", "Dr. Adebayo", model::AppointmentStatus::COMPLETED, "Routine consultation complete", 40.00), false);
    db.addAppointment(model::Appointment("APT-2002", "PAT-1009", "Samuel Kalu", "2026-11-06", "09:30", "Cardiology", "Dr. Adebayo", model::AppointmentStatus::COMPLETED, "BP stable at 125/80", 25.00), false);
    db.addAppointment(model::Appointment("APT-2003", "PAT-1002", "Amina Mohammed", "2026-11-06", "10:30", "General Practice", "Dr. Eze", model::AppointmentStatus::SCHEDULED, "Comprehensive checkup", 40.00), false);
    db.addAppointment(model::Appointment("APT-2004", "PAT-1003", "Babatunde Adeleke", "2026-11-06", "11:15", "Orthopedics", "Dr. Williams", model::AppointmentStatus::SCHEDULED, "Back spine consultation", 55.00), false);
    db.addAppointment(model::Appointment("APT-2005", "PAT-1010", "Zainab Abubakar", "2026-11-06", "12:00", "Surgery Follow-Up", "Dr. Eze", model::AppointmentStatus::SCHEDULED, "Suture removal and healing check", 25.00), false);
    db.addAppointment(model::Appointment("APT-2006", "PAT-1004", "Grace Danjuma", "2026-11-06", "14:00", "ENT", "Dr. Williams", model::AppointmentStatus::CANCELLED, "Patient requested reschedule", 55.00), false);
    db.addAppointment(model::Appointment("APT-2007", "PAT-1011", "Tunde Balogun", "2026-11-07", "10:00", "Endocrinology", "Dr. Adebayo", model::AppointmentStatus::SCHEDULED, "HbA1c test discussion", 25.00), false);

    // Enqueue some waiting patients in FIFO queue
    db.enqueuePatient("PAT-1002"); // Amina Mohammed
    db.enqueuePatient("PAT-1003"); // Babatunde Adeleke
    db.enqueuePatient("PAT-1007"); // Emergency: David Olatunji
    db.enqueuePatient("PAT-1010"); // Zainab Abubakar

    IDGenerator::setPatientCounterStart(1012);
    IDGenerator::setAppointmentCounterStart(2008);
}

} // namespace utils
} // namespace scapms
