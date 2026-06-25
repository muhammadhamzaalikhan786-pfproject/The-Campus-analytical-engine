#include "course_ops.h"

#include "../M2_student_ops/student_ops.h"

#include <cstdlib>
#include <iomanip>
#include <sstream>

using namespace std;

static const string COURSES_FILE = "courses.txt";
static const string ENROLLMENTS_FILE = "enrollments.txt";
static const string ATTENDANCE_FILE = "attendance_log.txt";

static bool activeEnrollmentStatus(const string& status) {
    return status == "active" || status == "enrolled";
}

static string numberText(int number) {
    stringstream output;
    output << number;
    return output.str();
}

static string nextEnrollmentId(const Table& rows) {
    int largest = 0;

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        if (!rows[i].empty() && rows[i][0].length() > 1
            && rows[i][0][0] == 'E') {
            int value = atoi(rows[i][0].substr(1).c_str());
            if (value > largest) {
                largest = value;
            }
        }
    }

    stringstream result;
    result << "E" << setfill('0') << setw(4) << largest + 1;
    return result.str();
}

Row findCourse(const string& courseCode) {
    return findRow(COURSES_FILE, 0, courseCode);
}

static int courseCredits(const string& courseCode) {
    Row course = findCourse(courseCode);
    if (course.size() > 2) {
        return atoi(course[2].c_str());
    }
    return 0;
}

static int activeEnrollmentCount(const string& courseCode) {
    Table enrollments = readTXT(ENROLLMENTS_FILE);
    int count = 0;

    for (int i = 0; i < static_cast<int>(enrollments.size()); i++) {
        if (enrollments[i].size() > 5
            && enrollments[i][2] == courseCode
            && activeEnrollmentStatus(enrollments[i][5])) {
            count++;
        }
    }
    return count;
}

static void updateStoredEnrollmentCount(const string& courseCode, int count) {
    Table courses = readTXT(COURSES_FILE);
    Row header = readHeader(COURSES_FILE);

    for (int i = 0; i < static_cast<int>(courses.size()); i++) {
        if (courses[i].size() > 5 && courses[i][0] == courseCode) {
            courses[i][5] = numberText(count);
            writeTXT(COURSES_FILE, header, courses);
            return;
        }
    }
}

bool checkPrerequisite(const string& roll, const string& courseCode) {
    Row course = findCourse(courseCode);
    if (course.empty()) {
        return false;
    }

    string prerequisite = "NONE";
    if (course.size() > 6) {
        prerequisite = course[6];
    } else if (course.size() > 4) {
        prerequisite = course[4];
    }

    if (prerequisite.empty() || prerequisite == "NONE") {
        return true;
    }

    Table enrollments = readTXT(ENROLLMENTS_FILE);
    for (int i = 0; i < static_cast<int>(enrollments.size()); i++) {
        if (enrollments[i].size() > 11
            && enrollments[i][1] == roll
            && enrollments[i][2] == prerequisite
            && !enrollments[i][11].empty()
            && enrollments[i][11] != "F") {
            return true;
        }
    }

    return false;
}

int getCreditLoad(const string& roll, int semester) {
    Table enrollments = readTXT(ENROLLMENTS_FILE);
    int total = 0;
    string wantedSemester = numberText(semester);

    for (int i = 0; i < static_cast<int>(enrollments.size()); i++) {
        if (enrollments[i].size() > 5
            && enrollments[i][1] == roll
            && enrollments[i][3] == wantedSemester
            && activeEnrollmentStatus(enrollments[i][5])) {
            total += courseCredits(enrollments[i][2]);
        }
    }

    return total;
}

