#include "grades.h"

#include "../M4_attendance/attendance.h"
#include "../M3_course_ops/course_ops.h"
#include "../M1_filehandler/filehandler.h"
#include "../M2_student_ops/student_ops.h"

#include <cstdlib>
#include <iomanip>
#include <sstream>

using namespace std;

static const string ENROLLMENTS_FILE = "enrollments.txt";

static string decimalText(double value) {
    stringstream output;
    output << fixed << setprecision(2) << value;
    return output.str();
}

static string wholeText(int value) {
    stringstream output;
    output << value;
    return output.str();
}

static bool validArray(const vector<double>& values,
                       int maximumCount, double maximumMark) {
    if (values.empty() || static_cast<int>(values.size()) > maximumCount) {
        return false;
    }

    for (int i = 0; i < static_cast<int>(values.size()); i++) {
        if (values[i] < 0.0 || values[i] > maximumMark) {
            return false;
        }
    }
    return true;
}

static double average(const vector<double>& values) {
    if (values.empty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (int i = 0; i < static_cast<int>(values.size()); i++) {
        sum += values[i];
    }
    return sum / values.size();
}

double bestThreeOfFive(const vector<double>& marks) {
    if (marks.empty()) {
        return 0.0;
    }

    if (marks.size() < 3) {
        return average(marks);
    }

    int firstLowest = 0;
    for (int i = 1; i < static_cast<int>(marks.size()); i++) {
        if (marks[i] < marks[firstLowest]) {
            firstLowest = i;
        }
    }

    int secondLowest = firstLowest == 0 ? 1 : 0;
    for (int i = 0; i < static_cast<int>(marks.size()); i++) {
        if (i != firstLowest && marks[i] < marks[secondLowest]) {
            secondLowest = i;
        }
    }

    double sum = 0.0;
    int count = 0;
    for (int i = 0; i < static_cast<int>(marks.size()); i++) {
        if (i != firstLowest && i != secondLowest) {
            sum += marks[i];
            count++;
        }
    }

    return count == 0 ? 0.0 : sum / count;
}

double computeWeightedTotal(double quizAverage, double assignmentAverage,
                            double midMarks, double finalMarks) {
    double midPercentage = midMarks / 40.0 * 100.0;
    double finalPercentage = finalMarks / 60.0 * 100.0;

    return quizAverage * 0.10
           + assignmentAverage * 0.10
           + midPercentage * 0.30
           + finalPercentage * 0.50;
}

string getLetterGrade(double total) {
    if (total >= 85.0) return "A";
    if (total >= 80.0) return "B+";
    if (total >= 70.0) return "B";
    if (total >= 65.0) return "C+";
    if (total >= 60.0) return "C";
    if (total >= 50.0) return "D";
    return "F";
}

double gradePoints(const string& letterGrade) {
    if (letterGrade == "A") return 4.0;
    if (letterGrade == "B+") return 3.5;
    if (letterGrade == "B") return 3.0;
    if (letterGrade == "C+") return 2.5;
    if (letterGrade == "C") return 2.0;
    if (letterGrade == "D") return 1.0;
    return 0.0;
}

string applyAttendancePenalty(const string& roll,
                              const string& courseCode,
                              const string& calculatedGrade) {
    if (getAttendancePct(roll, courseCode) < 75.0) {
        return "F";
    }
    return calculatedGrade;
}

static void addGradeColumns(Row& header, Table& rows) {
    if (header.size() >= 13) {
        return;
    }

    header.push_back("quiz_average");
    header.push_back("assignment_average");
    header.push_back("mid_marks");
    header.push_back("final_marks");
    header.push_back("weighted_total");
    header.push_back("letter_grade");
    header.push_back("gpa_points");

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        while (rows[i].size() < 13) {
            rows[i].push_back("");
        }
    }
}

