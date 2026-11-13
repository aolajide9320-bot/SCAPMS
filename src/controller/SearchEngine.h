#ifndef SCAPMS_CONTROLLER_SEARCHENGINE_H
#define SCAPMS_CONTROLLER_SEARCHENGINE_H

#include "../model/PatientRecord.h"
#include <vector>
#include <memory>
#include <string>
#include <chrono>

namespace scapms {
namespace controller {

enum class SearchAlgorithmType {
    LINEAR_SEARCH,
    BINARY_SEARCH,
    RECURSIVE_SEARCH
};

inline std::string searchTypeToString(SearchAlgorithmType type) {
    switch (type) {
        case SearchAlgorithmType::LINEAR_SEARCH: return "Linear Search (Iterative)";
        case SearchAlgorithmType::BINARY_SEARCH: return "Binary Search (Divide & Conquer)";
        case SearchAlgorithmType::RECURSIVE_SEARCH: return "Recursive Search";
        default: return "Unknown";
    }
}

/**
 * @brief Search result structure tracking performance metrics.
 */
struct SearchResult {
    std::shared_ptr<model::PatientRecord> match;
    std::vector<std::shared_ptr<model::PatientRecord>> multipleMatches;
    size_t comparisons{0};
    double durationMicroseconds{0.0};
    std::string algorithmUsed;
    bool found{false};
};

/**
 * @brief Student-implemented Searching Algorithms engine (Mandatory Requirements 4.a, 4.b, 4.c).
 *        No standard library search functions are used.
 */
class SearchEngine {
public:
    // 4.a: Student-implemented Linear Search by Patient ID
    static SearchResult linearSearchByID(
        const std::vector<std::shared_ptr<model::PatientRecord>>& list,
        const std::string& targetId);

    // 4.b: Student-implemented Binary Search by Patient ID (requires sorted list by ID)
    static SearchResult binarySearchByID(
        const std::vector<std::shared_ptr<model::PatientRecord>>& sortedList,
        const std::string& targetId);

    // Binary search by Patient Name (requires sorted list by Name)
    static SearchResult binarySearchByName(
        const std::vector<std::shared_ptr<model::PatientRecord>>& sortedList,
        const std::string& targetName);

    // 4.c: Student-implemented Recursive Search by Patient Name
    static SearchResult recursiveSearchByName(
        const std::vector<std::shared_ptr<model::PatientRecord>>& list,
        const std::string& targetName);

    // General substring search across all fields (Name, ID, Category)
    static std::vector<std::shared_ptr<model::PatientRecord>> filterByKeyword(
        const std::vector<std::shared_ptr<model::PatientRecord>>& list,
        const std::string& keyword);

private:
    // Helper recursive method for recursiveSearchByName
    static int recursiveSearchHelper(
        const std::vector<std::shared_ptr<model::PatientRecord>>& list,
        const std::string& targetLower,
        size_t index,
        size_t& comparisons);

    static std::string toLower(const std::string& str);
};

} // namespace controller
} // namespace scapms

#endif // SCAPMS_CONTROLLER_SEARCHENGINE_H
