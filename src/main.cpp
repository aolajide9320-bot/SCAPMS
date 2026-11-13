#include <windows.h>
#include <iostream>
#include <memory>
#include <cassert>
#include <cstring>
#include "model/PatientRecord.h"
#include "model/GeneralPatient.h"
#include "model/EmergencyPatient.h"
#include "model/FollowUpPatient.h"
#include "model/ClinicDatabase.h"
#include "controller/ClinicManager.h"
#include "controller/AppointmentController.h"
#include "controller/SearchEngine.h"
#include "controller/SortEngine.h"
#include "utils/Exceptions.h"
#include "utils/FileHandler.h"
#include "gui/MainWindow.h"

// Explicit demonstration of runtime polymorphism (Mandatory Requirement 1.d)
void demonstratePolymorphism() {
    std::cout << "========================================================\n";
    std::cout << "  DEMONSTRATING RUNTIME POLYMORPHISM VIA BASE REFERENCE \n";
    std::cout << "========================================================\n";

    std::vector<std::shared_ptr<scapms::model::PatientRecord>> sampleClinic;
    sampleClinic.push_back(std::make_shared<scapms::model::GeneralPatient>(
        "PAT-DEMO1", "Ada Lovelace", 32, "0801-111-2222", "Routine health screening", true));
    sampleClinic.push_back(std::make_shared<scapms::model::EmergencyPatient>(
        "PAT-DEMO2", "Alan Turing", 41, "0802-333-4444", 1, "Emergency Contact (911)"));
    sampleClinic.push_back(std::make_shared<scapms::model::FollowUpPatient>(
        "PAT-DEMO3", "Grace Hopper", 55, "0803-555-6666", "2026-10-15", "Cardiology Review", "Dr. Adebayo"));

    for (const auto& patientPtr : sampleClinic) {
        // Polymorphic invocation through base class reference
        scapms::model::processPatientRecord(*patientPtr, std::cout);
    }
    std::cout << "========================================================\n\n";
}

