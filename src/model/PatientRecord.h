#ifndef SCAPMS_MODEL_PATIENTRECORD_H
#define SCAPMS_MODEL_PATIENTRECORD_H

#include <string>
#include <iostream>
#include <memory>
#include <vector>

namespace scapms {
namespace model {

/**
 * @brief Abstract Base Class representing a general clinic patient record.
 */
class PatientRecord {
protected:
    std::string m_id;
    std::string m_name;
    int m_age;
    std::string m_contact;
    std::string m_registrationDate;

public:
    PatientRecord(std::string id, std::string name, int age,
                  std::string contact, std::string registrationDate = "");

    virtual ~PatientRecord() = default;

    // Getters
    const std::string& getId() const { return m_id; }
    const std::string& getName() const { return m_name; }
    int getAge() const { return m_age; }
    const std::string& getContact() const { return m_contact; }
    const std::string& getRegistrationDate() const { return m_registrationDate; }

    // Setters
    void setName(const std::string& name) { m_name = name; }
    void setAge(int age) { m_age = age; }
    void setContact(const std::string& contact) { m_contact = contact; }
    void setRegistrationDate(const std::string& date) { m_registrationDate = date; }

    // Pure Virtual Functions (Mandatory Requirement 1.a)
    virtual std::string getPatientType() const = 0;
    virtual std::string getDetails() const = 0;
    virtual double calculateConsultationFee() const = 0;
    virtual std::string serialize() const = 0;
    virtual std::unique_ptr<PatientRecord> clone() const = 0;

    // Virtual display method demonstrating runtime polymorphism
    virtual void printSummary(std::ostream& os = std::cout) const;

    // Operator overloads for sorting/comparisons
    bool operator<(const PatientRecord& other) const {
        return this->m_id < other.m_id;
    }
};

/**
 * @brief Standalone polymorphic function demonstrating runtime polymorphism
 *        through a common base-class reference (Mandatory Requirement 1.d).
 */
void processPatientRecord(const PatientRecord& record, std::ostream& os = std::cout);

} // namespace model
} // namespace scapms

#endif // SCAPMS_MODEL_PATIENTRECORD_H
