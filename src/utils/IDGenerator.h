#ifndef SCAPMS_UTILS_IDGENERATOR_H
#define SCAPMS_UTILS_IDGENERATOR_H

#include <string>
#include <sstream>
#include <iomanip>
#include <atomic>

namespace scapms {
namespace utils {

/**
 * @brief Thread-safe unique identifier generator for patients and appointments.
 */
class IDGenerator {
private:
    static std::atomic<int> s_patientCounter;
    static std::atomic<int> s_appointmentCounter;

public:
    static std::string generatePatientID(const std::string& prefix = "PAT") {
        int val = ++s_patientCounter;
        std::ostringstream oss;
        oss << prefix << "-" << std::setw(4) << std::setfill('0') << val;
        return oss.str();
    }

    static std::string generateAppointmentID() {
        int val = ++s_appointmentCounter;
        std::ostringstream oss;
        oss << "APT-" << std::setw(4) << std::setfill('0') << val;
        return oss.str();
    }

    static void setPatientCounterStart(int startVal) {
        if (startVal > s_patientCounter.load()) {
            s_patientCounter.store(startVal);
        }
    }

    static void setAppointmentCounterStart(int startVal) {
        if (startVal > s_appointmentCounter.load()) {
            s_appointmentCounter.store(startVal);
        }
    }
};

} // namespace utils
} // namespace scapms

#endif // SCAPMS_UTILS_IDGENERATOR_H
