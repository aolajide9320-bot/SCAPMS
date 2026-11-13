#ifndef SCAPMS_CONTROLLER_SORTENGINE_H
#define SCAPMS_CONTROLLER_SORTENGINE_H

#include "../model/PatientRecord.h"
#include <vector>
#include <memory>
#include <string>
#include <chrono>

namespace scapms {
namespace controller {

enum class SortAlgorithmType {
    QUICK_SORT,
    SELECTION_SORT,
    MERGE_SORT
};

enum class SortField {
    BY_NAME,
    BY_ID,
    BY_AGE,
    BY_DATE
};

inline std::string sortAlgorithmToString(SortAlgorithmType type) {
    switch (type) {
        case SortAlgorithmType::QUICK_SORT: return "Quick Sort (Divide & Conquer - O(N log N))";
        case SortAlgorithmType::SELECTION_SORT: return "Selection Sort (Iterative - O(N^2))";
        case SortAlgorithmType::MERGE_SORT: return "Merge Sort (Stable Divide & Conquer - O(N log N))";
        default: return "Unknown";
    }
}

inline std::string sortFieldToString(SortField field) {
    switch (field) {
        case SortField::BY_NAME: return "Patient Name";
        case SortField::BY_ID: return "Patient ID";
        case SortField::BY_AGE: return "Age";
        case SortField::BY_DATE: return "Registration Date";
        default: return "Unknown";
    }
}

/**
 * @brief Performance statistics recorded during manual sorting.
 */
struct SortResult {
    size_t comparisons{0};
    size_t swaps{0};
    double durationMicroseconds{0.0};
    std::string algorithmName;
    std::string fieldName;
    std::string reasonWhyAppropriate;
};

/**
 * @brief Student-implemented Sorting Algorithms (Mandatory Section 5).
 *        No standard library sort functions are used.
 */
class SortEngine {
public:
    static SortResult sort(
        std::vector<std::shared_ptr<model::PatientRecord>>& records,
        SortAlgorithmType algorithm,
        SortField field,
        bool ascending = true);

    // Explicit manual implementations
    static SortResult quickSort(
        std::vector<std::shared_ptr<model::PatientRecord>>& records,
        SortField field,
        bool ascending = true);

    static SortResult selectionSort(
        std::vector<std::shared_ptr<model::PatientRecord>>& records,
        SortField field,
        bool ascending = true);

    static SortResult mergeSort(
        std::vector<std::shared_ptr<model::PatientRecord>>& records,
        SortField field,
        bool ascending = true);

private:
    // Comparison helper function
    static bool compare(
        const std::shared_ptr<model::PatientRecord>& a,
        const std::shared_ptr<model::PatientRecord>& b,
        SortField field,
        bool ascending,
        size_t& comparisons);

    // Quick Sort helpers
    static void quickSortRecursive(
        std::vector<std::shared_ptr<model::PatientRecord>>& records,
        int low, int high,
        SortField field, bool ascending,
        size_t& comparisons, size_t& swaps);

    static int partition(
        std::vector<std::shared_ptr<model::PatientRecord>>& records,
        int low, int high,
        SortField field, bool ascending,
        size_t& comparisons, size_t& swaps);

    // Merge Sort helpers
    static void mergeSortRecursive(
        std::vector<std::shared_ptr<model::PatientRecord>>& records,
        int left, int right,
        SortField field, bool ascending,
        size_t& comparisons, size_t& swaps);

    static void merge(
        std::vector<std::shared_ptr<model::PatientRecord>>& records,
        int left, int mid, int right,
        SortField field, bool ascending,
        size_t& comparisons, size_t& swaps);
};

} // namespace controller
} // namespace scapms

#endif // SCAPMS_CONTROLLER_SORTENGINE_H
