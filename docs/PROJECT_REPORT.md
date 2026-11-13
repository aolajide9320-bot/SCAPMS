# COS 202 Programming II — Lab Assessment Project Report
## Smart Clinic Appointment & Patient Management System (SCAPMS)

**Institution:** Miva Open University  
**Faculty:** Computing & Applied Sciences  
**Course Code:** COS 202 (Computer Programming II)  
**Academic Session:** 2026/2027  
**Submission Date:** November 2026  

---

### i. Project Description
The **Smart Clinic Appointment & Patient Management System (SCAPMS)** is a comprehensive, multi-tier C++ desktop application developed to automate clinical outpatient administration for university healthcare centers. It provides clinic receptionists, triage nurses, and attending physicians with an event-driven graphical user interface to register patients, triage acute emergencies, schedule doctor appointments, maintain an equitable First-In-First-Out (FIFO) waiting queue, execute sub-microsecond searching and multi-attribute sorting, perform single-click undo of administrative operations, and generate executive medical reports.

---

### ii. System Features
- **Multi-Category Patient Management:** Distinguishes between General Outpatients, Emergency Patients (Triage Levels 1–5), and Follow-Up Patients.
- **Appointment Scheduling Engine:** Books, reschedules, cancels, and completes doctor consultations linked to appointable patients.
- **FIFO Waiting Queue:** Utilizes `std::queue` to manage patients waiting for consultation in arrival order.
- **Undo Administrative Stack:** Employs `std::stack` with the Command Design Pattern to reverse patient deletions, record edits, and appointment cancellations.
- **Fixed-Size Cache Array:** Implements `PatientRecord* recentPatients[10]` for O(1) recall of frequently accessed patient files.
- **Student-Implemented Searching Algorithms:** Custom-coded Linear Search (by ID), Binary Search (by ID and Name), and Recursive Search (by Name).
- **Student-Implemented Sorting Algorithms:** Custom-coded Quick Sort, Selection Sort, and Merge Sort allowing sorting by Patient Name, ID, Age, or Date.
- **Event-Driven Graphical User Interface:** Multi-tabbed window (Dashboard, Patients, Appointments, Queue, Search & Sort, Reports) with responsive layout managers (`VBoxLayout`, `HBoxLayout`, `GridLayout`), Segoe UI typography, and live timers.
- **Input Validation & Exception Handling:** Rigorous field checking with custom exception classes (`ClinicException`, `DuplicateIDException`, `InvalidInputException`, `PatientNotFoundException`, `QueueEmptyException`, `FileIOException`).
- **Data Persistence:** Automatic and manual export/import of patient and appointment registries to structured CSV files.

---

### iii. Class Hierarchy & Architectural Design
SCAPMS is organized following the **Model-View-Controller (MVC)** architectural pattern within dedicated namespaces:
- **`scapms::model::PatientRecord` (Abstract Base Class):** Encapsulates identity attributes (`m_id`, `m_name`, `m_age`, `m_contact`, `m_registrationDate`) and defines pure virtual functions: `getPatientType() = 0`, `getDetails() = 0`, `calculateConsultationFee() = 0`, `serialize() = 0`, and `clone() = 0`.
- **`scapms::model::Appointable` (Interface Class):** Defines pure virtual methods `scheduleAppointment()`, `cancelAppointment()`, and `hasActiveAppointment()`. Implemented by `GeneralPatient` and `FollowUpPatient`.
- **`scapms::model::GeneralPatient`:** Inherits from `PatientRecord` and `Appointable`. Models routine outpatient care and computes standard fees ($40 baseline + $15 evaluation).
- **`scapms::model::EmergencyPatient`:** Inherits from `PatientRecord` only. Bypasses `Appointable` because acute emergencies require immediate triage and bypass advance booking. Tracks severity levels 1–5 (Resuscitation to Non-Urgent) and emergency contact kin.
- **`scapms::model::FollowUpPatient`:** Inherits from `PatientRecord` and `Appointable`. Models continuity of care, previous visit dates, attending doctor, and discounted review fees ($25).
- **`scapms::model::Appointment`:** Represents consultation bookings (ID, patient reference, date, time, specialty, status, fee).
- **`scapms::model::ClinicDatabase`:** Central repository encapsulating `std::vector`, `std::queue`, `std::stack`, and the fixed cache array `recentPatients[10]`.
- **`scapms::controller::ClinicManager`:** Facade controller coordinating UI events, patient validation, searching, sorting, and reporting.

---

