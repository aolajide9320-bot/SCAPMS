#ifndef SCAPMS_UTILS_FILEHANDLER_H
#define SCAPMS_UTILS_FILEHANDLER_H

#include "../model/ClinicDatabase.h"
#include <string>

namespace scapms {
namespace utils {

/**
 * @brief Handles file persistence for patient records and appointments (Mandatory Requirement 11.d).
 */
class FileHandler {
public:
    static bool saveDatabase(const model::ClinicDatabase& db,
                             const std::string& patientFilePath,
                             const std::string& appointmentFilePath);

    static bool loadDatabase(model::ClinicDatabase& db,
                             const std::string& patientFilePath,
                             const std::string& appointmentFilePath);

    // Save/load to unified CSV/text file
    static bool exportFullDatabase(const model::ClinicDatabase& db,
                                   const std::string& filePath);

    static bool importFullDatabase(model::ClinicDatabase& db,
                                   const std::string& filePath);

    // Seed default clinic records for testing and demonstration
    static void loadSampleData(model::ClinicDatabase& db);
};

} // namespace utils
} // namespace scapms

#endif // SCAPMS_UTILS_FILEHANDLER_H