bool enterMarks(const string& roll, const string& courseCode,
                int semester, const vector<double>& quizzes,
                const vector<double>& assignments,
                double midMarks, double finalMarks) {
    if (!isStudentActive(roll) || findCourse(courseCode).empty()) {
        return false;
    }

    if (!validArray(quizzes, 5, 100.0)
        || !validArray(assignments, 20, 100.0)
        || midMarks < 0.0 || midMarks > 40.0
        || finalMarks < 0.0 || finalMarks > 60.0) {
        return false;
    }

    Table rows = readTXT(ENROLLMENTS_FILE);
    Row header = readHeader(ENROLLMENTS_FILE);
    addGradeColumns(header, rows);
    string wantedSemester = wholeText(semester);

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        if (rows[i].size() > 5
            && rows[i][1] == roll
            && rows[i][2] == courseCode
            && rows[i][3] == wantedSemester
            && (rows[i][5] == "active" || rows[i][5] == "enrolled")) {
            double quizAverage = bestThreeOfFive(quizzes);
            double assignmentAverage = average(assignments);
            double total = computeWeightedTotal(quizAverage,
                                                assignmentAverage,
                                                midMarks, finalMarks);
            string grade = applyAttendancePenalty(
                roll, courseCode, getLetterGrade(total));

            rows[i][6] = decimalText(quizAverage);
            rows[i][7] = decimalText(assignmentAverage);
            rows[i][8] = decimalText(midMarks);
            rows[i][9] = decimalText(finalMarks);
            rows[i][10] = decimalText(total);
            rows[i][11] = grade;
            rows[i][12] = decimalText(gradePoints(grade));
            return writeTXT(ENROLLMENTS_FILE, header, rows);
        }
    }

    return false;
}

bool getStoredGrade(const string& roll, const string& courseCode,
                    int semester, GradeRecord& record) {
    Table rows = readTXT(ENROLLMENTS_FILE);
    string wantedSemester = wholeText(semester);

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        if (rows[i].size() > 12
            && rows[i][1] == roll
            && rows[i][2] == courseCode
            && rows[i][3] == wantedSemester
            && !rows[i][11].empty()) {
            record.roll = roll;
            record.courseCode = courseCode;
            record.semester = semester;
            record.quizAverage = atof(rows[i][6].c_str());
            record.assignmentAverage = atof(rows[i][7].c_str());
            record.midMarks = atof(rows[i][8].c_str());
            record.finalMarks = atof(rows[i][9].c_str());
            record.weightedTotal = atof(rows[i][10].c_str());
            record.letterGrade = rows[i][11];
            record.gpaPoints = atof(rows[i][12].c_str());
            return true;
        }
    }
    return false;
}

double computeGPA(const string& roll, int semester) {
    Table rows = readTXT(ENROLLMENTS_FILE);
    string wantedSemester = wholeText(semester);
    double weightedPoints = 0.0;
    int totalCredits = 0;

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        if (rows[i].size() > 12
            && rows[i][1] == roll
            && rows[i][3] == wantedSemester
            && !rows[i][12].empty()) {
            Row course = findCourse(rows[i][2]);
            if (course.size() > 2) {
                int credits = atoi(course[2].c_str());
                weightedPoints += atof(rows[i][12].c_str()) * credits;
                totalCredits += credits;
            }
        }
    }

    if (totalCredits == 0) {
        return 0.0;
    }
    return weightedPoints / totalCredits;
}

Stats computeClassState(const string& courseCode, int semester) {
    Table rows = readTXT(ENROLLMENTS_FILE);
    vector<double> totals;
    string wantedSemester = wholeText(semester);
    Stats result;
    result.highest = 0.0;
    result.lowest = 0.0;
    result.mean = 0.0;
    result.median = 0.0;

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        if (rows[i].size() > 10
            && rows[i][2] == courseCode
            && rows[i][3] == wantedSemester
            && !rows[i][10].empty()) {
            totals.push_back(atof(rows[i][10].c_str()));
        }
    }

    if (totals.empty()) {
        return result;
    }

    for (int i = 0; i < static_cast<int>(totals.size()); i++) {
        int smallest = i;
        for (int j = i + 1; j < static_cast<int>(totals.size()); j++) {
            if (totals[j] < totals[smallest]) {
                smallest = j;
            }
        }
        if (smallest != i) {
            double temporary = totals[i];
            totals[i] = totals[smallest];
            totals[smallest] = temporary;
        }
    }

    double sum = 0.0;
    for (int i = 0; i < static_cast<int>(totals.size()); i++) {
        sum += totals[i];
    }

    result.lowest = totals[0];
    result.highest = totals[totals.size() - 1];
    result.mean = sum / totals.size();

    int middle = static_cast<int>(totals.size()) / 2;
    if (totals.size() % 2 == 0) {
        result.median = (totals[middle - 1] + totals[middle]) / 2.0;
    } else {
        result.median = totals[middle];
    }
    return result;
}
