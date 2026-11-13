import os
from PIL import Image, ImageDraw, ImageFont

def draw_uml_diagram(output_path):
    # High resolution canvas for sharp print quality
    width, height = 1800, 1200
    img = Image.new("RGB", (width, height), (248, 250, 252))
    draw = ImageDraw.Draw(img)

    # Load default or standard font
    try:
        font_title = ImageFont.truetype("arialbd.ttf", 32)
        font_box_title = ImageFont.truetype("arialbd.ttf", 20)
        font_stereo = ImageFont.truetype("ariali.ttf", 16)
        font_body = ImageFont.truetype("arial.ttf", 15)
        font_caption = ImageFont.truetype("arial.ttf", 14)
    except:
        font_title = font_box_title = font_stereo = font_body = font_caption = ImageFont.load_default()

    # Title Banner
    draw.rectangle([40, 30, width - 40, 90], fill=(30, 58, 138))
    draw.text((width // 2, 60), "SCAPMS - Unified Class Diagram & Architecture",
              fill=(255, 255, 255), font=font_title, anchor="mm")

    def draw_class_box(x, y, w, h, title, stereo=None, attrs=None, methods=None, header_color=(224, 231, 255), border_color=(30, 58, 138)):
        # Box background
        draw.rectangle([x, y, x + w, y + h], fill=(255, 255, 255), outline=border_color, width=2)
        # Header banner
        header_h = 42 if stereo else 32
        draw.rectangle([x, y, x + w, y + header_h], fill=header_color, outline=border_color, width=2)

        cur_y = y + (10 if stereo else 16)
        if stereo:
            draw.text((x + w // 2, cur_y), stereo, fill=(71, 85, 105), font=font_stereo, anchor="mm")
            cur_y += 18
        draw.text((x + w // 2, cur_y), title, fill=(15, 23, 42), font=font_box_title, anchor="mm")

        # Attributes section
        div1_y = y + header_h
        cur_y = div1_y + 8
        if attrs:
            for attr in attrs:
                draw.text((x + 12, cur_y), attr, fill=(30, 41, 59), font=font_body)
                cur_y += 20
        else:
            cur_y += 10

        # Divider line
        div2_y = cur_y + 4
        draw.line([x, div2_y, x + w, div2_y], fill=border_color, width=1)
        cur_y = div2_y + 8

        # Methods section
        if methods:
            for meth in methods:
                draw.text((x + 12, cur_y), meth, fill=(30, 41, 59), font=font_body)
                cur_y += 20

    # 1. Interface Appointable (Top Right)
    draw_class_box(
        1050, 130, 450, 190,
        "Appointable", "<<interface>>",
        [],
        [
            "+ scheduleAppointment(id, d, t, r) = 0",
            "+ cancelAppointment(id) : bool = 0",
            "+ hasActiveAppointment() : bool = 0",
            "+ getAppointmentIDs() : vector<string> = 0"
        ],
        header_color=(254, 243, 199), border_color=(180, 83, 9)
    )

    # 2. Abstract Class PatientRecord (Top Center)
    draw_class_box(
        500, 130, 470, 240,
        "PatientRecord", "<<abstract>>",
        [
            "# m_id : string",
            "# m_name : string",
            "# m_age : int",
            "# m_contact : string",
            "# m_registrationDate : string"
        ],
        [
            "+ getPatientType() : string = 0",
            "+ getDetails() : string = 0",
            "+ calculateConsultationFee() : double = 0",
            "+ serialize() : string = 0",
            "+ printSummary(os) : void",
            "+ clone() : unique_ptr<PatientRecord> = 0"
        ],
        header_color=(219, 234, 254), border_color=(29, 78, 216)
    )

    # 3. Subclasses of PatientRecord
    # GeneralPatient
    draw_class_box(
        80, 440, 390, 200,
        "GeneralPatient", None,
        [
            "- m_generalSymptoms : string",
            "- m_hasRoutineCheckup : bool",
            "- m_appointmentIds : vector<string>"
        ],
        [
            "+ getPatientType() : string [\"General\"]",
            "+ getDetails() : string",
            "+ calculateConsultationFee() : double",
            "+ scheduleAppointment(...) : void",
            "+ cancelAppointment(...) : bool"
        ],
        header_color=(220, 252, 231), border_color=(21, 128, 61)
    )

    # EmergencyPatient
    draw_class_box(
        520, 440, 390, 200,
        "EmergencyPatient", None,
        [
            "- m_severityLevel : int (1-5)",
            "- m_emergencyContact : string",
            "- m_triageCategory : string"
        ],
        [
            "+ getPatientType() : string [\"Emergency\"]",
            "+ getDetails() : string",
            "+ calculateConsultationFee() : double",
            "+ setSeverityLevel(level) : void",
            "+ getTriageCategory() : string"
        ],
        header_color=(254, 226, 226), border_color=(185, 28, 28)
    )

    # FollowUpPatient
    draw_class_box(
        960, 440, 420, 200,
        "FollowUpPatient", None,
        [
            "- m_previousVisitDate : string",
            "- m_followUpReason : string",
            "- m_attendingDoctor : string",
            "- m_appointmentIds : vector<string>"
        ],
        [
            "+ getPatientType() : string [\"Follow-Up\"]",
            "+ getDetails() : string",
            "+ calculateConsultationFee() : double",
            "+ scheduleAppointment(...) : void",
            "+ cancelAppointment(...) : bool"
        ],
        header_color=(243, 232, 255), border_color=(126, 34, 206)
    )

    # 4. Appointment Class
    draw_class_box(
        1440, 440, 320, 220,
        "Appointment", None,
        [
            "- m_appointmentId : string",
            "- m_patientId : string",
            "- m_patientName : string",
            "- m_date, m_time : string",
            "- m_department, m_doctor : string",
            "- m_status : AppointmentStatus",
            "- m_fee : double"
        ],
        [
            "+ getStatusString() : string",
            "+ setStatus(s) : void",
            "+ serialize() : string",
            "+ print(os) : void"
        ],
        header_color=(254, 249, 195), border_color=(161, 98, 7)
    )

    # 5. ClinicDatabase (Center Bottom)
    draw_class_box(
        350, 720, 580, 260,
        "ClinicDatabase", None,
        [
            "- m_patients : vector<shared_ptr<PatientRecord>>",
            "- m_waitingQueue : queue<string>  [FIFO]",
            "- m_undoStack : stack<shared_ptr<AdminOperation>>",
            "- m_recentPatients[10] : PatientRecord*  [Fixed Array Cache]",
            "- m_appointments : vector<Appointment>"
        ],
        [
            "+ addPatient(p), updatePatient(...), deletePatient(id)",
            "+ addAppointment(a), cancelAppointment(id), completeAppointment(id)",
            "+ enqueuePatient(id), dequeuePatient() : string",
            "+ pushUndoOperation(op), undoLastOperation() : bool",
            "+ recordPatientAccess(p*), getCachedPatient(idx) : PatientRecord*"
        ],
        header_color=(241, 245, 249), border_color=(51, 65, 85)
    )

    # 6. ClinicManager (Controller Facade)
    draw_class_box(
        1000, 740, 420, 230,
        "ClinicManager", "<<controller>>",
        [
            "- m_database : ClinicDatabase",
            "- m_appointmentController : AppointmentController"
        ],
        [
            "+ registerGeneralPatient(...)",
            "+ registerEmergencyPatient(...)",
            "+ registerFollowUpPatient(...)",
            "+ callNextPatient() : shared_ptr<PatientRecord>",
            "+ search(query, algo, field) : SearchResult",
            "+ sort(algo, field, asc) : SortResult",
            "+ undoLastAction() : bool"
        ],
        header_color=(224, 231, 255), border_color=(30, 58, 138)
    )

    # 7. Utilities & Engines (Bottom Left & Right)
    draw_class_box(
        50, 740, 260, 200,
        "SearchEngine & SortEngine", "<<utility>>",
        [],
        [
            "+ linearSearchByID(...)",
            "+ binarySearchByID(...)",
            "+ recursiveSearchByName(...)",
            "+ quickSort(...)",
            "+ selectionSort(...)",
            "+ mergeSort(...)"
        ],
        header_color=(241, 245, 249), border_color=(71, 85, 105)
    )

    draw_class_box(
        1460, 740, 300, 200,
        "MainWindow & LayoutManager", "<<gui>>",
        [
            "- m_panels[6] : HWND",
            "- m_manager : ClinicManager"
        ],
        [
            "+ VBoxLayout, HBoxLayout",
            "+ GridLayout",
            "+ refreshDashboard()",
            "+ onTabChanged(idx)",
            "+ onTimerTick(id)"
        ],
        header_color=(241, 245, 249), border_color=(71, 85, 105)
    )

    # ------------------ Drawing Relationship Connectors ------------------
    # Inheritance lines from Subclasses to PatientRecord
    # PatientRecord center bottom = (735, 370)
    # GeneralPatient top = (275, 440)
    # EmergencyPatient top = (715, 440)
    # FollowUpPatient top = (1170, 440)
    draw.line([(735, 370), (735, 405)], fill=(30, 58, 138), width=2)
    draw.line([(275, 405), (1170, 405)], fill=(30, 58, 138), width=2)
    draw.line([(275, 405), (275, 440)], fill=(30, 58, 138), width=2)
    draw.line([(715, 405), (715, 440)], fill=(30, 58, 138), width=2)
    draw.line([(1170, 405), (1170, 440)], fill=(30, 58, 138), width=2)

    # Hollow triangle for Inheritance at (735, 370)
    draw.polygon([(735, 370), (725, 385), (745, 385)], fill=(255, 255, 255), outline=(30, 58, 138))

    # Interface realization from GeneralPatient and FollowUpPatient to Appointable
    # Appointable bottom = (1275, 320)
    draw.line([(1275, 320), (1275, 360)], fill=(180, 83, 9), width=2)
    draw.line([(400, 360), (1275, 360)], fill=(180, 83, 9), width=2)
    draw.line([(400, 360), (400, 440)], fill=(180, 83, 9), width=2)
    draw.polygon([(1275, 320), (1265, 335), (1285, 335)], fill=(255, 255, 255), outline=(180, 83, 9))
    draw.text((800, 345), "<<realizes>> Appointable Interface", fill=(180, 83, 9), font=font_caption)

    # Composition diamond from ClinicDatabase to PatientRecord & Appointment
    # ClinicDatabase top = (640, 720)
    draw.polygon([(640, 720), (630, 705), (640, 690), (650, 705)], fill=(30, 58, 138), outline=(30, 58, 138))
    draw.line([(640, 690), (640, 650)], fill=(30, 58, 138), width=2)
    draw.text((655, 680), "1 : N (Composition)", fill=(30, 58, 138), font=font_caption)

    # Composition from ClinicDatabase to Appointment
    draw.line([(930, 780), (1550, 780), (1550, 660)], fill=(161, 98, 7), width=2)
    draw.polygon([(930, 780), (945, 770), (960, 780), (945, 790)], fill=(161, 98, 7), outline=(161, 98, 7))
    draw.text((1200, 760), "1 : N Appointments", fill=(161, 98, 7), font=font_caption)

    # Association between ClinicManager and ClinicDatabase
    draw.line([(1000, 830), (930, 830)], fill=(30, 58, 138), width=2)
    draw.text((945, 810), "manages", fill=(30, 58, 138), font=font_caption)

    # Legend / Key
    draw.rectangle([50, 1020, width - 50, 1160], fill=(255, 255, 255), outline=(148, 163, 184), width=1)
    draw.text((70, 1035), "UML DIAGRAM ARCHITECTURE KEY & COMPLIANCE SUMMARY:", fill=(15, 23, 42), font=font_box_title)
    draw.text((70, 1070), "1. Inheritance & Abstract Classes: GeneralPatient, EmergencyPatient, and FollowUpPatient inherit from abstract PatientRecord.", fill=(51, 65, 85), font=font_body)
    draw.text((70, 1095), "2. Interface Realization: Appointable is implemented by GeneralPatient & FollowUpPatient (Emergency bypasses advance bookings).", fill=(51, 65, 85), font=font_body)
    draw.text((70, 1120), "3. Polymorphism: PatientRecord base pointer/reference dynamic dispatch for calculateConsultationFee() and getDetails().", fill=(51, 65, 85), font=font_body)
    draw.text((950, 1070), "4. Composition: ClinicDatabase encapsulates std::vector, std::queue, std::stack, and array cache[10].", fill=(51, 65, 85), font=font_body)
    draw.text((950, 1095), "5. Controller & MVC Separation: ClinicManager coordinates UI panels, AppointmentController, and Algorithm Engines.", fill=(51, 65, 85), font=font_body)

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    img.save(output_path)
    print(f"UML diagram saved to: {output_path}")

if __name__ == "__main__":
    draw_uml_diagram(r"c:\Users\User\Documents\Antigravity\SCAPMS\screenshots\uml_diagram.png")

