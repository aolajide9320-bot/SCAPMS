import os
import sys
from reportlab.lib.pagesizes import letter
from reportlab.lib import colors
from reportlab.lib.units import inch
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, Image, PageBreak, KeepTogether, Preformatted
)
from reportlab.pdfgen import canvas

class NumberedCanvas(canvas.Canvas):
    def __init__(self, *args, **kwargs):
        super(NumberedCanvas, self).__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_page_number(num_pages)
            canvas.Canvas.showPage(self)
        canvas.Canvas.save(self)

    def draw_page_number(self, page_count):
        if self._pageNumber == 1:
            return  # Suppress headers/footers on title page
        self.saveState()
        self.setFont("Helvetica", 9)
        self.setFillColor(colors.HexColor("#475569"))

        # Header
        self.drawString(54, letter[1] - 36, "COS202 Programming II — Smart Clinic Appointment & Patient Management (SCAPMS)")
        self.setStrokeColor(colors.HexColor("#CBD5E1"))
        self.setLineWidth(0.5)
        self.line(54, letter[1] - 42, letter[0] - 54, letter[1] - 42)

        # Footer
        text = f"Page {self._pageNumber} of {page_count}"
        self.drawRightString(letter[0] - 54, 32, text)
        self.drawString(54, 32, "Mid-Semester Lab Assessment Project Document | Miva Open University")
        self.line(54, 44, letter[0] - 54, 44)
        self.restoreState()

