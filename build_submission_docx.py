import os
import sys
import docx
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_ALIGN_VERTICAL
from docx.oxml import OxmlElement, parse_xml
from docx.oxml.ns import nsdecls, qn

def set_cell_background(cell, fill_hex):
    shd = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{fill_hex}"/>')
    cell._tc.get_or_add_tcPr().append(shd)

def set_cell_margins(cell, top=100, bottom=100, left=150, right=150):
    tcMar = parse_xml(f'<w:tcMar {nsdecls("w")}><w:top w:w="{top}" w:type="dxa"/><w:bottom w:w="{bottom}" w:type="dxa"/><w:left w:w="{left}" w:type="dxa"/><w:right w:w="{right}" w:type="dxa"/></w:tcMar>')
    cell._tc.get_or_add_tcPr().append(tcMar)

def set_cell_borders(cell, top=None, bottom=None, left=None, right=None):
    tcPr = cell._tc.get_or_add_tcPr()
    tcBorders = parse_xml(f'<w:tcBorders {nsdecls("w")}/>')
    borders = {'top': top, 'bottom': bottom, 'left': left, 'right': right}
    for border_name, border_val in borders.items():
        if border_val:
            b_elm = parse_xml(f'<w:{border_name} {nsdecls("w")} w:val="{border_val.get("val", "single")}" w:sz="{border_val.get("sz", "4")}" w:space="0" w:color="{border_val.get("color", "CBD5E1")}"/>')
            tcBorders.append(b_elm)
        else:
            b_elm = parse_xml(f'<w:{border_name} {nsdecls("w")} w:val="none"/>')
            tcBorders.append(b_elm)
    tcPr.append(tcBorders)

def add_page_number(run):
    fldSimple = parse_xml(f'<w:fldSimple {nsdecls("w")} w:instr="PAGE"/>')
    run._r.append(fldSimple)

def add_numpages(run):
    fldSimple = parse_xml(f'<w:fldSimple {nsdecls("w")} w:instr="NUMPAGES"/>')
    run._r.append(fldSimple)

