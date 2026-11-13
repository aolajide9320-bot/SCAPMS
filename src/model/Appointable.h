#ifndef SCAPMS_MODEL_APPOINTABLE_H
#define SCAPMS_MODEL_APPOINTABLE_H

#include <string>
#include <vector>

namespace scapms {
namespace model {

/**
 * @brief Interface-like abstract class defining appointment capabilities for schedulable patient types.
 */
class Appointable {
public:
    virtual ~Appointable() = default;

    /**
     * @brief Schedules an appointment for this patient.
     */
    virtual void scheduleAppointment(const std::string& appointmentId,
                                    const std::string& date,
                                    const std::string& time,
                                    const std::string& reason) = 0;

    /**
     * @brief Cancels an existing appointment.
     */
    virtual bool cancelAppointment(const std::string& appointmentId) = 0;

    /**
     * @brief Checks if the patient has any currently active (scheduled) appointments.
     */
    virtual bool hasActiveAppointment() const = 0;

    /**
     * @brief Gets list of appointment IDs associated with this patient.
     */
    virtual std::vector<std::string> getAppointmentIDs() const = 0;
};

} // namespace model
} // namespace scapms

#endif // SCAPMS_MODEL_APPOINTABLE_H
