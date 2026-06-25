#include "reports.h"

#include "../M4_attendance/attendance.h"
#include "../M6_fee_tracker/fee_tracker.h"
#include "../M1_filehandler/filehandler.h"
#include "../M5_grades/grades.h"
#include "../M2_student_ops/student_ops.h"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

void printMeritList() {
    Table students = listActiveStudents();

    for (int i = 0; i < static_cast<int>(students.size()); i++) {
        int highest = i;
        for (int j = i + 1; j < static_cast<int>(students.size()); j++) {
            if (atof(students[j][4].c_str())
                > atof(students[highest][4].c_str())) {
                highest = j;
            }
        }
        if (highest != i) {
            Row temporary = students[i];
            students[i] = students[highest];
            students[highest] = temporary;
        }
    }

    cout << "\nMERIT LIST\n";
    cout << left << setw(7) << "Rank"
         << setw(15) << "Roll Number"
         << setw(28) << "Student Name"
         << setw(12) << "CGPA" << "\n";
    cout << string(62, '-') << "\n";

    for (int i = 0; i < static_cast<int>(students.size()); i++) {
        cout << left << setw(7) << i + 1
             << setw(15) << students[i][0]
             << setw(28) << students[i][1]
             << setw(12) << students[i][4] << "\n";
    }
}

void printAttendanceDefaulters() {
    vector<AttendanceShortage> rows = getShortageList();

    cout << "\nATTENDANCE DEFAULTERS\n";
    cout << left << setw(15) << "Roll Number"
         << setw(26) << "Student Name"
         << setw(14) << "Course"
         << setw(12) << "Percent" << "\n";
    cout << string(67, '-') << "\n";

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        cout << left << setw(15) << rows[i].roll
             << setw(26) << rows[i].name
             << setw(14) << rows[i].courseCode
             << fixed << setprecision(2)
             << setw(12) << rows[i].percentage << "\n";
    }

    if (rows.empty()) {
        cout << "No attendance defaulters were found.\n";
    }
}

void printFeeDefaulters(const string& currentDate) {
    vector<FeeDefaulter> rows = getDefaulters(currentDate);

    cout << "\nFEE DEFAULTERS AS OF " << currentDate << "\n";
    cout << left << setw(15) << "Roll Number"
         << setw(26) << "Student Name"
         << setw(12) << "Semester"
         << setw(16) << "Outstanding"
         << setw(14) << "Weeks Late" << "\n";
    cout << string(83, '-') << "\n";

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        cout << left << setw(15) << rows[i].roll
             << setw(26) << rows[i].name
             << setw(12) << rows[i].semester
             << fixed << setprecision(2)
             << setw(16) << rows[i].outstandingAmount
             << setw(14) << rows[i].weeksOverdue << "\n";
    }

    if (rows.empty()) {
        cout << "No fee defaulters were found.\n";
    }
}

void printSemesterResult(int semester) {
    Table students = listActiveStudents();
    Table enrollments = readTXT("enrollments.txt");

    cout << "\nSEMESTER " << semester << " RESULT SHEET\n";
    cout << left << setw(15) << "Roll Number"
         << setw(23) << "Student Name"
         << setw(10) << "Course"
         << setw(9) << "Total"
         << setw(9) << "Grade"
         << setw(9) << "GPA"
         << setw(13) << "Attendance" << "\n";
    cout << string(88, '=') << "\n";

    for (int s = 0; s < static_cast<int>(students.size()); s++) {
        bool printedStudent = false;

        for (int e = 0; e < static_cast<int>(enrollments.size()); e++) {
            if (enrollments[e].size() <= 5
                || enrollments[e][1] != students[s][0]
                || atoi(enrollments[e][3].c_str()) != semester
                || (enrollments[e][5] != "active"
                    && enrollments[e][5] != "enrolled")) {
                continue;
            }

            GradeRecord grade;
            bool hasGrade = getStoredGrade(students[s][0],
                                           enrollments[e][2],
                                           semester, grade);
            double attendance = getAttendancePct(students[s][0],
                                                 enrollments[e][2]);

            cout << left
                 << setw(15) << (printedStudent ? "" : students[s][0])
                 << setw(23) << (printedStudent ? "" : students[s][1])
                 << setw(10) << enrollments[e][2];

            if (hasGrade) {
                cout << fixed << setprecision(2)
                     << setw(9) << grade.weightedTotal
                     << setw(9) << grade.letterGrade
                     << setw(9) << computeGPA(students[s][0], semester);
            } else {
                cout << setw(9) << "N/A"
                     << setw(9) << "N/A"
                     << setw(9) << "N/A";
            }

            cout << setw(13) << (attendance >= 75.0 ? "Clear" : "Shortage")
                 << "\n";
            printedStudent = true;
        }
        if (printedStudent) {
            cout << string(88, '-') << "\n";
        }
    }
}

void printDepartmentSummary() {
    Table students = listActiveStudents();
    vector<string> departments;
    vector<int> counts;
    vector<double> cgpaTotals;
    vector<int> passingCounts;

    for (int i = 0; i < static_cast<int>(students.size()); i++) {
        int departmentIndex = -1;
        for (int j = 0; j < static_cast<int>(departments.size()); j++) {
            if (departments[j] == students[i][2]) {
                departmentIndex = j;
            }
        }

        if (departmentIndex == -1) {
            departments.push_back(students[i][2]);
            counts.push_back(0);
            cgpaTotals.push_back(0.0);
            passingCounts.push_back(0);
            departmentIndex = static_cast<int>(departments.size()) - 1;
        }

        double cgpa = atof(students[i][4].c_str());
        counts[departmentIndex]++;
        cgpaTotals[departmentIndex] += cgpa;
        if (cgpa >= 2.0) {
            passingCounts[departmentIndex]++;
        }
    }

    cout << "\nDEPARTMENT SUMMARY\n";
    cout << left << setw(34) << "Department"
         << setw(12) << "Students"
         << setw(14) << "Avg CGPA"
         << setw(14) << "Pass Rate" << "\n";
    cout << string(74, '-') << "\n";

    for (int i = 0; i < static_cast<int>(departments.size()); i++) {
        double averageCgpa = counts[i] == 0
                             ? 0.0 : cgpaTotals[i] / counts[i];
        double passRate = counts[i] == 0
                          ? 0.0 : passingCounts[i] * 100.0 / counts[i];

        cout << left << setw(34) << departments[i]
             << setw(12) << counts[i]
             << fixed << setprecision(2)
             << setw(14) << averageCgpa
             << setw(14) << passRate << "\n";
    }
}

bool exportReportToFile(const string& reportName,
                        const string& outputFilename,
                        int semester,
                        const string& currentDate) {
    ofstream output(outputFilename.c_str());
    if (!output.is_open()) {
        return false;
    }

    streambuf* originalOutput = cout.rdbuf();
    cout.rdbuf(output.rdbuf());

    if (reportName == "merit") {
        printMeritList();
    } else if (reportName == "attendance") {
        printAttendanceDefaulters();
    } else if (reportName == "fees") {
        printFeeDefaulters(currentDate);
    } else if (reportName == "result") {
        printSemesterResult(semester);
    } else if (reportName == "department") {
        printDepartmentSummary();
    } else {
        cout.rdbuf(originalOutput);
        output.close();
        return false;
    }

    cout.rdbuf(originalOutput);
    output.close();
    return true;
}
