#include "SortEngine.h"
#include <utility>

namespace scapms {
namespace controller {

bool SortEngine::compare(
    const std::shared_ptr<model::PatientRecord>& a,
    const std::shared_ptr<model::PatientRecord>& b,
    SortField field,
    bool ascending,
    size_t& comparisons) {

    comparisons++;
    if (!a || !b) return false;

    bool less = false;
    switch (field) {
        case SortField::BY_NAME:
            less = a->getName() < b->getName();
            break;
        case SortField::BY_ID:
            less = a->getId() < b->getId();
            break;
        case SortField::BY_AGE:
            less = a->getAge() < b->getAge();
            break;
        case SortField::BY_DATE:
            less = a->getRegistrationDate() < b->getRegistrationDate();
            break;
    }

    return ascending ? less : !less;
}

SortResult SortEngine::sort(
    std::vector<std::shared_ptr<model::PatientRecord>>& records,
    SortAlgorithmType algorithm,
    SortField field,
    bool ascending) {

    switch (algorithm) {
        case SortAlgorithmType::QUICK_SORT:
            return quickSort(records, field, ascending);
        case SortAlgorithmType::SELECTION_SORT:
            return selectionSort(records, field, ascending);
        case SortAlgorithmType::MERGE_SORT:
            return mergeSort(records, field, ascending);
        default:
            return quickSort(records, field, ascending);
    }
}

// -------------------------------------------------------------
// Quick Sort Implementation (O(N log N))
// -------------------------------------------------------------
SortResult SortEngine::quickSort(
    std::vector<std::shared_ptr<model::PatientRecord>>& records,
    SortField field,
    bool ascending) {

    SortResult res;
    res.algorithmName = "Quick Sort";
    res.fieldName = sortFieldToString(field);
    res.reasonWhyAppropriate =
        "Quick Sort is optimal for general clinic database reordering because of its average "
        "O(N log N) runtime complexity and in-place partitioning with minimal memory overhead.";

    auto start = std::chrono::high_resolution_clock::now();

    if (records.size() > 1) {
        quickSortRecursive(records, 0, static_cast<int>(records.size()) - 1,
                           field, ascending, res.comparisons, res.swaps);
    }

    auto end = std::chrono::high_resolution_clock::now();
    res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
    return res;
}

void SortEngine::quickSortRecursive(
    std::vector<std::shared_ptr<model::PatientRecord>>& records,
    int low, int high,
    SortField field, bool ascending,
    size_t& comparisons, size_t& swaps) {

    if (low < high) {
        int pi = partition(records, low, high, field, ascending, comparisons, swaps);
        quickSortRecursive(records, low, pi - 1, field, ascending, comparisons, swaps);
        quickSortRecursive(records, pi + 1, high, field, ascending, comparisons, swaps);
    }
}

int SortEngine::partition(
    std::vector<std::shared_ptr<model::PatientRecord>>& records,
    int low, int high,
    SortField field, bool ascending,
    size_t& comparisons, size_t& swaps) {

    const auto& pivot = records[high];
    int i = low - 1;

    for (int j = low; j < high; ++j) {
        if (compare(records[j], pivot, field, ascending, comparisons)) {
            i++;
            std::swap(records[i], records[j]);
            swaps++;
        }
    }
    std::swap(records[i + 1], records[high]);
    swaps++;
    return i + 1;
}

// -------------------------------------------------------------
// Selection Sort Implementation (O(N^2))
// -------------------------------------------------------------
SortResult SortEngine::selectionSort(
    std::vector<std::shared_ptr<model::PatientRecord>>& records,
    SortField field,
    bool ascending) {

    SortResult res;
    res.algorithmName = "Selection Sort";
    res.fieldName = sortFieldToString(field);
    res.reasonWhyAppropriate =
        "Selection Sort is appropriate for small batch lists or memory-constrained caches "
        "because it minimizes memory writes and performs at most O(N) swaps.";

    auto start = std::chrono::high_resolution_clock::now();

    int n = static_cast<int>(records.size());
    for (int i = 0; i < n - 1; ++i) {
        int targetIdx = i;
        for (int j = i + 1; j < n; ++j) {
            if (compare(records[j], records[targetIdx], field, ascending, res.comparisons)) {
                targetIdx = j;
            }
        }
        if (targetIdx != i) {
            std::swap(records[i], records[targetIdx]);
            res.swaps++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
    return res;
}

// -------------------------------------------------------------
// Merge Sort Implementation (O(N log N) Stable)
// -------------------------------------------------------------
SortResult SortEngine::mergeSort(
    std::vector<std::shared_ptr<model::PatientRecord>>& records,
    SortField field,
    bool ascending) {

    SortResult res;
    res.algorithmName = "Merge Sort";
    res.fieldName = sortFieldToString(field);
    res.reasonWhyAppropriate =
        "Merge Sort is ideal when stable sorting is required (preserving existing order of "
        "patients with identical ages or dates) with guaranteed worst-case O(N log N) performance.";

    auto start = std::chrono::high_resolution_clock::now();

    if (records.size() > 1) {
        mergeSortRecursive(records, 0, static_cast<int>(records.size()) - 1,
                           field, ascending, res.comparisons, res.swaps);
    }

    auto end = std::chrono::high_resolution_clock::now();
    res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
    return res;
}

void SortEngine::mergeSortRecursive(
    std::vector<std::shared_ptr<model::PatientRecord>>& records,
    int left, int right,
    SortField field, bool ascending,
    size_t& comparisons, size_t& swaps) {

    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortRecursive(records, left, mid, field, ascending, comparisons, swaps);
        mergeSortRecursive(records, mid + 1, right, field, ascending, comparisons, swaps);
        merge(records, left, mid, right, field, ascending, comparisons, swaps);
    }
}

void SortEngine::merge(
    std::vector<std::shared_ptr<model::PatientRecord>>& records,
    int left, int mid, int right,
    SortField field, bool ascending,
    size_t& comparisons, size_t& swaps) {

    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<std::shared_ptr<model::PatientRecord>> L(n1);
    std::vector<std::shared_ptr<model::PatientRecord>> R(n2);

    for (int i = 0; i < n1; ++i) L[i] = records[left + i];
    for (int j = 0; j < n2; ++j) R[j] = records[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (compare(L[i], R[j], field, ascending, comparisons) || L[i] == R[j]) {
            records[k] = L[i];
            i++;
        } else {
            records[k] = R[j];
            j++;
        }
        swaps++;
        k++;
    }

    while (i < n1) {
        records[k] = L[i];
        i++;
        k++;
        swaps++;
    }

    while (j < n2) {
        records[k] = R[j];
        j++;
        k++;
        swaps++;
    }
}

} // namespace controller
} // namespace scapms
