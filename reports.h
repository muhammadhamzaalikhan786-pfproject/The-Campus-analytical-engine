#ifndef REPORTS_H
#define REPORTS_H

#include <string>

void printMeritList();
void printAttendanceDefaulters();
void printFeeDefaulters(const std::string& currentDate);
void printSemesterResult(int semester);
void printDepartmentSummary();
bool exportReportToFile(const std::string& reportName,
                        const std::string& outputFilename,
                        int semester,
                        const std::string& currentDate);

#endif
