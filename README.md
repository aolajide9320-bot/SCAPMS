# Smart Clinic Appointment & Patient Management System (SCAPMS)
**COS 202: Computer Programming II — Mid-Semester Lab Assessment Project**  
**Institution:** Miva Open University  

---

## Quick Start & Verification

### 1. Automated Verification & Unit Tests
Run the automated test suite directly to verify all 6 mandatory requirements (OOP, Data Structures, Searching, Sorting, Exceptions, Reports):
```cmd
bin\scapms.exe --test
```

### 2. Launching the Graphical User Interface (GUI)
Launch the interactive desktop application:
```cmd
bin\scapms.exe
```

### 3. Rebuilding the Project from Source
To compile the entire project from scratch with MSVC (C++17):
```cmd
build.bat
```

### 4. Master Submission Documents (DOCX & PDF)
The complete submission document is available in both Word (.docx) and PDF (.pdf) formats:
- **Microsoft Word Document:** `SCAPMS_Midsemester_Assessment_Submission.docx`
- **PDF Document:** `SCAPMS_Midsemester_Assessment_Submission.pdf`

Both documents contain:
1. Cover Page & University Metadata
2. Part 1: Comprehensive Project Report (Sections i through xii)
3. Part 2: Complete UML Class Relationship & Architecture Diagram
4. Part 3: High-Resolution Screenshots & Operational Walkthrough of all 6 GUI views
5. Part 4: Complete Labelled C++ Source Code (Headers, Source Files, GUI Files, Data Files, and Build Script)

---

## Directory Structure
```
SCAPMS/
├── bin/
│   └── scapms.exe                   <- Standalone 64-bit Windows Executable
├── src/
│   ├── model/                       <- Abstract PatientRecord, Appointable, Subclasses, Database
│   │   ├── PatientRecord.h / .cpp
│   │   ├── Appointable.h
│   │   ├── GeneralPatient.h / .cpp
│   │   ├── EmergencyPatient.h / .cpp
│   │   ├── FollowUpPatient.h / .cpp
│   │   ├── Appointment.h / .cpp
│   │   └── ClinicDatabase.h / .cpp
│   ├── controller/                  <- ClinicManager, AppointmentController, Search & Sort
│   │   ├── ClinicManager.h / .cpp
│   │   ├── AppointmentController.h / .cpp
│   │   ├── SearchEngine.h / .cpp
│   │   └── SortEngine.h / .cpp
│   ├── utils/                       <- Exceptions, IDGenerator, FileHandler
│   │   ├── Exceptions.h
│   │   ├── IDGenerator.h / .cpp
│   │   └── FileHandler.h / .cpp
│   ├── gui/                         <- Win32 Event Loop, LayoutManager, Panels, Dialogs
│   │   ├── LayoutManager.h / .cpp
│   │   ├── PatientDialog.h / .cpp
│   │   ├── AppointmentDialog.h / .cpp
│   │   ├── MainWindow.h / .cpp
│   │   └── GuiConstants.h
│   └── main.cpp                     <- Entry point & automated test harness
├── data/
│   ├── patients.csv                 <- Seeded patient records
│   └── appointments.csv             <- Seeded appointment records
├── screenshots/                     <- High-resolution GUI screenshots & UML diagram
│   ├── 1_main_dashboard.png
│   ├── 2_patient_registration.png
│   ├── 3_appointment_management.png
│   ├── 4_waiting_queue.png
│   ├── 5_search_sort_interface.png
│   ├── 6_clinical_reports.png
│   ├── 7_patient_dialog.png
│   ├── 8_appointment_dialog.png
│   └── uml_diagram.png
├── docs/
│   └── PROJECT_REPORT.md            <- Full technical report in Markdown
├── build.bat                        <- Automated MSVC C++17 build script
├── capture_gui.py                   <- Automated screenshot capture script
├── generate_uml.py                  <- UML diagram generation script
├── build_submission_pdf.py          <- Master PDF generator
├── build_submission_docx.py         <- Master Word DOCX generator
├── SCAPMS_Midsemester_Assessment_Submission.docx <- Master Submission DOCX
└── SCAPMS_Midsemester_Assessment_Submission.pdf  <- Master Submission PDF
```

