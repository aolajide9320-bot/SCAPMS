#include "IDGenerator.h"

namespace scapms {
namespace utils {

std::atomic<int> IDGenerator::s_patientCounter{1000};
std::atomic<int> IDGenerator::s_appointmentCounter{2000};

} // namespace utils
} // namespace scapms
