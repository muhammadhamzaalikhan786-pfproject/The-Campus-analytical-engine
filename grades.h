#ifndef GRADES_H
#define GRADES_H

#include <string>
#include <vector>

struct Stats {
    double highest;
    double lowest;
    double mean;
    double median;
};

struct GradeRecord {
    std::string roll;
    std::string courseCode;
    int semester;
    double quizAverage;
    double assignmentAverage;
    double midMarks;
    double finalMarks;
    double weightedTotal;
    std::string letterGrade;
    double gpaPoints;
};

bool enterMarks(const std::string& roll, const std::string& courseCode,
                int semester, const std::vector<double>& quizzes,
                const std::vector<double>& assignments,
                double midMarks, double finalMarks);
double bestThreeOfFive(const std::vector<double>& marks);
double computeWeightedTotal(double quizAverage, double assignmentAverage,
                            double midMarks, double finalMarks);
std::string getLetterGrade(double total);
double gradePoints(const std::string& letterGrade);
double computeGPA(const std::string& roll, int semester);
Stats computeClassState(const std::string& courseCode, int semester);
std::string applyAttendancePenalty(const std::string& roll,
                                   const std::string& courseCode,
                                   const std::string& calculatedGrade);
bool getStoredGrade(const std::string& roll, const std::string& courseCode,
                    int semester, GradeRecord& record);

#endif