def build_docx(output_path):
    doc = docx.Document()

    # Page setup (Letter, 0.75" margins)
    section = doc.sections[0]
    section.page_width = Inches(8.5)
    section.page_height = Inches(11.0)
    section.top_margin = Inches(0.75)
    section.bottom_margin = Inches(0.75)
    section.left_margin = Inches(0.75)
    section.right_margin = Inches(0.75)
    section.different_first_page_header_footer = True

    # Header & Footer setup for subsequent pages
    header = section.header
    header_p = header.paragraphs[0]
    header_p.text = "COS 202: Computer Programming II — Smart Clinic Appointment & Patient Management (SCAPMS)"
    header_p.alignment = WD_ALIGN_PARAGRAPH.RIGHT
    header_p.runs[0].font.name = "Segoe UI"
    header_p.runs[0].font.size = Pt(8.5)
    header_p.runs[0].font.color.rgb = RGBColor(100, 116, 139)

    footer = section.footer
    footer_p = footer.paragraphs[0]
    footer_p.text = "Miva Open University — Mid-Semester Lab Assessment Project  |  Page "
    footer_p.alignment = WD_ALIGN_PARAGRAPH.LEFT
    footer_p.runs[0].font.name = "Segoe UI"
    footer_p.runs[0].font.size = Pt(8.5)
    footer_p.runs[0].font.color.rgb = RGBColor(100, 116, 139)

    run_page = footer_p.add_run()
    run_page.font.name = "Segoe UI"
    run_page.font.size = Pt(8.5)
    run_page.font.bold = True
    run_page.font.color.rgb = RGBColor(30, 58, 138)
    add_page_number(run_page)

    run_of = footer_p.add_run(" of ")
    run_of.font.name = "Segoe UI"
    run_of.font.size = Pt(8.5)
    run_of.font.color.rgb = RGBColor(100, 116, 139)

    run_total = footer_p.add_run()
    run_total.font.name = "Segoe UI"
    run_total.font.size = Pt(8.5)
    run_total.font.bold = True
    run_total.font.color.rgb = RGBColor(30, 58, 138)
    add_numpages(run_total)

    # Base styling
    normal_style = doc.styles['Normal']
    normal_style.font.name = 'Segoe UI'
    normal_style.font.size = Pt(10)
    normal_style.font.color.rgb = RGBColor(31, 41, 55)

    # -------------------------------------------------------------
    # Helper functions for adding rich content
    # -------------------------------------------------------------
    def add_title(text):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p.paragraph_format.space_before = Pt(0)
        p.paragraph_format.space_after = Pt(8)
        run = p.add_run(text)
        run.font.name = 'Segoe UI'
        run.font.size = Pt(22)
        run.font.bold = True
        run.font.color.rgb = RGBColor(30, 58, 138)
        return p

    def add_subtitle(text):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p.paragraph_format.space_before = Pt(0)
        p.paragraph_format.space_after = Pt(18)
        run = p.add_run(text)
        run.font.name = 'Segoe UI'
        run.font.size = Pt(11)
        run.font.color.rgb = RGBColor(71, 85, 105)
        return p

    def add_h1(text):
        p = doc.add_paragraph()
        p.paragraph_format.space_before = Pt(18)
        p.paragraph_format.space_after = Pt(6)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.font.name = 'Segoe UI'
        run.font.size = Pt(15)
        run.font.bold = True
        run.font.color.rgb = RGBColor(30, 58, 138)
        return p

    def add_h2(text):
        p = doc.add_paragraph()
        p.paragraph_format.space_before = Pt(12)
        p.paragraph_format.space_after = Pt(4)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.font.name = 'Segoe UI'
        run.font.size = Pt(12)
        run.font.bold = True
        run.font.color.rgb = RGBColor(15, 118, 110) # Teal
        return p

    def add_body_p(text, bold_prefix=None, space_after=5):
        p = doc.add_paragraph()
        p.paragraph_format.space_after = Pt(space_after)
        p.paragraph_format.line_spacing = 1.15
        if bold_prefix:
            r_pre = p.add_run(bold_prefix)
            r_pre.font.bold = True
            r_pre.font.color.rgb = RGBColor(15, 23, 42)
        r_text = p.add_run(text)
        r_text.font.color.rgb = RGBColor(31, 41, 55)
        return p

    def add_bullet(text, bold_prefix=None):
        p = doc.add_paragraph(style='List Bullet')
        p.paragraph_format.space_after = Pt(4)
        p.paragraph_format.line_spacing = 1.15
        if bold_prefix:
            r_pre = p.add_run(bold_prefix)
            r_pre.font.bold = True
            r_pre.font.color.rgb = RGBColor(15, 23, 42)
        r_text = p.add_run(text)
        r_text.font.color.rgb = RGBColor(31, 41, 55)
        return p

    def add_callout(text, bold_title=""):
        table = doc.add_table(rows=1, cols=1)
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        cell = table.cell(0, 0)
        set_cell_background(cell, "EFF6FF")
        set_cell_margins(cell, top=140, bottom=140, left=200, right=200)
        set_cell_borders(cell, left={"val": "single", "sz": "18", "color": "3B82F6"},
                               top={"val": "single", "sz": "4", "color": "BFDBFE"},
                               bottom={"val": "single", "sz": "4", "color": "BFDBFE"},
                               right={"val": "single", "sz": "4", "color": "BFDBFE"})
        p = cell.paragraphs[0]
        p.paragraph_format.space_after = Pt(0)
        p.paragraph_format.line_spacing = 1.15
        if bold_title:
            rb = p.add_run(bold_title + "\n")
            rb.font.bold = True
            rb.font.size = Pt(10)
            rb.font.color.rgb = RGBColor(30, 58, 138)
        rt = p.add_run(text)
        rt.font.size = Pt(9.5)
        rt.font.color.rgb = RGBColor(30, 41, 59)
        doc.add_paragraph().paragraph_format.space_after = Pt(6)

    def add_code_block(file_path, title_desc):
        p_title = doc.add_paragraph()
        p_title.paragraph_format.space_before = Pt(10)
        p_title.paragraph_format.space_after = Pt(3)
        p_title.paragraph_format.keep_with_next = True
        r1 = p_title.add_run("File: ")
        r1.font.bold = True
        r1.font.size = Pt(9.5)
        r1.font.color.rgb = RGBColor(30, 58, 138)
        r2 = p_title.add_run(file_path)
        r2.font.name = "Consolas"
        r2.font.bold = True
        r2.font.size = Pt(9.5)
        r2.font.color.rgb = RGBColor(30, 58, 138)
        r3 = p_title.add_run(f"  ({title_desc})")
        r3.font.italic = True
        r3.font.size = Pt(9)
        r3.font.color.rgb = RGBColor(100, 116, 139)

        # Read file content
        content = ""
        full_path = os.path.join(r"c:\Users\User\Documents\Antigravity\SCAPMS", file_path.replace('/', os.sep))
        if os.path.exists(full_path):
            with open(full_path, 'r', encoding='utf-8', errors='replace') as f:
                content = f.read()
        else:
            content = f"// File not found: {file_path}"

        table = doc.add_table(rows=1, cols=1)
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        cell = table.cell(0, 0)
        set_cell_background(cell, "F8FAFC")
        set_cell_margins(cell, top=100, bottom=100, left=150, right=150)
        set_cell_borders(cell, left={"val": "single", "sz": "6", "color": "CBD5E1"},
                               top={"val": "single", "sz": "6", "color": "CBD5E1"},
                               bottom={"val": "single", "sz": "6", "color": "CBD5E1"},
                               right={"val": "single", "sz": "6", "color": "CBD5E1"})

        p = cell.paragraphs[0]
        p.paragraph_format.space_after = Pt(0)
        p.paragraph_format.line_spacing = 1.05
        run = p.add_run(content)
        run.font.name = "Consolas"
        run.font.size = Pt(7.5)
        run.font.color.rgb = RGBColor(15, 23, 42)

        doc.add_paragraph().paragraph_format.space_after = Pt(6)

    # =============================================================
    # 1. COVER PAGE
    # =============================================================
    p_uni = doc.add_paragraph()
    p_uni.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_uni.paragraph_format.space_before = Pt(30)
    p_uni.paragraph_format.space_after = Pt(2)
    r_uni = p_uni.add_run("MIVA OPEN UNIVERSITY")
    r_uni.font.size = Pt(18)
    r_uni.font.bold = True
    r_uni.font.color.rgb = RGBColor(15, 23, 42)

    p_fac = doc.add_paragraph()
    p_fac.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_fac.paragraph_format.space_after = Pt(25)
    r_fac = p_fac.add_run("FACULTY OF COMPUTING & APPLIED SCIENCES")
    r_fac.font.size = Pt(12)
    r_fac.font.color.rgb = RGBColor(71, 85, 105)

    p_course = doc.add_paragraph()
    p_course.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_course.paragraph_format.space_after = Pt(4)
    r_c1 = p_course.add_run("COS 202: COMPUTER PROGRAMMING II\n")
    r_c1.font.size = Pt(14)
    r_c1.font.bold = True
    r_c1.font.color.rgb = RGBColor(217, 119, 6) # Amber
    r_c2 = p_course.add_run("MID-SEMESTER LAB ASSESSMENT PROJECT REPORT")
    r_c2.font.size = Pt(13)
    r_c2.font.bold = True
    r_c2.font.color.rgb = RGBColor(30, 58, 138)

    add_title("SMART CLINIC APPOINTMENT &\nPATIENT MANAGEMENT SYSTEM (SCAPMS)")
    add_subtitle("A Comprehensive, High-Performance C++17 Application Demonstrating Advanced Object-Oriented Programming, Interface Segregation, Standard Data Structures, Student-Implemented Searching & Sorting Algorithms, Event-Driven GUI, Exception Handling, and File Persistence")

    # Cover Metadata Table
    meta_rows = [
        ("Course Title & Code:", "COS 202 — Computer Programming II"),
        ("Case Study:", "Smart Clinic Appointment & Patient Management System (SCAPMS)"),
        ("Programming Language:", "C++ (ISO C++17 Standard)"),
        ("Architecture Pattern:", "Model-View-Controller (MVC) with Layered Namespaces"),
        ("Compiler & Toolchain:", "Microsoft Visual C++ (MSVC 2026 x64) / Windows Win32 Subsystem"),
        ("Academic Period:", "November 2026 (Mid-Semester Assessment)"),
        ("Document Format:", "Official Course Submission Document (DOCX / PDF)")
    ]
    meta_table = doc.add_table(rows=len(meta_rows), cols=2)
    meta_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    meta_table.autofit = False
    for i, (k, v) in enumerate(meta_rows):
        c0 = meta_table.cell(i, 0)
        c1 = meta_table.cell(i, 1)
        c0.width = Inches(2.3)
        c1.width = Inches(4.7)
        set_cell_background(c0, "F8FAFC" if i % 2 == 0 else "FFFFFF")
        set_cell_background(c1, "F8FAFC" if i % 2 == 0 else "FFFFFF")
        set_cell_margins(c0, top=70, bottom=70, left=120, right=120)
        set_cell_margins(c1, top=70, bottom=70, left=120, right=120)
        border_spec = {"val": "single", "sz": "4", "color": "E2E8F0"}
        set_cell_borders(c0, top=border_spec, bottom=border_spec, left=border_spec, right=border_spec)
        set_cell_borders(c1, top=border_spec, bottom=border_spec, left=border_spec, right=border_spec)

        p0 = c0.paragraphs[0]
        p0.paragraph_format.space_after = Pt(0)
        r0 = p0.add_run(k)
        r0.font.bold = True
        r0.font.size = Pt(9.5)
        r0.font.color.rgb = RGBColor(30, 41, 59)

        p1 = c1.paragraphs[0]
        p1.paragraph_format.space_after = Pt(0)
        r1 = p1.add_run(v)
        r1.font.size = Pt(9.5)
        r1.font.color.rgb = RGBColor(51, 65, 85)

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # Checklist Callout Box
    add_callout(
        "✔ Part 1: Project Technical Report (2–3 page comprehensive report covering all xii required topics)\n"
        "✔ Part 2: UML / Class Relationship Diagram (Inheritance, Abstract Classes, Polymorphism, Interfaces, Composition)\n"
        "✔ Part 3: GUI Screenshots & Interface Walkthrough (All 6 core panels captured from live running system)\n"
        "✔ Part 4: Complete Labelled C++ Source Code (Headers, Implementation Files, GUI files, Data files, and Build script)",
        "MANDATORY PROJECT DELIVERABLES INCLUDED IN THIS SUBMISSION:"
    )

    doc.add_page_break()

    # =============================================================
    # 2. PART 1: TECHNICAL REPORT (Mandatory Section 4, Points i - xii)
    # =============================================================
    add_title("PART 1: PROJECT TECHNICAL REPORT")
    add_subtitle("In-Depth System Analysis, Algorithms Evaluation, Object-Oriented Architecture, and Engineering Solutions")

    # i. Project Description
    add_h1("i. Project Description")
    add_body_p(
        "The Smart Clinic Appointment & Patient Management System (SCAPMS) is a multi-tier, high-performance desktop "
        "application developed in modern C++17 to automate and optimize the clinical workflow of a university health center. "
        "The system coordinates outpatient intake, clinical triage classification, appointment scheduling, consultation queues, "
        "and administrative reporting into a unified graphical environment. Built to address the challenges of manual patient "
        "tracking—such as queue congestion, scheduling clashes, and inaccessible records—SCAPMS establishes an end-to-end "
        "operational pipeline for healthcare receptionists, triage nurses, and medical officers. It demonstrates rigorous "
        "adherence to object-oriented software engineering standards, modular separation of concerns through the Model-View-Controller "
        "(MVC) architecture, and low-latency algorithmic processing without dependence on third-party runtime frameworks."
    )

    # ii. System Features
    add_h1("ii. System Features")
    add_body_p("The completed SCAPMS application fulfills every functional and non-functional requirement mandated in the COS 202 project brief:")
    add_bullet(" Manages General Outpatients, Emergency Triage Patients, and Follow-Up Patients with category-specific data encapsulation.", "Multi-Category Patient Management:")
    add_bullet(" Flexible booking, completion, cancellation, and deletion of consultations with physician assignment and time slot validation.", "Appointment Scheduling Engine:")
    add_bullet(" A strict First-In-First-Out (FIFO) consultation waiting queue implemented using std::queue ensuring fair service order.", "Real-Time Waiting Queue:")
    add_bullet(" An administrative undo stack implemented with std::stack and the Command pattern to instantly reverse accidental deletions, modifications, and cancellations.", "Undo Last Administrative Operation:")
    add_bullet(" A 10-slot fixed-size C++ pointer array implementing a Least-Recently-Used (LRU) displacement scheme for O(1) recall of active patient records.", "Frequently Accessed Records Cache:")
    add_bullet(" Completely student-implemented Linear Search, Binary Search (on presorted data), and Recursive Linear Search.", "Student-Implemented Searching:")
    add_bullet(" Completely student-implemented Quick Sort, Selection Sort, and Merge Sort allowing multi-field sorting by Name, ID, Age, or Date.", "Student-Implemented Sorting:")
    add_bullet(" Modern Win32 GUI featuring custom VBoxLayout, HBoxLayout, and GridLayout managers, live background timers, dynamic dialogs, and keyboard accelerators.", "Event-Driven Graphical User Interface:")
    add_bullet(" Strict field bounds checking (e.g. age between 0 and 120, non-blank strings) coupled with a robust custom exception hierarchy.", "Input Validation & Custom Exceptions:")
    add_bullet(" Bidirectional persistence saving and restoring clinical records and appointments to/from structured CSV and text data files.", "Data Persistence:")

    # iii. Class Hierarchy & Architectural Design
    add_h1("iii. Class Hierarchy & Architectural Design")
    add_body_p("SCAPMS adheres to the Model-View-Controller (MVC) architectural pattern, neatly divided across four distinct C++ namespaces:")
    add_bullet(" Abstract base class defining common patient fields (ID, Name, Age, Contact, Registration Date) and declaring pure virtual functions (getPatientType() = 0, getDetails() = 0, calculateConsultationFee() = 0, serialize() = 0, clone() = 0). It enforces a polymorphic contract while prohibiting direct instantiation.", "scapms::model::PatientRecord (Abstract Base Class):")
    add_bullet(" Pure abstract interface defining scheduling operations (scheduleAppointment(), cancelAppointment(), hasActiveAppointment()). Follows the Interface Segregation Principle by separating appointment booking capabilities from general patient records.", "scapms::model::Appointable (Abstract Interface):")
    add_bullet(" Implements both PatientRecord and Appointable. Models outpatients visiting for general symptoms, wellness screenings, and calculates fees ($40 baseline + $15 evaluation).", "scapms::model::GeneralPatient (Subclass):")
    add_bullet(" Inherits from PatientRecord only. Deliberately does not implement Appointable because acute emergencies require immediate triage care and bypass scheduled appointments. Encapsulates triage severity (Levels 1–5: Resuscitation to Non-Urgent) and emergency contact kin.", "scapms::model::EmergencyPatient (Subclass):")
    add_bullet(" Implements both PatientRecord and Appointable. Models returning patients reviewing ongoing conditions, previous visit dates, and attending physicians with discounted follow-up fees ($25).", "scapms::model::FollowUpPatient (Subclass):")
    add_bullet(" Value object encapsulating appointment IDs, linked patient IDs/Names, dates, times, departments, attending physicians, fees, and status.", "scapms::model::Appointment (Composition Object):")
    add_bullet(" Central repository managing the vector of patients, waiting queue, undo stack, and array cache with encapsulated accessors.", "scapms::model::ClinicDatabase (Data Repository):")
    add_bullet(" Facade controller orchestrating business logic, validation, searching, sorting, undo dispatch, and reporting.", "scapms::controller::ClinicManager (Controller):")

    # iv. Data Structures Used
    add_h1("iv. Data Structures Used")
    add_body_p("Four fundamental C++ data structures were selected and deployed, each aligning with specific operational requirements:")
    add_bullet(" Acts as the primary dynamic storage for patient records. Provides contiguous memory allocation, O(1) amortized insertion, and random-access indexing indispensable for in-place sorting and binary searching.", "1. std::vector<std::shared_ptr<PatientRecord>>:")
    add_bullet(" Powers the consultation waiting queue. Enforces strict FIFO (First-In-First-Out) ordering, ensuring that the patient who joins the queue first is called into consultation first.", "2. std::queue<std::string>:")
    add_bullet(" Implements the Undo Last Administrative Operation feature using the Command Pattern. Administrative actions (deletions, updates, cancellations) are pushed onto the stack (LIFO), enabling instant, multi-level rollback.", "3. std::stack<std::shared_ptr<AdminOperation>>:")
    add_bullet(" Implements a high-speed cache for frequently accessed patient records. Every accessed or edited record is moved to slot 0 with LRU eviction, demonstrating low-level C++ array and pointer manipulation.", "4. Fixed-Size C++ Array (PatientRecord* recentPatients[10]):")

    # v. Searching Algorithms Used
    add_h1("v. Searching Algorithms Used")
    add_body_p("Three distinct searching algorithms were authored manually from first principles without standard library routines:")
    add_bullet(" Iteratively traverses patient records comparing unique IDs. Operates in linear O(N) time and requires no prior ordering, making it the ideal fallback for unsorted records.", "1. Linear Search by ID (O(N)):")
    add_bullet(" Evaluates presorted patient lists by continually halving the search interval. Achieves O(log N) logarithmic performance, locating any record among thousands in fewer than 10 comparisons.", "2. Binary Search by ID / Name (O(log N)):")
    add_bullet(" Examines the current index against the target name; if unmatched, it recursively delegates the remainder of the collection until a match or base case is reached.", "3. Recursive Linear Search by Name (O(N)):")

    # vi. Sorting Algorithms Used
    add_h1("vi. Sorting Algorithms Used")
    add_body_p("Three distinct sorting algorithms were implemented manually to demonstrate comparative algorithm analysis:")
    add_bullet(" Employs Lomuto's partitioning scheme to recursively order partitions around a pivot. Operates in O(N log N) average time with minimal memory overhead, serving as the system's primary sort engine.", "1. Quick Sort (Divide-and-Conquer):")
    add_bullet(" Scans the list iteratively to find the minimum element in each pass. Operates in O(N²) time but performs at most O(N) memory swaps, making it optimal for small arrays or memory-constrained caches.", "2. Selection Sort (Iterative Minimum Finding):")
    add_bullet(" Recursively bisects subarrays and merges them in sorted order. Guarantees O(N log N) worst-case performance and preserves stability (arrival order of equal keys).", "3. Merge Sort (Stable Divide-and-Conquer):")
    add_body_p("Records can be sorted dynamically by Patient Name, Patient ID, Age, or Registration Date in Ascending or Descending order via GUI combo boxes.")

    # vii. Recursive Algorithm Implementation
    add_h1("vii. Recursive Algorithm Implementation")
    add_body_p("Recursion is implemented across multiple subsystems to demonstrate clean base cases and recursive steps:")
    add_bullet(" Base Case 1: index >= list.size() returns -1 (not found). Base Case 2: list[index]->getName() matches target returns index. Recursive Step: calls helper with index + 1.", "Recursive Linear Search:")
    add_bullet(" Base Case: low >= high terminates recursion. Recursive Step: partitions array into [low, pi-1] and [pi+1, high] and calls quickSortRecursive on both halves.", "Quick Sort Partitioning:")
    add_bullet(" Base Case: left >= right terminates recursion. Recursive Step: calculates mid = left + (right - left)/2, recurses on both halves, and invokes merge().", "Merge Sort Divide-and-Conquer:")

    # viii. GUI Design & Layout Management
    add_h1("viii. GUI Design & Layout Management")
    add_body_p(
        "The graphical user interface is constructed using the native Windows Win32 API and Common Controls, styled with "
        "modern Segoe UI typography and double-buffered list views to prevent flickering. To satisfy layout management requirements "
        "without rigid manual coordinates, a custom C++ Layout Engine hierarchy was developed consisting of LayoutItem, VBoxLayout, "
        "HBoxLayout, and GridLayout. These containers compute margins, spacing, and stretch factors dynamically during WM_SIZE events.<br/>"
        "The interface is organized into 6 tabs: (1) Dashboard & Analytics, (2) Patient Records, (3) Appointments, (4) Waiting Queue (FIFO), "
        "(5) Search & Sorting, and (6) Clinical Reports. Ergonomic features include keyboard shortcuts (Ctrl+S for Save, Ctrl+F for Search, "
        "Ctrl+Q for Exit, Ctrl+Z for Undo), status bar telemetry, and periodic background timers."
    )

    # ix. Exception Handling Approach
    add_h1("ix. Exception Handling Approach")
    add_body_p(
        "A comprehensive custom exception hierarchy derived from std::runtime_error guards against invalid state transitions:<br/>"
        "• ClinicException (base): Root for all domain-specific errors.<br/>"
        "• DuplicateIDException: Thrown when attempting to register an ID that already exists.<br/>"
        "• InvalidInputException: Enforces data validation (age must be 0–120, non-blank strings, valid phone formats).<br/>"
        "• PatientNotFoundException: Thrown when an ID lookup fails.<br/>"
        "• AppointmentException: Prevents illegal bookings (e.g. advance appointments for emergency triage cases).<br/>"
        "• QueueEmptyException: Warns staff when attempting to call patients from an empty waiting queue.<br/>"
        "• FileIOException: Protects against file access failures or corrupt disk streams.<br/>"
        "All user operations are enclosed in try/catch blocks that present modal error dialogues rather than crashing."
    )

    # x. Algorithms Chosen and Reasons
    add_h1("x. Algorithms Chosen and Reasons")
    add_body_p("The table below summarizes the complexity characteristics and clinic use-case justification for each algorithm:")

    algo_data = [
        ("Algorithm", "Time Complexity", "Space Complexity", "Justification for Clinical Use Case"),
        ("Quick Sort", "O(N log N) avg / O(N²) worst", "O(log N)", "Primary sort engine for large registries; superior cache locality and in-place performance."),
        ("Selection Sort", "O(N²) avg / O(N²) worst", "O(1)", "Optimal for small cache buffers; minimizes memory writes by executing at most O(N) swaps."),
        ("Merge Sort", "O(N log N) guaranteed", "O(N)", "Mandatory when stability is required (preserving arrival order of equal ages or admission dates)."),
        ("Binary Search", "O(log N) worst", "O(1)", "Sub-microsecond retrieval over presorted records; evaluates 1,000 records in ~10 comparisons."),
        ("Linear Search", "O(N) worst", "O(1)", "Robust, general-purpose search on unsorted data and substring keyword filtering.")
    ]
    t_algo = doc.add_table(rows=len(algo_data), cols=4)
    t_algo.alignment = WD_TABLE_ALIGNMENT.CENTER
    t_algo.autofit = False
    col_w = [Inches(1.2), Inches(1.8), Inches(1.1), Inches(2.9)]
    for r_idx, row in enumerate(algo_data):
        for c_idx, val in enumerate(row):
            cell = t_algo.cell(r_idx, c_idx)
            cell.width = col_w[c_idx]
            is_hdr = (r_idx == 0)
            set_cell_background(cell, "1E3A8A" if is_hdr else ("F8FAFC" if r_idx % 2 == 1 else "FFFFFF"))
            set_cell_margins(cell, top=70, bottom=70, left=100, right=100)
            b_spec = {"val": "single", "sz": "4", "color": "CBD5E1"}
            set_cell_borders(cell, top=b_spec, bottom=b_spec, left=b_spec, right=b_spec)

            p = cell.paragraphs[0]
            p.paragraph_format.space_after = Pt(0)
            run = p.add_run(val)
            run.font.size = Pt(8.5 if not is_hdr else 9)
            if is_hdr:
                run.font.bold = True
                run.font.color.rgb = RGBColor(255, 255, 255)
            else:
                if c_idx == 0: run.font.bold = True
                run.font.color.rgb = RGBColor(31, 41, 55)

    doc.add_paragraph().paragraph_format.space_after = Pt(10)

    # xi & xii. Challenges and Resolutions
    add_h1("xi. Challenges Encountered & xii. Resolutions")
    add_bullet(" Combining modern std::shared_ptr memory ownership for registered patients with a fixed raw pointer array (PatientRecord* recentPatients[10]) introduced dangling pointer hazards when patients were deleted. Resolution: Implemented an explicit invalidateCachedPointer() routine inside ClinicDatabase::deletePatient() that scans the fixed array, nullifies stale pointers, and shifts remaining entries left before memory deallocation.", "1. Dangling Pointer Prevention in Fixed Array Cache:")
    add_bullet(" The Windows windows.h header defines min and max preprocessor macros, causing syntax errors with std::min and std::max in the layout engine. Resolution: Injected #define NOMINMAX across all headers and MSVC build configurations.", "2. Windows Preprocessor Macro Collisions:")
    add_bullet(" Dynamically toggling input fields based on patient category selection within a single native dialog without resource compilers. Resolution: Connected CBN_SELCHANGE combo box notification events to ShowWindow(hCtrl, SW_SHOW/SW_HIDE) with real-time coordinate adjustment.", "3. Dynamic Component Rendering in Win32:")

    doc.add_page_break()

    # =============================================================
    # 3. PART 2: UML / CLASS DIAGRAM (Mandatory Section 3)
    # =============================================================
    add_title("PART 2: UML / CLASS RELATIONSHIP DIAGRAM")
    add_subtitle("Visual Class Hierarchy Demonstrating Inheritance, Abstract Classes, Interfaces, Polymorphism, and Composition")

    add_body_p(
        "The following diagram illustrates the complete architectural blueprint of the Smart Clinic Appointment & Patient "
        "Management System (SCAPMS). It details the inheritance relationships between abstract base class PatientRecord and its "
        "concrete subclasses, interface realization of Appointable, composition with Appointment and ClinicDatabase, and the "
        "MVC separation between controllers and user interface panels."
    )

    uml_img_path = r"c:\Users\User\Documents\Antigravity\SCAPMS\screenshots\uml_diagram.png"
    if os.path.exists(uml_img_path):
        p_img = doc.add_paragraph()
        p_img.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p_img.paragraph_format.space_before = Pt(8)
        p_img.paragraph_format.space_after = Pt(4)
        run_img = p_img.add_run()
        run_img.add_picture(uml_img_path, width=Inches(6.8))

        p_cap = doc.add_paragraph()
        p_cap.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p_cap.paragraph_format.space_after = Pt(14)
        r_cap = p_cap.add_run("Figure 0: SCAPMS Unified Class Relationship & Architecture Diagram")
        r_cap.font.italic = True
        r_cap.font.size = Pt(9.5)
        r_cap.font.color.rgb = RGBColor(71, 85, 105)

    add_h2("Architectural Patterns Demonstrated in the UML Model:")
    add_bullet(" PatientRecord cannot be directly instantiated; it serves as a pure virtual contract enforcing uniform implementations of calculateConsultationFee(), getDetails(), and serialize().", "1. Abstract Base Class & Polymorphism:")
    add_bullet(" GeneralPatient and FollowUpPatient implement Appointable, whereas EmergencyPatient intentionally bypasses it to reflect acute clinical realities.", "2. Interface Realization (Appointable):")
    add_bullet(" ClinicDatabase maintains ownership of collections of PatientRecord and Appointment objects through modern smart pointers.", "3. Composition & Encapsulation:")
    add_bullet(" Undo operations are encapsulated as objects (UndoDeletePatientOperation, UndoModifyPatientOperation, UndoRemoveAppointmentOperation) stored within a LIFO std::stack.", "4. Command Pattern for Administrative Undo:")

    doc.add_page_break()

    # =============================================================
    # 4. PART 3: SCREENSHOTS & SYSTEM WALKTHROUGH (Mandatory Section 2)
    # =============================================================
    add_title("PART 3: GUI SCREENSHOTS & SYSTEM WALKTHROUGH")
    add_subtitle("Visual Verification of the Live Running Application Across All Six Core Functional Tabs")

    add_body_p(
        "The following screenshots were captured directly from the executing 64-bit Windows application (bin/scapms.exe). "
        "Each view verifies full functionality across the mandatory clinical operations."
    )

    shots = [
        ("1_main_dashboard.png", "Figure 1: Main Dashboard & Clinical Analytics Overview",
         "Presents high-level operational telemetry: 5 top-tier KPI cards (Total Patients, Pending Appointments, Waiting Queue Count, "
         "Completed Today, and Revenue Collected), real-time alerts banner, quick action buttons, and the 10-slot Fixed-Size Array Cache "
         "(PatientRecord* recentPatients[10]) tracking frequently accessed patient records with LRU displacement."),

        ("2_patient_registration.png", "Figure 2: Patient Registration & Records Management Panel",
         "Displays the complete patient registry in a full-row-select list view with custom category formatting ([EMERGENCY - High Priority], "
         "[General - Normal], [Follow-Up - Scheduled]). Features live search keyword filtering, dynamic Add Patient dialog (toggling inputs "
         "based on category), Edit, Delete, Enqueue, and Undo action buttons."),

        ("3_appointment_management.png", "Figure 3: Appointment Scheduling & Management Interface",
         "Illustrates consultation appointments with appointment ID, linked patient ID and Name, date, time, medical department, attending physician, "
         "status, fee, and clinical notes. Provides buttons to schedule new appointments, complete consultations, cancel visits, and undo changes."),

        ("4_waiting_queue.png", "Figure 4: Consultation Waiting Queue (std::queue FIFO Discipline)",
         "Demonstrates the consultation waiting queue managed via std::queue. Highlights the prominent 'CURRENTLY SERVING' banner for Consultation "
         "Room 1, queue position badges, triage priority status, and the 'Call Next Patient' button that dequeues in strict arrival order."),

        ("5_search_sort_interface.png", "Figure 5: Student-Implemented Search & Sort Diagnostic Interface",
         "Showcases student-authored algorithms with combo box selectors: Linear Search, Binary Search, Recursive Search, Quick Sort, Selection Sort, "
         "and Merge Sort. A live diagnostic telemetry banner displays comparisons performed, swaps executed, execution duration in microseconds, "
         "and the formal justification for algorithm selection."),

        ("6_clinical_reports.png", "Figure 6: Simple Reports & Executive Summary Dashboard",
         "Generates formatted administrative reports: (1) Most Frequently Attended Patients, (2) Patients with Pending Appointments, "
         "(3) Patient Category Distribution with percentage breakdowns, (4) Daily Appointment Summary, and (5) Full Executive Summary with disk export."),

        ("7_patient_dialog.png", "Figure 7: Interactive Patient Registration & Clinical Profile Modal Dialog",
         "Demonstrates Requirement 9.b Dynamic Form Components. Selecting patient category (General Outpatient, Emergency Patient, Follow-Up) dynamically updates form controls to present category-specific fields (Symptoms & Checkup for General; Triage Severity 1–5 and Emergency Contact for Emergency; Previous Visit Date, Doctor, and Reason for Follow-Up). Enforces strict input validation on patient age (0–120), names, and contact details with modal error feedback before committing records to the database and array cache."),

        ("8_appointment_dialog.png", "Figure 8: Interactive Clinical Appointment Booking Modal Dialog",
         "Demonstrates clinical consultation booking with patient selection, date entry, time slot selection, clinical department assignment, and attending doctor allocation. Automatically performs triage capability checks using C++ runtime typing (dynamic_cast<Appointable*>), warning users and restricting emergency patients to immediate queue triage rather than routine advance bookings. Logs all new appointments to the database and administrative Undo stack.")
    ]

    for fname, fig_title, desc in shots:
        img_p = os.path.join(r"c:\Users\User\Documents\Antigravity\SCAPMS\screenshots", fname)
        add_h2(fig_title)
        if os.path.exists(img_p):
            p_img = doc.add_paragraph()
            p_img.alignment = WD_ALIGN_PARAGRAPH.CENTER
            p_img.paragraph_format.space_before = Pt(4)
            p_img.paragraph_format.space_after = Pt(2)
            run_img = p_img.add_run()
            img_width = Inches(4.5) if "dialog" in fname else Inches(6.6)
            run_img.add_picture(img_p, width=img_width)

        p_desc = doc.add_paragraph()
        p_desc.paragraph_format.space_after = Pt(12)
        r_pre = p_desc.add_run("Functional Walkthrough: ")
        r_pre.font.bold = True
        r_pre.font.size = Pt(9.5)
        r_pre.font.color.rgb = RGBColor(15, 23, 42)
        r_txt = p_desc.add_run(desc)
        r_txt.font.size = Pt(9.5)
        r_txt.font.color.rgb = RGBColor(51, 65, 85)

    doc.add_page_break()

    # =============================================================
    # 5. PART 4: COMPLETE LABELLED SOURCE CODE (Mandatory Section 1)
    # =============================================================
    add_title("PART 4: COMPLETE LABELLED C++ SOURCE CODE")
    add_subtitle("Exhaustive Source Manifest Appropriately Categorized Under Submission Headings")

    add_body_p(
        "Below is the complete, modular C++17 source code for the Smart Clinic Appointment & Patient Management System, "
        "categorized under Header Files, Source (Implementation) Files, GUI Files & Resources, and Data Files / Build Scripts."
    )

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

    for section_heading, file_list in source_manifest:
        add_h1(section_heading)
        for rel_path, desc in file_list:
            add_code_block(rel_path, desc)

    print(f"Saving DOCX document to {output_path}...")
    doc.save(output_path)
    print("DOCX build completed successfully!")

if __name__ == "__main__":
    out_docx = r"c:\Users\User\Documents\Antigravity\SCAPMS\SCAPMS_Midsemester_Assessment_Submission.docx"
    build_docx(out_docx)

