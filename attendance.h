#ifndef ATTENDANCE_H
#define ATTENDANCE_H

#include "../M1_filehandler/filehandler.h"

#include <string>
#include <vector>

struct AttendanceShortage {
    std::string roll;
    std::string name;
    std::string courseCode;
    double percentage;
};

bool markAttendance(const std::string& courseCode, int semester,
                    const std::string& date);
double getAttendancePct(const std::string& roll,
                        const std::string& courseCode);
std::vector<AttendanceShortage> getShortageList();
bool undoLastSession();
void printDailySheet(const std::string& courseCode,
                     const std::string& date);

#endif
