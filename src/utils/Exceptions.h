#ifndef SCAPMS_UTILS_EXCEPTIONS_H
#define SCAPMS_UTILS_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace scapms {
namespace utils {

/**
 * @brief Base exception class for all SCAPMS domain exceptions.
 */
class ClinicException : public std::runtime_error {
public:
    explicit ClinicException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Thrown when an attempt is made to insert a patient with an existing ID.
 */
class DuplicateIDException : public ClinicException {
public:
    explicit DuplicateIDException(const std::string& id)
        : ClinicException("Error: Patient ID '" + id + "' already exists in the clinic database.") {}
};

/**
 * @brief Thrown when input data fails validation rules (e.g., invalid age, blank name).
 */
class InvalidInputException : public ClinicException {
public:
    explicit InvalidInputException(const std::string& field, const std::string& reason)
        : ClinicException("Validation Error in '" + field + "': " + reason) {}
};

/**
 * @brief Thrown when a patient record or appointment is not found.
 */
class PatientNotFoundException : public ClinicException {
public:
    explicit PatientNotFoundException(const std::string& id)
        : ClinicException("Error: Patient with ID '" + id + "' was not found.") {}
};

/**
 * @brief Thrown when an appointment operation fails.
 */
class AppointmentException : public ClinicException {
public:
    explicit AppointmentException(const std::string& message)
        : ClinicException("Appointment Error: " + message) {}
};

/**
 * @brief Thrown when file read/write operations encounter an error.
 */
class FileIOException : public ClinicException {
public:
    explicit FileIOException(const std::string& filename, const std::string& detail)
        : ClinicException("File I/O Error on '" + filename + "': " + detail) {}
};

/**
 * @brief Thrown when attempting to dequeue from an empty waiting queue.
 */
class QueueEmptyException : public ClinicException {
public:
    QueueEmptyException()
        : ClinicException("Notice: The waiting queue is currently empty. No patient to call.") {}
};

} // namespace utils
} // namespace scapms

#endif // SCAPMS_UTILS_EXCEPTIONS_H
