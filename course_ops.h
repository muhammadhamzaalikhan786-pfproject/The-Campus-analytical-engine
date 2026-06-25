#ifndef COURSE_OPS_H
#define COURSE_OPS_H

#include "../M1_filehandler/filehandler.h"

#include <string>

enum EnrollResult {
    ENROLL_SUCCESS,
    STUDENT_NOT_ACTIVE,
    COURSE_NOT_FOUND,
    COURSE_FULL,
    ALREADY_ENROLLED,
    CREDIT_LIMIT_EXCEEDED,
    PREREQUISITE_NOT_PASSED,
    ENROLL_FILE_ERROR
};

EnrollResult enrollStudent(const std::string& roll,
                           const std::string& courseCode,
                           int semester,
                           const std::string& enrollmentDate);
bool dropCourse(const std::string& roll, const std::string& courseCode,
                int semester);
int getCreditLoad(const std::string& roll, int semester);
bool checkPrerequisite(const std::string& roll,
                       const std::string& courseCode);
Table listEnrolledStudents(const std::string& courseCode, int semester);
Row findCourse(const std::string& courseCode);
std::string enrollResultMessage(EnrollResult result);

#endif