def build_pdf(output_pdf_path):
    doc = SimpleDocTemplate(
        output_pdf_path,
        pagesize=letter,
        leftMargin=54,
        rightMargin=54,
        topMargin=54,
        bottomMargin=54
    )

    styles = getSampleStyleSheet()

    # Custom typography styles
    title_style = ParagraphStyle(
        'DocTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=24,
        leading=28,
        textColor=colors.HexColor("#1E3A8A"),
        alignment=1, # Center
        spaceAfter=12
    )

    subtitle_style = ParagraphStyle(
        'DocSubTitle',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=13,
        leading=17,
        textColor=colors.HexColor("#334155"),
        alignment=1,
        spaceAfter=25
    )

    h1_style = ParagraphStyle(
        'Heading1Custom',
        parent=styles['Heading1'],
        fontName='Helvetica-Bold',
        fontSize=15,
        leading=19,
        textColor=colors.HexColor("#1E3A8A"),
        spaceBefore=16,
        spaceAfter=8,
        keepWithNext=True
    )

    h2_style = ParagraphStyle(
        'Heading2Custom',
        parent=styles['Heading2'],
        fontName='Helvetica-Bold',
        fontSize=12,
        leading=16,
        textColor=colors.HexColor("#0F766E"),
        spaceBefore=10,
        spaceAfter=6,
        keepWithNext=True
    )

    body_style = ParagraphStyle(
        'BodyCustom',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=10,
        leading=14,
        textColor=colors.HexColor("#1F2937"),
        spaceAfter=6
    )

    body_bold = ParagraphStyle(
        'BodyBold',
        parent=body_style,
        fontName='Helvetica-Bold'
    )

    callout_style = ParagraphStyle(
        'CalloutText',
        parent=styles['Normal'],
        fontName='Helvetica-Oblique',
        fontSize=9.5,
        leading=13.5,
        textColor=colors.HexColor("#1E293B")
    )

    code_title_style = ParagraphStyle(
        'CodeTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=10,
        leading=13,
        textColor=colors.HexColor("#1E3A8A"),
        spaceBefore=10,
        spaceAfter=4,
        keepWithNext=True
    )

    code_block_style = ParagraphStyle(
        'CodeSnippet',
        parent=styles['Normal'],
        fontName='Courier',
        fontSize=7.2,
        leading=9.2,
        textColor=colors.HexColor("#0F172A")
    )

    elements = []

    # =========================================================================
    # COVER / TITLE PAGE
    # =========================================================================
    elements.append(Spacer(1, 40))
    elements.append(Paragraph("MIVA OPEN UNIVERSITY", ParagraphStyle('UniName', fontName='Helvetica-Bold', fontSize=18, leading=22, alignment=1, textColor=colors.HexColor("#0F172A"))))
    elements.append(Paragraph("FACULTY OF COMPUTING & APPLIED SCIENCES", ParagraphStyle('FacName', fontName='Helvetica', fontSize=12, leading=16, alignment=1, textColor=colors.HexColor("#475569"))))
    elements.append(Spacer(1, 30))

    elements.append(Paragraph("COS 202: COMPUTER PROGRAMMING II", ParagraphStyle('CourseCode', fontName='Helvetica-Bold', fontSize=15, leading=19, alignment=1, textColor=colors.HexColor("#D97706"))))
    elements.append(Paragraph("MID-SEMESTER LAB ASSESSMENT PROJECT", ParagraphStyle('Assessment', fontName='Helvetica-Bold', fontSize=13, leading=17, alignment=1, textColor=colors.HexColor("#1E3A8A"))))
    elements.append(Spacer(1, 20))

    elements.append(Paragraph("SMART CLINIC APPOINTMENT &<br/>PATIENT MANAGEMENT SYSTEM<br/>(SCAPMS)", title_style))
    elements.append(Paragraph("A High-Performance, Multi-Tier C++ Object-Oriented Application Demonstrating Advanced OOP, Interface Design, Data Structures, Custom Searching & Sorting Algorithms, Event-Driven GUI, Exception Handling, and File Persistence", subtitle_style))
    elements.append(Spacer(1, 30))

    # Metadata table
    meta_data = [
        [Paragraph("<b>Course:</b>", body_style), Paragraph("COS 202 — Computer Programming II", body_style)],
        [Paragraph("<b>Case Study:</b>", body_style), Paragraph("Smart Clinic Appointment & Patient Management System (SCAPMS)", body_style)],
        [Paragraph("<b>Programming Language:</b>", body_style), Paragraph("C++ (ISO C++17 Standard)", body_style)],
        [Paragraph("<b>Development Architecture:</b>", body_style), Paragraph("Model-View-Controller (MVC) with Clean Layered Namespaces", body_style)],
        [Paragraph("<b>Target Compiler & Environment:</b>", body_style), Paragraph("Microsoft Visual C++ (MSVC 2026 x64) / Windows GUI Subsystem", body_style)],
        [Paragraph("<b>Submission Date:</b>", body_style), Paragraph("November 2026", body_style)],
        [Paragraph("<b>Assessment Type:</b>", body_style), Paragraph("Mid-Semester Practical Laboratory Examination Project", body_style)]
    ]
    meta_table = Table(meta_data, colWidths=[150, 350])
    meta_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F8FAFC")),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#CBD5E1")),
        ('INNERGRID', (0,0), (-1,-1), 0.5, colors.HexColor("#E2E8F0")),
        ('TOPPADDING', (0,0), (-1,-1), 6),
        ('BOTTOMPADDING', (0,0), (-1,-1), 6),
        ('LEFTPADDING', (0,0), (-1,-1), 12),
        ('RIGHTPADDING', (0,0), (-1,-1), 12),
    ]))
    elements.append(meta_table)
    elements.append(Spacer(1, 40))

    # Executive Checklist Box
    checklist_data = [
        [Paragraph("<b>Mandatory Project Deliverables Included in this Submission:</b>", body_bold)],
        [Paragraph("✓ <b>Section 4: Comprehensive Project Report</b> (2–3 page analysis covering all xii required topics)", body_style)],
        [Paragraph("✓ <b>Section 3: UML Class Diagram & Architecture</b> (Inheritance, Abstract Classes, Polymorphism, Interfaces, Composition)", body_style)],
        [Paragraph("✓ <b>Section 2: GUI Screenshots & Interface Walkthrough</b> (All 6 core panels captured from live running system)", body_style)],
        [Paragraph("✓ <b>Section 1: Complete Labelled Source Code</b> (Header files, Source files, GUI resources, and Data files)", body_style)]
    ]
    chk_table = Table(checklist_data, colWidths=[500])
    chk_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#EFF6FF")),
        ('BOX', (0,0), (-1,-1), 1.5, colors.HexColor("#3B82F6")),
        ('TOPPADDING', (0,0), (-1,-1), 6),
        ('BOTTOMPADDING', (0,0), (-1,-1), 6),
        ('LEFTPADDING', (0,0), (-1,-1), 12),
    ]))
    elements.append(chk_table)

    elements.append(PageBreak())

    # =========================================================================
    # PART 1: 2-3 PAGE PROJECT REPORT (Mandatory Section 4)
    # =========================================================================
    elements.append(Paragraph("PART 1: PROJECT TECHNICAL REPORT", title_style))
    elements.append(Spacer(1, 10))

    # i. Project Description
    elements.append(Paragraph("i. Project Description", h1_style))
    elements.append(Paragraph(
        "The <b>Smart Clinic Appointment & Patient Management System (SCAPMS)</b> is an enterprise-grade clinical desktop software solution designed to streamline healthcare delivery within university health centers. The software provides clinic receptionists, triage nurses, and medical staff with a cohesive graphical user interface (GUI) to manage patient admissions, doctor consultations, emergency priority triage, and administrative records. SCAPMS implements an end-to-end clinical workflow: registering outpatients across distinct medical categories, scheduling and managing doctor appointments, maintaining a First-In-First-Out (FIFO) consultation waiting queue, providing sub-microsecond searching and sorting over clinical databases, facilitating reversible administrative transactions via an undo stack, and generating comprehensive analytical reports.",
        body_style
    ))

    # ii. System Features
    elements.append(Paragraph("ii. System Features", h1_style))
    elements.append(Paragraph(
        "SCAPMS strictly fulfills every mandatory requirement stipulated in the COS 202 project brief:",
        body_style
    ))
    feat_points = [
        "<b>Multi-Category Patient Registration & Lifecycle:</b> Manages General Outpatients, Emergency Triage Patients, and Follow-Up Review Patients with polymorphic fee calculation and clinical attribute tracking.",
        "<b>Comprehensive Appointment Scheduling:</b> Books, reschedules, cancels, and completes consultations linked to qualified appointable patients.",
        "<b>Real-Time Waiting Queue Management (FIFO):</b> Implemented using <code>std::queue</code> to guarantee equitable, first-come first-served patient consultation order.",
        "<b>Reversible Administrative Operations (Undo Stack):</b> Implemented via <code>std::stack</code> utilizing the Command Design Pattern to reverse patient deletions, modifications, and appointment cancellations.",
        "<b>High-Speed Cache for Frequently Accessed Records:</b> A fixed-size C++ array (<code>PatientRecord* recentPatients[10]</code>) providing rapid O(1) recall of active patient pointers.",
        "<b>Student-Implemented Searching Algorithms:</b> Manual Linear Search, Divide-and-Conquer Binary Search, and Recursive Linear Search (zero reliance on standard library searching).",
        "<b>Student-Implemented Sorting Algorithms:</b> Manual Quick Sort, Selection Sort, and Merge Sort allowing multi-field sorting by Patient Name, ID, Age, and Registration Date.",
        "<b>Event-Driven Graphical User Interface:</b> A native Windows Win32 / Common Controls interface featuring responsive layout management, dynamic controls, interactive dialogs, live timers, and keyboard accelerators.",
        "<b>Input Validation & Exception Handling:</b> Comprehensive input bounds checking paired with a custom exception hierarchy.",
        "<b>Data Persistence:</b> Robust file I/O exporting and importing clinic databases to structured CSV and unified text files."
    ]
    for fp in feat_points:
        elements.append(Paragraph(f"• {fp}", body_style))

    # iii. Class Hierarchy & Architectural Design
    elements.append(Paragraph("iii. Class Hierarchy & Architectural Design", h1_style))
    elements.append(Paragraph(
        "SCAPMS is organized following the <b>Model-View-Controller (MVC)</b> architecture with explicit C++ namespaces (<code>scapms::model</code>, <code>scapms::controller</code>, <code>scapms::gui</code>, and <code>scapms::utils</code>):",
        body_style
    ))
    elements.append(Paragraph(
        "• <b>Abstract Base Class (<code>PatientRecord</code>):</b> Encapsulates common patient identity attributes (<code>m_id</code>, <code>m_name</code>, <code>m_age</code>, <code>m_contact</code>, <code>m_registrationDate</code>). It defines pure virtual functions (<code>getPatientType()</code>, <code>getDetails()</code>, <code>calculateConsultationFee()</code>, <code>serialize()</code>, and <code>clone()</code>), preventing direct instantiation while establishing a uniform polymorphic contract.<br/>"
        "• <b>Interface-Like Abstract Class (<code>Appointable</code>):</b> Defines pure virtual scheduling capabilities (<code>scheduleAppointment()</code>, <code>cancelAppointment()</code>, <code>hasActiveAppointment()</code>). This adheres to the <i>Interface Segregation Principle</i>, allowing appointment capabilities to be composed only onto schedulable patient types.<br/>"
        "• <b>Concrete Subclasses:</b><br/>"
        "&nbsp;&nbsp;&nbsp;&nbsp;1. <b><code>GeneralPatient</code>:</b> Multiple inheritance from <code>PatientRecord</code> and <code>Appointable</code>. Tracks general symptoms, routine checkup status, and computes standard consultation fees ($40 baseline, +$15 evaluation).<br/>"
        "&nbsp;&nbsp;&nbsp;&nbsp;2. <b><code>EmergencyPatient</code>:</b> Inherits from <code>PatientRecord</code> only. It deliberately does <i>not</i> implement <code>Appointable</code> because emergency triage cases require immediate acute care and bypass advance appointment slots. Tracks severity levels 1–5 (Resuscitation to Non-Urgent) and emergency contact kin.<br/>"
        "&nbsp;&nbsp;&nbsp;&nbsp;3. <b><code>FollowUpPatient</code>:</b> Multiple inheritance from <code>PatientRecord</code> and <code>Appointable</code>. Tracks previous visit dates, follow-up indications, attending physician, and calculates discounted follow-up consultation fees ($25).<br/>"
        "• <b>Composition & Encapsulation (<code>ClinicDatabase</code>):</b> Owns the collection of registered patients (<code>std::vector&lt;std::shared_ptr&lt;PatientRecord&gt;&gt;</code>) and clinical appointments (<code>std::vector&lt;Appointment&gt;</code>), mediating all internal state transitions.",
        body_style
    ))

    # iv. Data Structures Used
    elements.append(Paragraph("iv. Data Structures Used", h1_style))
    elements.append(Paragraph(
        "Four dedicated C++ data structures are utilized, matching their exact algorithmic strengths:<br/>"
        "1. <b><code>std::vector&lt;std::shared_ptr&lt;PatientRecord&gt;&gt;</code>:</b> Serves as the primary dynamic repository for registered patients. It offers contiguous memory locality, O(1) amortized insertion, and random access indexing crucial for sorting and binary searching.<br/>"
        "2. <b><code>std::queue&lt;std::string&gt;</code>:</b> Implements the consultation waiting queue. Operating on a strict First-In-First-Out (FIFO) basis, patients who arrive first are dequeued first when a consulting physician summons the next patient.<br/>"
        "3. <b><code>std::stack&lt;std::shared_ptr&lt;AdminOperation&gt;&gt;</code>:</b> Powers the Undo Last Administrative Operation feature. Adhering to the Last-In-First-Out (LIFO) discipline, every patient deletion, record modification, or appointment cancellation is pushed as a command object onto the stack, allowing instant reversion.<br/>"
        "4. <b>Fixed-Size C++ Array (<code>PatientRecord* recentPatients[10]</code>):</b> Acts as a fast cache for frequently accessed records. On each patient lookup or update, the patient pointer is shifted to the head (slot 0) using a Least-Recently-Used (LRU) displacement scheme, demonstrating low-level pointer array manipulation.",
        body_style
    ))

    # v. Searching Algorithms Used
    elements.append(Paragraph("v. Searching Algorithms Used", h1_style))
    elements.append(Paragraph(
        "Three distinct searching algorithms were implemented manually from scratch (<code>SearchEngine</code>):<br/>"
        "1. <b>Iterative Linear Search (O(N)):</b> Sequentially inspects each patient record by unique Patient ID. It operates reliably on unsorted data.<br/>"
        "2. <b>Divide-and-Conquer Binary Search (O(log N)):</b> Evaluates sorted arrays by continually halving the search interval. Demonstrates logarithmic efficiency over large clinic registries.<br/>"
        "3. <b>Recursive Linear Search (O(N)):</b> Recursively traverses patient names by breaking down the array inspection into a head-and-tail recursive evaluation.",
        body_style
    ))

    # vi. Sorting Algorithms Used
    elements.append(Paragraph("vi. Sorting Algorithms Used", h1_style))
    elements.append(Paragraph(
        "Three sorting algorithms were manually developed without standard library routines (<code>SortEngine</code>):<br/>"
        "1. <b>Quick Sort (O(N log N) Average):</b> Employs recursive divide-and-conquer partitioning using Lomuto's partitioning scheme. Highly efficient for general clinic reordering.<br/>"
        "2. <b>Selection Sort (O(N^2)):</b> Iteratively locates the minimum element and performs at most O(N) memory swaps, optimal for small lists or memory-constrained hardware.<br/>"
        "3. <b>Merge Sort (O(N log N) Guaranteed):</b> Recursively splits subarrays and merges them, ensuring stability (preserving arrival order of equal keys).<br/>"
        "Records can be sorted dynamically by <b>Patient Name, Patient ID, Age,</b> or <b>Registration Date</b> in Ascending or Descending order.",
        body_style
    ))

    # vii. Recursive Algorithm Used
    elements.append(Paragraph("vii. Recursive Algorithm Used", h1_style))
    elements.append(Paragraph(
        "Recursion is demonstrated across multiple core system components:<br/>"
        "• <b>Recursive Linear Search (<code>recursiveSearchByName</code>):</b> Base Case 1 terminates if index ≥ size (unsuccessful). Base Case 2 returns index if the name matches. Recursive Step calls <code>helper(list, target, index + 1)</code>.<br/>"
        "• <b>Quick Sort Recursive Decomposition:</b> Partitions the vector into sub-ranges around a pivot, then recursively invokes <code>quickSortRecursive</code> on left and right partitions.<br/>"
        "• <b>Merge Sort Recursive Divide-and-Conquer:</b> Recursively bisects the vector into halves until subproblems reach size 1, then invokes <code>merge()</code>.",
        body_style
    ))

    # viii. GUI Design & Layout Management
    elements.append(Paragraph("viii. GUI Design & Layout Management", h1_style))
    elements.append(Paragraph(
        "The user interface is built using native Windows Win32 API and Common Controls with a modern flat aesthetic and Segoe UI typography. Rather than relying on rigid manual positioning, a custom object-oriented <b>Layout Engine</b> (<code>VBoxLayout</code>, <code>HBoxLayout</code>, <code>GridLayout</code>) automatically recalibrates widget boundaries during <code>WM_SIZE</code> events.<br/>"
        "The GUI incorporates 6 organized tabs: <i>Dashboard & Analytics</i> (KPI cards, array cache, alert banner), <i>Patient Records</i> (List view with color-coded tags, CRUD actions), <i>Appointments</i> (scheduling/status), <i>Waiting Queue</i> (active consulting room indicator, FIFO dequeue), <i>Search & Sorting</i> (live benchmarking diagnostics), and <i>Clinical Reports</i> (multi-report generator). Keyboard accelerators (<code>Ctrl+S</code>, <code>Ctrl+F</code>, <code>Ctrl+Q</code>, <code>Ctrl+Z</code>) and periodic background timers enhance clinic staff ergonomics.",
        body_style
    ))

    # ix. Exception Handling Approach
    elements.append(Paragraph("ix. Exception Handling Approach", h1_style))
    elements.append(Paragraph(
        "A modular, robust exception hierarchy rooted at <code>scapms::utils::ClinicException</code> protects the runtime state:<br/>"
        "• <code>DuplicateIDException</code>: Intercepts duplicate patient ID registrations.<br/>"
        "• <code>InvalidInputException</code>: Enforces field bounds (e.g., patient age must be 0–120 years, non-blank strings).<br/>"
        "• <code>PatientNotFoundException</code>: Thrown when querying nonexistent records.<br/>"
        "• <code>AppointmentException</code>: Prevents illegal bookings (e.g., booking advance appointments for emergency triage cases).<br/>"
        "• <code>QueueEmptyException</code>: Warns clinic staff when attempting to dequeue from an empty waiting line.<br/>"
        "• <code>FileIOException</code>: Handles file access and corrupted disk streams.<br/>"
        "Every UI event handler encapsulates operations in <code>try { ... } catch (const ClinicException& e)</code> blocks that display user-friendly modal dialogues rather than crashing.",
        body_style
    ))

    # x. Algorithms Chosen and Reasons
    elements.append(Paragraph("x. Algorithms Chosen and Reasons", h1_style))
    algo_table_data = [
        [Paragraph("<b>Algorithm</b>", body_bold), Paragraph("<b>Time Complexity</b>", body_bold), Paragraph("<b>Space</b>", body_bold), Paragraph("<b>Justification for Clinic Use Case</b>", body_bold)],
        [Paragraph("<b>Quick Sort</b>", body_style), Paragraph("O(N log N) avg<br/>O(N^2) worst", body_style), Paragraph("O(log N)", body_style), Paragraph("Chosen as the primary sort engine due to its exceptional cache locality and minimal overhead on large patient registries.", body_style)],
        [Paragraph("<b>Selection Sort</b>", body_style), Paragraph("O(N^2)", body_style), Paragraph("O(1)", body_style), Paragraph("Optimal for small lists and memory-constrained cache buffers because it makes at most O(N) swaps.", body_style)],
        [Paragraph("<b>Merge Sort</b>", body_style), Paragraph("O(N log N)", body_style), Paragraph("O(N)", body_style), Paragraph("Chosen when stability is mandatory (preserving existing order of patients with identical ages or admission dates).", body_style)],
        [Paragraph("<b>Binary Search</b>", body_style), Paragraph("O(log N)", body_style), Paragraph("O(1)", body_style), Paragraph("Sub-microsecond retrieval when records are presorted by ID or Name; evaluates 1,000 records in ~10 comparisons.", body_style)],
        [Paragraph("<b>Linear Search</b>", body_style), Paragraph("O(N)", body_style), Paragraph("O(1)", body_style), Paragraph("Versatile fallback when searching unsorted patient records or filtering by free-text keywords.", body_style)]
    ]
    at = Table(algo_table_data, colWidths=[90, 85, 55, 270])
    at.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#F1F5F9")),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
        ('TOPPADDING', (0,0), (-1,-1), 5),
        ('BOTTOMPADDING', (0,0), (-1,-1), 5),
    ]))
    elements.append(at)
    elements.append(Spacer(1, 8))

    # xi & xii. Challenges and Resolutions
    elements.append(Paragraph("xi. Challenges Encountered & xii. Resolutions", h1_style))
    elements.append(Paragraph(
        "• <b>Challenge 1: Safe Polymorphic Memory Management vs Raw Cache Pointers:</b> Combining modern <code>std::shared_ptr</code> for patient ownership with a fixed raw pointer array (<code>PatientRecord* recentPatients[10]</code>) posed dangling pointer risks upon deletion.<br/>"
        "&nbsp;&nbsp;<i>Resolution:</i> Implemented <code>invalidateCachedPointer()</code> inside <code>ClinicDatabase::deletePatient()</code> which scans and nullifies any cached pointer before the shared pointer is freed.<br/>"
        "• <b>Challenge 2: Windows Macro Collision with C++ Standard Library:</b> <code>windows.h</code> defines preprocessor macros for <code>min</code> and <code>max</code>, breaking <code>std::min</code> and <code>std::max</code> in layout managers.<br/>"
        "&nbsp;&nbsp;<i>Resolution:</i> Injected <code>#define NOMINMAX</code> across all GUI headers and MSVC build configurations.<br/>"
        "• <b>Challenge 3: Multi-Category Dynamic Field Toggling:</b> Rendering distinct input forms for Emergency, General, and Follow-Up patients inside a single registration dialog.<br/>"
        "&nbsp;&nbsp;<i>Resolution:</i> Implemented dynamic visibility toggling via <code>ShowWindow(hCtrl, SW_SHOW/SW_HIDE)</code> responding dynamically to <code>CBN_SELCHANGE</code> combo events.",
        body_style
    ))

    elements.append(PageBreak())

    # =========================================================================
    # PART 2: UML / CLASS DIAGRAM (Mandatory Section 3)
    # =========================================================================
    elements.append(Paragraph("PART 2: UML / CLASS RELATIONSHIP DIAGRAM", title_style))
    elements.append(Paragraph(
        "The following diagram illustrates the complete architectural hierarchy of SCAPMS, highlighting <b>Inheritance</b> (subclasses deriving from abstract <code>PatientRecord</code>), <b>Interface Realization</b> (<code>GeneralPatient</code> and <code>FollowUpPatient</code> implementing <code>Appointable</code>), <b>Polymorphic Invocations</b>, and <b>Composition</b> (<code>ClinicDatabase</code> owning patients and appointments).",
        body_style
    ))
    elements.append(Spacer(1, 10))

    uml_img_path = r"c:\Users\User\Documents\Antigravity\SCAPMS\screenshots\uml_diagram.png"
    if os.path.exists(uml_img_path):
        elements.append(Image(uml_img_path, width=7.0*inch, height=4.6*inch))
    elements.append(Spacer(1, 10))

    elements.append(Paragraph("<b>Architectural Highlights Demonstrated in the UML Model:</b>", body_bold))
    elements.append(Paragraph(
        "1. <b>Abstract Base Class (<code>PatientRecord</code>):</b> Cannot be instantiated; defines pure virtual functions <code>calculateConsultationFee() = 0</code>, <code>getDetails() = 0</code>, and <code>serialize() = 0</code>.<br/>"
        "2. <b>Interface Separation (<code>Appointable</code>):</b> Implemented by General and Follow-Up patients. Emergency patients intentionally bypass it to reflect real-world clinical triage reality.<br/>"
        "3. <b>Runtime Polymorphism:</b> Processed via common base reference <code>void processPatientRecord(const PatientRecord& record)</code> with dynamic virtual dispatch.<br/>"
        "4. <b>Composition & Containers:</b> <code>ClinicDatabase</code> encapsulates <code>std::vector</code>, <code>std::queue</code> (FIFO waiting queue), <code>std::stack</code> (Undo operations), and fixed-size cache array <code>PatientRecord* recentPatients[10]</code>.<br/>"
        "5. <b>Command Pattern:</b> Concrete undo classes (<code>UndoDeletePatientOperation</code>, <code>UndoAddPatientOperation</code>, <code>UndoModifyPatientOperation</code>, <code>UndoRemoveAppointmentOperation</code>) encapsulate state reversals.",
        body_style
    ))

    elements.append(PageBreak())

    # =========================================================================
    # PART 3: SCREENSHOTS & INTERFACE WALKTHROUGH (Mandatory Section 2)
    # =========================================================================
    elements.append(Paragraph("PART 3: GUI SCREENSHOTS & SYSTEM WALKTHROUGH", title_style))
    elements.append(Paragraph(
        "The following screenshots depict the live graphical user interface of the compiled <b>SCAPMS</b> application executing on Windows 64-bit, showcasing each mandatory functional component.",
        body_style
    ))
    elements.append(Spacer(1, 8))

    shots = [
        ("1_main_dashboard.png", "Figure 1: Main Dashboard & Clinical Overview",
         "Displays top-tier KPI cards (Total Patients, Pending Appointments, Waiting Queue Count, Completed Today, Realized Revenue), real-time live alert banner, quick action buttons, and the 10-slot Fixed-Size Array Cache showing frequently accessed patient records."),

        ("2_patient_registration.png", "Figure 2: Patient Registration & Records Management",
         "Illustrates the patient list view with custom tags ([EMERGENCY - High Priority], [General - Normal], [Follow-Up - Scheduled]), live search filtering, and action buttons (+ Add Patient with dynamic category fields, Edit, Delete, Queue, Undo)."),

        ("3_appointment_management.png", "Figure 3: Appointment Scheduling & Management Interface",
         "Shows active and past clinic consultations with appointment ID, linked patient ID/Name, date, time, specialty department, doctor name, fee, and clinical notes. Supports booking, cancellation, completion, and deletion."),

        ("4_waiting_queue.png", "Figure 4: FIFO Waiting Queue & Active Consultation Indicator",
         "Demonstrates the std::queue First-In-First-Out waiting line. Displays prominent 'CURRENTLY SERVING' banner for Consultation Room 1, queue position badges, triage priority labels, and 'Call Next Patient' button."),

        ("5_search_sort_interface.png", "Figure 5: Student-Implemented Search & Sort Diagnostic Interface",
         "Demonstrates user-selectable algorithms via combo boxes: Linear Search, Binary Search, Recursive Search, Quick Sort, Selection Sort, Merge Sort. The performance banner displays comparisons, swaps, execution time in microseconds, and algorithmic rationale."),

        ("6_clinical_reports.png", "Figure 6: Simple Reports & Analytics Dashboard",
         "Displays pre-built analytical reports: (1) Most Frequently Attended Patients, (2) Patients with Pending Appointments, (3) Patient Category Distribution with percentages, (4) Daily Appointment Summary, and (5) Full Executive Summary with disk export."),

        ("7_patient_dialog.png", "Figure 7: Interactive Patient Registration & Clinical Profile Modal Dialog",
         "Demonstrates Requirement 9.b Dynamic Form Components. Selecting patient category (General Outpatient, Emergency Patient, Follow-Up) dynamically updates form controls to present category-specific fields (Symptoms & Checkup for General; Triage Severity 1–5 and Emergency Contact for Emergency; Previous Visit Date, Doctor, and Reason for Follow-Up). Enforces strict input validation on patient age (0–120), names, and contact details with modal error feedback before committing records to the database and array cache."),

        ("8_appointment_dialog.png", "Figure 8: Interactive Clinical Appointment Booking Modal Dialog",
         "Demonstrates clinical consultation booking with patient selection, date entry, time slot selection, clinical department assignment, and attending doctor allocation. Automatically performs triage capability checks using C++ runtime typing (dynamic_cast<Appointable*>), warning users and restricting emergency patients to immediate queue triage rather than routine advance bookings. Logs all new appointments to the database and administrative Undo stack.")
    ]

    for fname, fig_title, desc in shots:
        img_p = os.path.join(r"c:\Users\User\Documents\Antigravity\SCAPMS\screenshots", fname)
        elements.append(Paragraph(fig_title, h2_style))
        if os.path.exists(img_p):
            if "dialog" in fname:
                elements.append(Image(img_p, width=4.8*inch, height=4.2*inch))
            else:
                elements.append(Image(img_p, width=6.8*inch, height=3.6*inch))
        elements.append(Spacer(1, 4))
        elements.append(Paragraph(f"<b>Operational Walkthrough:</b> {desc}", body_style))
        elements.append(Spacer(1, 10))

    elements.append(PageBreak())

    # =========================================================================
    # PART 4: COMPLETE LABELLED C++ SOURCE CODE (Mandatory Section 1)
    # =========================================================================
    elements.append(Paragraph("PART 4: COMPLETE LABELLED C++ SOURCE CODE", title_style))
    elements.append(Paragraph(
        "Below is the complete, modular C++17 source code for SCAPMS, categorized under standard academic headings as requested by the submission guidelines.",
        body_style
    ))
    elements.append(Spacer(1, 10))

    source_manifest = [
        # i. Header Files
        ("i. HEADER FILES", [
            ("src/model/PatientRecord.h", "Abstract PatientRecord Base Class"),
            ("src/model/Appointable.h", "Appointable Interface Abstract Class"),
            ("src/model/GeneralPatient.h", "GeneralPatient Class Header"),
            ("src/model/EmergencyPatient.h", "EmergencyPatient Class Header"),
            ("src/model/FollowUpPatient.h", "FollowUpPatient Class Header"),
            ("src/model/Appointment.h", "Appointment Class Header"),
            ("src/model/ClinicDatabase.h", "ClinicDatabase & Undo Operations Header"),
            ("src/controller/ClinicManager.h", "ClinicManager Facade Controller Header"),
            ("src/controller/AppointmentController.h", "AppointmentController Header"),
            ("src/controller/SearchEngine.h", "Student-Implemented SearchEngine Header"),
            ("src/controller/SortEngine.h", "Student-Implemented SortEngine Header"),
            ("src/utils/Exceptions.h", "Custom Exception Hierarchy Header"),
            ("src/utils/IDGenerator.h", "Atomic ID Generator Header"),
            ("src/utils/FileHandler.h", "File Persistence Handler Header"),
            ("src/gui/LayoutManager.h", "GUI Layout Management Engine Header"),
            ("src/gui/GuiConstants.h", "GUI Resource and Control Constants Header"),
            ("src/gui/PatientDialog.h", "Dynamic Patient Dialog Header"),
            ("src/gui/AppointmentDialog.h", "Appointment Dialog Header"),
            ("src/gui/MainWindow.h", "Main Application Window Header")
        ]),
        # ii. Source Files
        ("ii. SOURCE (IMPLEMENTATION) FILES", [
            ("src/model/PatientRecord.cpp", "PatientRecord Base Implementation & Polymorphism Function"),
            ("src/model/GeneralPatient.cpp", "GeneralPatient Implementation"),
            ("src/model/EmergencyPatient.cpp", "EmergencyPatient Implementation"),
            ("src/model/FollowUpPatient.cpp", "FollowUpPatient Implementation"),
            ("src/model/Appointment.cpp", "Appointment Implementation"),
            ("src/model/ClinicDatabase.cpp", "ClinicDatabase Data Structures & Undo Commands"),
            ("src/controller/ClinicManager.cpp", "ClinicManager Controller Logic"),
            ("src/controller/AppointmentController.cpp", "AppointmentController & Reporting Logic"),
            ("src/controller/SearchEngine.cpp", "SearchEngine (Linear, Binary, Recursive Algorithms)"),
            ("src/controller/SortEngine.cpp", "SortEngine (Quick Sort, Selection Sort, Merge Sort)"),
            ("src/utils/IDGenerator.cpp", "IDGenerator Static Initialization"),
            ("src/utils/FileHandler.cpp", "FileHandler CSV Import/Export & Seed Data"),
            ("src/main.cpp", "Main Entry Point & Automated Requirement Verification Tests")
        ]),
        # iii. GUI Files
        ("iii. GUI FILES & RESOURCE LOGIC", [
            ("src/gui/LayoutManager.cpp", "LayoutManager Hierarchy (VBoxLayout, HBoxLayout, GridLayout)"),
            ("src/gui/PatientDialog.cpp", "PatientDialog Dynamic Form Component Logic"),
            ("src/gui/AppointmentDialog.cpp", "AppointmentDialog Modal Dialog Logic"),
            ("src/gui/MainWindow.cpp", "MainWindow Event Loop, Tab Controls, Custom Presentation & Timers")
        ]),
        # iv. Data Files & Scripts
        ("iv. DATA FILES & AUTOMATION BUILD SCRIPTS", [
            ("data/patients.csv", "Sample Clinical Patient Database (CSV)"),
            ("data/appointments.csv", "Sample Clinical Appointments Database (CSV)"),
            ("build.bat", "Automated MSVC C++17 Build Batch Script")
        ])
    ]

    root_dir = r"c:\Users\User\Documents\Antigravity\SCAPMS"

    for section_heading, file_list in source_manifest:
        elements.append(Paragraph(section_heading, h1_style))
        elements.append(Spacer(1, 6))

        for rel_path, desc in file_list:
            full_path = os.path.join(root_dir, rel_path.replace('/', os.sep))
            elements.append(Paragraph(f"File: <b>{rel_path}</b> — <i>{desc}</i>", code_title_style))

            if os.path.exists(full_path):
                try:
                    with open(full_path, 'r', encoding='utf-8', errors='replace') as f:
                        code_content = f.read()
                except Exception as e:
                    code_content = f"// Error reading file: {e}"
            else:
                code_content = f"// File not found on disk: {rel_path}"

            # Format code block
            # Truncate if exceptionally huge or wrap cleanly
            code_lines = code_content.split('\n')
            # Chunking code to fit within printable page frames
            chunk_size = 42
            for chunk_start in range(0, len(code_lines), chunk_size):
                chunk = code_lines[chunk_start:chunk_start + chunk_size]
                chunk_str = '\n'.join(chunk)

                code_table = Table([[Preformatted(chunk_str, code_block_style)]], colWidths=[492])
                code_table.setStyle(TableStyle([
                    ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F8FAFC")),
                    ('BOX', (0,0), (-1,-1), 0.75, colors.HexColor("#CBD5E1")),
                    ('TOPPADDING', (0,0), (-1,-1), 3),
                    ('BOTTOMPADDING', (0,0), (-1,-1), 3),
                    ('LEFTPADDING', (0,0), (-1,-1), 6),
                    ('RIGHTPADDING', (0,0), (-1,-1), 6),
                ]))
                elements.append(code_table)
                elements.append(Spacer(1, 3))

            elements.append(Spacer(1, 8))

    print(f"Compiling PDF document to {output_pdf_path}...")
    doc.build(elements, canvasmaker=NumberedCanvas)
    print("PDF build completed successfully!")

if __name__ == "__main__":
    out_pdf = r"c:\Users\User\Documents\Antigravity\SCAPMS\SCAPMS_Midsemester_Assessment_Submission.pdf"
    build_pdf(out_pdf)