EnrollResult enrollStudent(const string& roll, const string& courseCode,
                           int semester, const string& enrollmentDate) {
    if (!isStudentActive(roll)) {
        return STUDENT_NOT_ACTIVE;
    }

    Row course = findCourse(courseCode);
    if (course.empty()) {
        return COURSE_NOT_FOUND;
    }

    int capacity = 0;
    if (course.size() > 4) {
        capacity = atoi(course[4].c_str());
    } else if (course.size() > 3) {
        capacity = atoi(course[3].c_str());
    }

    if (activeEnrollmentCount(courseCode) >= capacity) {
        return COURSE_FULL;
    }

    Table enrollments = readTXT(ENROLLMENTS_FILE);
    string wantedSemester = numberText(semester);

    for (int i = 0; i < static_cast<int>(enrollments.size()); i++) {
        if (enrollments[i].size() > 5
            && enrollments[i][1] == roll
            && enrollments[i][2] == courseCode
            && enrollments[i][3] == wantedSemester
            && activeEnrollmentStatus(enrollments[i][5])) {
            return ALREADY_ENROLLED;
        }
    }

    int credits = courseCredits(courseCode);
    if (getCreditLoad(roll, semester) + credits > 21) {
        return CREDIT_LIMIT_EXCEEDED;
    }

    if (!checkPrerequisite(roll, courseCode)) {
        return PREREQUISITE_NOT_PASSED;
    }

    Row newEnrollment;
    newEnrollment.push_back(nextEnrollmentId(enrollments));
    newEnrollment.push_back(roll);
    newEnrollment.push_back(courseCode);
    newEnrollment.push_back(wantedSemester);
    newEnrollment.push_back(enrollmentDate);
    newEnrollment.push_back("enrolled");

    if (!appendTXT(ENROLLMENTS_FILE, newEnrollment)) {
        return ENROLL_FILE_ERROR;
    }

    updateStoredEnrollmentCount(courseCode,
                                activeEnrollmentCount(courseCode));
    return ENROLL_SUCCESS;
}

bool dropCourse(const string& roll, const string& courseCode, int semester) {
    Table attendance = readTXT(ATTENDANCE_FILE);

    for (int i = 0; i < static_cast<int>(attendance.size()); i++) {
        int rollIndex = attendance[i].size() >= 5 ? 1 : 0;
        int courseIndex = attendance[i].size() >= 5 ? 2 : 1;

        if (courseIndex < static_cast<int>(attendance[i].size())
            && attendance[i][rollIndex] == roll
            && attendance[i][courseIndex] == courseCode) {
            return false;
        }
    }

    Table enrollments = readTXT(ENROLLMENTS_FILE);
    Row header = readHeader(ENROLLMENTS_FILE);
    string wantedSemester = numberText(semester);

    for (int i = 0; i < static_cast<int>(enrollments.size()); i++) {
        if (enrollments[i].size() > 5
            && enrollments[i][1] == roll
            && enrollments[i][2] == courseCode
            && enrollments[i][3] == wantedSemester
            && activeEnrollmentStatus(enrollments[i][5])) {
            enrollments[i][5] = "dropped";
            if (!writeTXT(ENROLLMENTS_FILE, header, enrollments)) {
                return false;
            }
            updateStoredEnrollmentCount(courseCode,
                                        activeEnrollmentCount(courseCode));
            return true;
        }
    }

    return false;
}

Table listEnrolledStudents(const string& courseCode, int semester) {
    Table enrollments = readTXT(ENROLLMENTS_FILE);
    Table students;
    string wantedSemester = numberText(semester);

    for (int i = 0; i < static_cast<int>(enrollments.size()); i++) {
        if (enrollments[i].size() > 5
            && enrollments[i][2] == courseCode
            && enrollments[i][3] == wantedSemester
            && activeEnrollmentStatus(enrollments[i][5])) {
            Row student = searchByRoll(enrollments[i][1]);
            if (!student.empty()) {
                students.push_back(student);
            }
        }
    }

    return students;
}

string enrollResultMessage(EnrollResult result) {
    if (result == ENROLL_SUCCESS) return "Enrollment completed.";
    if (result == STUDENT_NOT_ACTIVE) return "Student is missing or inactive.";
    if (result == COURSE_NOT_FOUND) return "Course does not exist.";
    if (result == COURSE_FULL) return "Course has no available seat.";
    if (result == ALREADY_ENROLLED) return "Student is already enrolled.";
    if (result == CREDIT_LIMIT_EXCEEDED) return "The 21 credit-hour limit would be exceeded.";
    if (result == PREREQUISITE_NOT_PASSED) return "Required prerequisite has not been passed.";
    return "Enrollment file could not be updated.";
}
