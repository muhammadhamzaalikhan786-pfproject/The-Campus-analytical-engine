#include "attendance.h"

#include "../M3_course_ops/course_ops.h"
#include "../M2_student_ops/student_ops.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

static const string ATTENDANCE_FILE = "attendance_log.txt";
static Table attendanceBackup;
static Row attendanceHeaderBackup;
static bool backupAvailable = false;

static string nextLogId(const Table& rows) {
    int largest = 0;

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        if (!rows[i].empty() && rows[i][0].length() > 1
            && rows[i][0][0] == 'L') {
            int value = atoi(rows[i][0].substr(1).c_str());
            if (value > largest) {
                largest = value;
            }
        }
    }

    stringstream result;
    result << "L" << setfill('0') << setw(5) << largest + 1;
    return result.str();
}

static bool isAttendanceStatus(const string& status) {
    return status == "P" || status == "A" || status == "L";
}

bool markAttendance(const string& courseCode, int semester,
                    const string& date) {
    Table students = listEnrolledStudents(courseCode, semester);
    if (students.empty()) {
        cout << "No enrolled students were found for this course.\n";
        return false;
    }

    attendanceBackup = readTXT(ATTENDANCE_FILE);
    attendanceHeaderBackup = readHeader(ATTENDANCE_FILE);
    backupAvailable = true;
    Table workingRows = attendanceBackup;
    bool hasLogId = attendanceHeaderBackup.size() >= 5
                    && attendanceHeaderBackup[0] == "log_id";

    cout << "\nMark P (present), A (absent), or L (late).\n";
    for (int i = 0; i < static_cast<int>(students.size()); i++) {
        string status;
        do {
            cout << students[i][0] << " - " << students[i][1] << ": ";
            getline(cin, status);
            if (status == "p") status = "P";
            if (status == "a") status = "A";
            if (status == "l") status = "L";
            if (!isAttendanceStatus(status)) {
                cout << "Enter only P, A, or L.\n";
            }
        } while (!isAttendanceStatus(status));

        Row row;
        if (hasLogId) {
            row.push_back(nextLogId(workingRows));
        }
        row.push_back(students[i][0]);
        row.push_back(courseCode);
        row.push_back(date);
        row.push_back(status);
        workingRows.push_back(row);
    }

    if (!writeTXT(ATTENDANCE_FILE, attendanceHeaderBackup, workingRows)) {
        backupAvailable = false;
        return false;
    }
    return true;
}

double getAttendancePct(const string& roll, const string& courseCode) {
    Table rows = readTXT(ATTENDANCE_FILE);
    double earned = 0.0;
    int sessions = 0;

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        int rollIndex = rows[i].size() >= 5 ? 1 : 0;
        int courseIndex = rows[i].size() >= 5 ? 2 : 1;
        int statusIndex = rows[i].size() >= 5 ? 4 : 3;

        if (statusIndex < static_cast<int>(rows[i].size())
            && rows[i][rollIndex] == roll
            && rows[i][courseIndex] == courseCode) {
            sessions++;
            if (rows[i][statusIndex] == "P") {
                earned += 1.0;
            } else if (rows[i][statusIndex] == "L") {
                earned += 0.5;
            }
        }
    }

    if (sessions == 0) {
        return 0.0;
    }
    return earned / sessions * 100.0;
}

vector<AttendanceShortage> getShortageList() {
    Table enrollments = readTXT("enrollments.txt");
    vector<AttendanceShortage> shortages;

    for (int i = 0; i < static_cast<int>(enrollments.size()); i++) {
        if (enrollments[i].size() <= 5
            || (enrollments[i][5] != "active"
                && enrollments[i][5] != "enrolled")) {
            continue;
        }

        bool alreadyChecked = false;
        for (int j = 0; j < i; j++) {
            if (enrollments[j].size() > 5
                && enrollments[j][1] == enrollments[i][1]
                && enrollments[j][2] == enrollments[i][2]) {
                alreadyChecked = true;
            }
        }
        if (alreadyChecked) {
            continue;
        }

        double percentage = getAttendancePct(enrollments[i][1],
                                             enrollments[i][2]);
        if (percentage < 75.0) {
            AttendanceShortage item;
            item.roll = enrollments[i][1];
            item.name = getStudentName(item.roll);
            item.courseCode = enrollments[i][2];
            item.percentage = percentage;
            shortages.push_back(item);
        }
    }

    return shortages;
}

bool undoLastSession() {
    if (!backupAvailable) {
        return false;
    }

    bool restored = writeTXT(ATTENDANCE_FILE,
                             attendanceHeaderBackup,
                             attendanceBackup);
    if (restored) {
        backupAvailable = false;
    }
    return restored;
}

void printDailySheet(const string& courseCode, const string& date) {
    Table rows = readTXT(ATTENDANCE_FILE);
    cout << "\n" << left
         << setw(15) << "Roll Number"
         << setw(28) << "Student Name"
         << setw(10) << "Status" << "\n";
    cout << string(53, '-') << "\n";

    int printed = 0;
    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        int rollIndex = rows[i].size() >= 5 ? 1 : 0;
        int courseIndex = rows[i].size() >= 5 ? 2 : 1;
        int dateIndex = rows[i].size() >= 5 ? 3 : 2;
        int statusIndex = rows[i].size() >= 5 ? 4 : 3;

        if (statusIndex < static_cast<int>(rows[i].size())
            && rows[i][courseIndex] == courseCode
            && rows[i][dateIndex] == date) {
            cout << left
                 << setw(15) << rows[i][rollIndex]
                 << setw(28) << getStudentName(rows[i][rollIndex])
                 << setw(10) << rows[i][statusIndex] << "\n";
            printed++;
        }
    }

    if (printed == 0) {
        cout << "No attendance rows found for this course and date.\n";
    }
}
