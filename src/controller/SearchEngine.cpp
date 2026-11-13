#include "SearchEngine.h"
#include <algorithm>
#include <cctype>

namespace scapms {
namespace controller {

std::string SearchEngine::toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return lower;
}

// 4.a: Linear Search by Patient ID
SearchResult SearchEngine::linearSearchByID(
    const std::vector<std::shared_ptr<model::PatientRecord>>& list,
    const std::string& targetId) {

    SearchResult res;
    res.algorithmUsed = "Linear Search by ID (O(N))";
    auto start = std::chrono::high_resolution_clock::now();

    std::string searchTarget = toLower(targetId);

    for (size_t i = 0; i < list.size(); ++i) {
        res.comparisons++;
        if (list[i] && toLower(list[i]->getId()) == searchTarget) {
            res.match = list[i];
            res.multipleMatches.push_back(list[i]);
            res.found = true;
            break; // Stop at first exact ID match
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
    return res;
}

// 4.b: Binary Search by Patient ID
SearchResult SearchEngine::binarySearchByID(
    const std::vector<std::shared_ptr<model::PatientRecord>>& sortedList,
    const std::string& targetId) {

    SearchResult res;
    res.algorithmUsed = "Binary Search by ID (O(log N))";
    auto start = std::chrono::high_resolution_clock::now();

    std::string target = toLower(targetId);
    int low = 0;
    int high = static_cast<int>(sortedList.size()) - 1;

    while (low <= high) {
        res.comparisons++;
        int mid = low + (high - low) / 2;
        if (!sortedList[mid]) {
            break;
        }

        std::string midId = toLower(sortedList[mid]->getId());
        if (midId == target) {
            res.match = sortedList[mid];
            res.multipleMatches.push_back(sortedList[mid]);
            res.found = true;
            break;
        } else if (midId < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
    return res;
}

// Binary Search by Patient Name
SearchResult SearchEngine::binarySearchByName(
    const std::vector<std::shared_ptr<model::PatientRecord>>& sortedList,
    const std::string& targetName) {

    SearchResult res;
    res.algorithmUsed = "Binary Search by Name (O(log N))";
    auto start = std::chrono::high_resolution_clock::now();

    std::string target = toLower(targetName);
    int low = 0;
    int high = static_cast<int>(sortedList.size()) - 1;

    while (low <= high) {
        res.comparisons++;
        int mid = low + (high - low) / 2;
        if (!sortedList[mid]) break;

        std::string midName = toLower(sortedList[mid]->getName());
        if (midName == target) {
            res.match = sortedList[mid];
            res.multipleMatches.push_back(sortedList[mid]);
            res.found = true;
            break;
        } else if (midName < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
    return res;
}

// 4.c: Recursive Search by Patient Name
SearchResult SearchEngine::recursiveSearchByName(
    const std::vector<std::shared_ptr<model::PatientRecord>>& list,
    const std::string& targetName) {

    SearchResult res;
    res.algorithmUsed = "Recursive Linear Search by Name (O(N))";
    auto start = std::chrono::high_resolution_clock::now();

    std::string target = toLower(targetName);
    size_t comparisons = 0;
    int foundIndex = recursiveSearchHelper(list, target, 0, comparisons);

    res.comparisons = comparisons;
    if (foundIndex >= 0 && static_cast<size_t>(foundIndex) < list.size()) {
        res.match = list[foundIndex];
        res.multipleMatches.push_back(list[foundIndex]);
        res.found = true;
    }

    auto end = std::chrono::high_resolution_clock::now();
    res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
    return res;
}

int SearchEngine::recursiveSearchHelper(
    const std::vector<std::shared_ptr<model::PatientRecord>>& list,
    const std::string& targetLower,
    size_t index,
    size_t& comparisons) {

    // Base Case 1: Reached end of list without match
    if (index >= list.size()) {
        return -1;
    }

    comparisons++;
    // Base Case 2: Match found at current index
    if (list[index] && toLower(list[index]->getName()) == targetLower) {
        return static_cast<int>(index);
    }

    // Recursive Step: Check tail of list
    return recursiveSearchHelper(list, targetLower, index + 1, comparisons);
}

std::vector<std::shared_ptr<model::PatientRecord>> SearchEngine::filterByKeyword(
    const std::vector<std::shared_ptr<model::PatientRecord>>& list,
    const std::string& keyword) {

    if (keyword.empty()) return list;

    std::string needle = toLower(keyword);
    std::vector<std::shared_ptr<model::PatientRecord>> matches;

    for (const auto& p : list) {
        if (!p) continue;
        if (toLower(p->getId()).find(needle) != std::string::npos ||
            toLower(p->getName()).find(needle) != std::string::npos ||
            toLower(p->getPatientType()).find(needle) != std::string::npos ||
            toLower(p->getContact()).find(needle) != std::string::npos) {
            matches.push_back(p);
        }
    }
    return matches;
}

} // namespace controller
} // namespace scapms