int runAutomatedVerification() {
    std::cout << "\n========================================================\n";
    std::cout << "   RUNNING AUTOMATED UNIT TESTS & REQUIREMENT CHECKS     \n";
    std::cout << "========================================================\n";

    // 1. Test OOP & Polymorphism
    demonstratePolymorphism();

    // 2. Test Data Structures (std::vector, std::queue, std::stack, fixed-array cache)
    std::cout << "[Test 2] Testing Data Structures & Operations...\n";
    scapms::model::ClinicDatabase db;
    scapms::utils::FileHandler::loadSampleData(db);
    std::cout << " -> Vector: Loaded " << db.getPatientCount() << " registered patients.\n";
    assert(db.getPatientCount() > 0);

    // Waiting queue (FIFO)
    size_t qInitial = db.getQueueSize();
    std::cout << " -> Queue: Initial waiting queue size = " << qInitial << "\n";
    std::string nextInQueue = db.peekNextPatient();
    std::string dequeued = db.dequeuePatient();
    assert(nextInQueue == dequeued);
    std::cout << " -> Queue FIFO: Dequeued patient ID " << dequeued << " (served first).\n";

    // Stack (Undo)
    std::cout << " -> Stack: Testing Undo functionality...\n";
    size_t beforeDelete = db.getPatientCount();
    db.deletePatient("PAT-1001", true);
    assert(db.getPatientCount() == beforeDelete - 1);
    std::cout << "    Deleted PAT-1001. Patient count now = " << db.getPatientCount() << "\n";
    std::cout << "    Top of Undo Stack: " << db.getNextUndoDescription() << "\n";
    bool undoOk = db.undoLastOperation();
    assert(undoOk);
    assert(db.getPatientCount() == beforeDelete);
    std::cout << "    Undo executed! PAT-1001 restored. Patient count = " << db.getPatientCount() << "\n";

    // Fixed array cache (10 items)
    std::cout << " -> Array Cache: Cache size = " << db.getRecentCacheCount() << "/10 slots utilized.\n";
    assert(db.getRecentCacheCount() <= 10);

    // 3. Test Searching Algorithms (Student-implemented)
    std::cout << "\n[Test 3] Testing Student-Implemented Searching Algorithms...\n";
    auto linearRes = scapms::controller::SearchEngine::linearSearchByID(db.getAllPatients(), "PAT-1002");
    std::cout << " -> Linear Search: Found=" << (linearRes.found ? "YES" : "NO")
              << " | Comparisons=" << linearRes.comparisons
              << " | Time=" << linearRes.durationMicroseconds << " us\n";
    assert(linearRes.found);

    // Prepare sorted list for binary search
    auto sortedPatients = db.getAllPatients();
    scapms::controller::SortEngine::quickSort(sortedPatients, scapms::controller::SortField::BY_ID, true);
    auto binRes = scapms::controller::SearchEngine::binarySearchByID(sortedPatients, "PAT-1002");
    std::cout << " -> Binary Search: Found=" << (binRes.found ? "YES" : "NO")
              << " | Comparisons=" << binRes.comparisons
              << " | Time=" << binRes.durationMicroseconds << " us\n";
    assert(binRes.found);

    auto recRes = scapms::controller::SearchEngine::recursiveSearchByName(db.getAllPatients(), "Amina Mohammed");
    std::cout << " -> Recursive Search: Found=" << (recRes.found ? "YES" : "NO")
              << " | Comparisons=" << recRes.comparisons
              << " | Time=" << recRes.durationMicroseconds << " us\n";
    assert(recRes.found);

    // 4. Test Sorting Algorithms (Student-implemented)
    std::cout << "\n[Test 4] Testing Student-Implemented Sorting Algorithms...\n";
    auto testList1 = db.getAllPatients();
    auto qsRes = scapms::controller::SortEngine::quickSort(testList1, scapms::controller::SortField::BY_NAME, true);
    std::cout << " -> Quick Sort (by Name): Comparisons=" << qsRes.comparisons
              << " | Swaps=" << qsRes.swaps << " | Time=" << qsRes.durationMicroseconds << " us\n";
    for (size_t i = 1; i < testList1.size(); ++i) {
        assert(testList1[i - 1]->getName() <= testList1[i]->getName());
    }

    auto testList2 = db.getAllPatients();
    auto selRes = scapms::controller::SortEngine::selectionSort(testList2, scapms::controller::SortField::BY_AGE, true);
    std::cout << " -> Selection Sort (by Age): Comparisons=" << selRes.comparisons
              << " | Swaps=" << selRes.swaps << " | Time=" << selRes.durationMicroseconds << " us\n";
    for (size_t i = 1; i < testList2.size(); ++i) {
        assert(testList2[i - 1]->getAge() <= testList2[i]->getAge());
    }

    auto testList3 = db.getAllPatients();
    auto msRes = scapms::controller::SortEngine::mergeSort(testList3, scapms::controller::SortField::BY_ID, true);
    std::cout << " -> Merge Sort (by ID): Comparisons=" << msRes.comparisons
              << " | Swaps=" << msRes.swaps << " | Time=" << msRes.durationMicroseconds << " us\n";
    for (size_t i = 1; i < testList3.size(); ++i) {
        assert(testList3[i - 1]->getId() <= testList3[i]->getId());
    }

    // 5. Test Exception Handling
    std::cout << "\n[Test 5] Testing Custom Exception Hierarchy...\n";
    bool caughtDuplicate = false;
    try {
        db.addPatient(std::make_shared<scapms::model::GeneralPatient>(
            "PAT-1002", "Duplicate Tester", 30, "0800-000-0000"), true);
    } catch (const scapms::utils::DuplicateIDException& e) {
        caughtDuplicate = true;
        std::cout << " -> Caught expected DuplicateIDException: " << e.what() << "\n";
    }
    assert(caughtDuplicate);

    bool caughtInvalidAge = false;
    try {
        scapms::controller::ClinicManager::validatePatientInput("Test User", 150, "0800");
    } catch (const scapms::utils::InvalidInputException& e) {
        caughtInvalidAge = true;
        std::cout << " -> Caught expected InvalidInputException: " << e.what() << "\n";
    }
    assert(caughtInvalidAge);

    // 6. Test Reports
    std::cout << "\n[Test 6] Testing Clinical Reporting Engine...\n";
    scapms::controller::AppointmentController aptCtrl(db);
    auto topAttended = aptCtrl.getMostFrequentlyAttendedPatients(3);
    std::cout << " -> Top Frequently Attended Count: " << topAttended.size() << "\n";
    auto catDist = aptCtrl.getCategoryDistribution();
    std::cout << " -> Category Distribution: General=" << catDist.generalCount
              << ", Emergency=" << catDist.emergencyCount
              << ", FollowUp=" << catDist.followUpCount
              << ", Total=" << catDist.total << "\n";
    auto dailySummary = aptCtrl.getDailySummary();
    std::cout << " -> Daily Summary: Total=" << dailySummary.totalAppointments
              << ", Completed=" << dailySummary.completed
              << ", Pending=" << dailySummary.pending << "\n";

    std::cout << "\n========================================================\n";
    std::cout << "   [ALL 6 CORE REQUIREMENT TESTS PASSED SUCCESSFULLY]    \n";
    std::cout << "========================================================\n\n";
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow) {
    // Initialize GUI Application
    scapms::gui::MainWindow mainWindow;
    if (!mainWindow.initialize(hInstance, nCmdShow)) {
        MessageBoxA(NULL, "Failed to initialize SCAPMS graphical user interface window.",
                    "Initialization Error", MB_ICONERROR);
        return 1;
    }

    return mainWindow.runMessageLoop();
}

int main(int argc, char* argv[]) {
    if (argc > 1 && (std::strcmp(argv[1], "--test") == 0 || std::strcmp(argv[1], "-t") == 0)) {
        return runAutomatedVerification();
    }

    // Default: print polymorphism demo then launch GUI
    demonstratePolymorphism();

    HINSTANCE hInstance = GetModuleHandle(NULL);
    return WinMain(hInstance, NULL, GetCommandLineA(), SW_SHOW);
}