### iv. Data Structures Used
1. **`std::vector<std::shared_ptr<PatientRecord>>`:** Primary dynamic repository for registered patients, providing O(1) amortized insertion, contiguous cache locality, and random access indexing.
2. **`std::queue<std::string>`:** Manages patients awaiting consultation on a strict First-In-First-Out (FIFO) basis.
3. **`std::stack<std::shared_ptr<AdminOperation>>`:** Powers the Undo feature using the Command Pattern (LIFO), enabling instant rollback of administrative actions.
4. **Fixed-Size C++ Array (`PatientRecord* recentPatients[10]`):** Fixed 10-slot cache with Least-Recently-Used (LRU) displacement for immediate access to active records.

---

### v. Searching Algorithms Used
- **Linear Search by ID (O(N)):** Iteratively scans patient records by unique ID. Operates reliably on unsorted data.
- **Binary Search by ID / Name (O(log N)):** Divides search space in half at each step over sorted arrays. Evaluates 1,000 records in ~10 comparisons.
- **Recursive Linear Search by Name (O(N)):** Evaluates head element and recursively delegates tail search until a match or end-of-list base case is reached.

---

### vi. Sorting Algorithms Used
- **Quick Sort (O(N log N) Average):** In-place divide-and-conquer using Lomuto partitioning. Optimal for general dataset reordering.
- **Selection Sort (O(N^2)):** Scans for the minimum element in each pass. Executes at most O(N) swaps, ideal for memory-constrained caches.
- **Merge Sort (O(N log N) Guaranteed):** Stable recursive divide-and-conquer, preserving initial order of equal keys (e.g., identical ages).

---

### vii. Recursive Algorithm Implementation
Recursion is leveraged across three core subsystems:
1. **`SearchEngine::recursiveSearchHelper`:** Recursively searches for patient names by evaluating `index` against `list.size()` (base case) and recurring on `index + 1`.
2. **`SortEngine::quickSortRecursive`:** Recursively partitions the array into sub-ranges around a pivot element.
3. **`SortEngine::mergeSortRecursive`:** Recursively bisects vectors into halves before merging sorted subarrays.

---

### viii. GUI Design & Layout Management
Constructed using Windows Win32 API and Common Controls with custom layout classes (`VBoxLayout`, `HBoxLayout`, `GridLayout`) to dynamically resize controls during `WM_SIZE` events. The interface provides:
- Dashboard with 5 KPI summary cards, alert banner, and recent cache view.
- Patient management table with custom status tags (`[EMERGENCY - High Priority]`, `[General - Normal]`, `[Follow-Up - Scheduled]`).
- Dynamic components in registration dialog (dynamically toggles fields based on category).
- Waiting queue view with prominent "NOW CALLING" banner and FIFO dequeue button.
- Search & sort tab with real-time diagnostic metrics (comparisons, swaps, duration in microseconds).
- Report tab with pre-built clinical summaries and file export.
- Keyboard shortcuts (`Ctrl+S`, `Ctrl+F`, `Ctrl+Q`, `Ctrl+Z`) and background refresh timers.

---

### ix. Exception Handling Approach
Structured hierarchy derived from `std::runtime_error`:
- `ClinicException` (base)
- `DuplicateIDException`
- `InvalidInputException` (e.g. age not in range 0–120)
- `PatientNotFoundException`
- `AppointmentException`
- `QueueEmptyException`
- `FileIOException`
GUI handlers enclose actions in `try/catch` blocks that present clear modal message dialogs.

---

### x. Algorithm Comparison & Rationale

| Algorithm | Time Complexity (Avg / Worst) | Space | Use Case Justification |
| :--- | :--- | :--- | :--- |
| **Quick Sort** | O(N log N) / O(N^2) | O(log N) | Primary sort engine for large patient records; superior cache performance. |
| **Selection Sort** | O(N^2) / O(N^2) | O(1) | Small cache buffers; minimizes memory writes with at most O(N) swaps. |
| **Merge Sort** | O(N log N) / O(N^2) | O(N) | Stable sorting when relative order of identical keys must be preserved. |
| **Binary Search** | O(log N) / O(log N) | O(1) | Sub-microsecond record lookup once sorted by ID or Name. |
| **Linear Search** | O(N) / O(N) | O(1) | Reliable fallback for unsorted lists and substring filtering. |

---

### xi. Challenges Encountered & xii. Solutions
1. **Shared Ownership vs Raw Cache Pointers:** Resolved by implementing `invalidateCachedPointer()` in `ClinicDatabase::deletePatient()` to remove references before deallocation.
2. **Win32 Macro Name Collisions:** Resolved by defining `#define NOMINMAX` across all headers and MSVC compiler arguments.
3. **Dynamic Form Controls in Native GUI:** Resolved by binding `CBN_SELCHANGE` notifications to `ShowWindow()` toggling for category-specific controls.

