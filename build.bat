@echo off
setlocal enabledelayedexpansion

echo ==============================================================
echo   Building Smart Clinic Appointment and Patient Management (SCAPMS)
echo   COS202 Programming II - Lab Assessment Project
echo ==============================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if not exist bin mkdir bin

echo Compiling C++ source files with MSVC C++17...

cl /nologo /std:c++17 /EHsc /O2 /W3 /utf-8 /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS ^
    /I src ^
    src\main.cpp ^
    src\model\PatientRecord.cpp ^
    src\model\GeneralPatient.cpp ^
    src\model\EmergencyPatient.cpp ^
    src\model\FollowUpPatient.cpp ^
    src\model\Appointment.cpp ^
    src\model\ClinicDatabase.cpp ^
    src\controller\ClinicManager.cpp ^
    src\controller\AppointmentController.cpp ^
    src\controller\SearchEngine.cpp ^
    src\controller\SortEngine.cpp ^
    src\utils\IDGenerator.cpp ^
    src\utils\FileHandler.cpp ^
    src\gui\LayoutManager.cpp ^
    src\gui\PatientDialog.cpp ^
    src\gui\AppointmentDialog.cpp ^
    src\gui\MainWindow.cpp ^
    /link /OUT:bin\scapms.exe ^
    user32.lib gdi32.lib comctl32.lib shell32.lib comdlg32.lib

if %ERRORLEVEL% EQU 0 (
    echo ==============================================================
    echo [BUILD SUCCESS] Executable created at bin\scapms.exe
    echo ==============================================================
) else (
    echo ==============================================================
    echo [BUILD FAILED] Please check compilation errors above.
    echo ==============================================================
)
