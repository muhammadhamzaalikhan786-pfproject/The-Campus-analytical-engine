#include "fee_tracker.h"

#include "../M1_filehandler/filehandler.h"
#include "../M2_student_ops/student_ops.h"

#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

static const string FEES_FILE = "fees.txt";

static bool leapYear(int year) {
    if (year % 400 == 0) return true;
    if (year % 100 == 0) return false;
    return year % 4 == 0;
}

static void parseDateParts(const string& date,
                           int& day, int& month, int& year) {
    day = atoi(date.substr(0, 2).c_str());
    month = atoi(date.substr(3, 2).c_str());
    year = atoi(date.substr(6, 4).c_str());
}

bool validateDate(const string& date) {
    if (date.length() != 10 || date[2] != '-' || date[5] != '-') {
        return false;
    }

    for (int i = 0; i < 10; i++) {
        if (i != 2 && i != 5
            && !isdigit(static_cast<unsigned char>(date[i]))) {
            return false;
        }
    }

    int day, month, year;
    parseDateParts(date, day, month, year);
    if (year < 1 || month < 1 || month > 12) {
        return false;
    }

    int monthLengths[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };
    if (leapYear(year)) {
        monthLengths[1] = 29;
    }

    return day >= 1 && day <= monthLengths[month - 1];
}

static long totalDayNumber(const string& date) {
    int day, month, year;
    parseDateParts(date, day, month, year);
    long total = day;

    for (int currentYear = 1; currentYear < year; currentYear++) {
        total += leapYear(currentYear) ? 366 : 365;
    }

    int monthLengths[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };
    if (leapYear(year)) {
        monthLengths[1] = 29;
    }

    for (int currentMonth = 1; currentMonth < month; currentMonth++) {
        total += monthLengths[currentMonth - 1];
    }

    return total;
}

int daysBetween(const string& firstDate, const string& secondDate) {
    if (!validateDate(firstDate) || !validateDate(secondDate)) {
        return 0;
    }
    return static_cast<int>(totalDayNumber(secondDate)
                            - totalDayNumber(firstDate));
}

static string numberText(int value) {
    stringstream output;
    output << value;
    return output.str();
}

static string moneyText(double value) {
    stringstream output;
    output << fixed << setprecision(2) << value;
    return output.str();
}

bool recordPayment(const string& roll, int semester,
                   double amount, const string& paidDate,
                   const string& paymentMethod) {
    if (!isStudentActive(roll) || amount <= 0.0
        || !validateDate(paidDate) || paymentMethod.empty()) {
        return false;
    }

    Table rows = readTXT(FEES_FILE);
    Row header = readHeader(FEES_FILE);
    string wantedSemester = numberText(semester);

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        if (rows[i].size() > 8
            && rows[i][1] == roll
            && rows[i][2] == wantedSemester) {
            double totalFee = atof(rows[i][3].c_str());
            double alreadyPaid = atof(rows[i][4].c_str());
            if (alreadyPaid + amount > totalFee) {
                return false;
            }

            double newPaid = alreadyPaid + amount;
            rows[i][4] = moneyText(newPaid);
            rows[i][6] = paidDate;
            rows[i][7] = paymentMethod;

            if (newPaid >= totalFee) {
                rows[i][8] = daysBetween(rows[i][5], paidDate) > 0
                             ? "paid_late" : "paid";
            } else {
                rows[i][8] = "partial";
            }
            return writeTXT(FEES_FILE, header, rows);
        }
    }

    return false;
}

double computeLateFine(const string& roll, int semester) {
    Table rows = readTXT(FEES_FILE);
    string wantedSemester = numberText(semester);

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        if (rows[i].size() > 8
            && rows[i][1] == roll
            && rows[i][2] == wantedSemester) {
            if (!validateDate(rows[i][5]) || !validateDate(rows[i][6])) {
                return 0.0;
            }
            int lateDays = daysBetween(rows[i][5], rows[i][6]);
            int completeWeeks = lateDays > 0 ? lateDays / 7 : 0;
            return atof(rows[i][3].c_str()) * 0.02 * completeWeeks;
        }
    }
    return 0.0;
}

void generateReceipt(const string& roll, int semester) {
    Table rows = readTXT(FEES_FILE);
    string wantedSemester = numberText(semester);

    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        if (rows[i].size() > 8
            && rows[i][1] == roll
            && rows[i][2] == wantedSemester) {
            double tuition = atof(rows[i][3].c_str());
            double paid = atof(rows[i][4].c_str());
            double fine = computeLateFine(roll, semester);
            double balance = tuition + fine - paid;

            cout << "\n" << setfill('=') << setw(50) << "" << "\n";
            cout << setfill(' ') << setw(33) << "CAMPUS FEE RECEIPT\n";
            cout << setfill('=') << setw(50) << "" << "\n";
            cout << setfill(' ') << left << setw(22) << "Roll Number:"
                 << roll << "\n";
            cout << left << setw(22) << "Student Name:"
                 << getStudentName(roll) << "\n";
            cout << left << setw(22) << "Semester:"
                 << semester << "\n";
            cout << left << setw(22) << "Tuition Fee:"
                 << fixed << setprecision(2) << tuition << "\n";
            cout << left << setw(22) << "Late Fine:"
                 << fine << "\n";
            cout << left << setw(22) << "Total Payable:"
                 << tuition + fine << "\n";
            cout << left << setw(22) << "Amount Paid:"
                 << paid << "\n";
            cout << left << setw(22) << "Balance:"
                 << (balance > 0.0 ? balance : 0.0) << "\n";
            cout << left << setw(22) << "Payment Date:"
                 << rows[i][6] << "\n";
            cout << left << setw(22) << "Payment Method:"
                 << rows[i][7] << "\n";
            cout << setfill('=') << setw(50) << "" << setfill(' ') << "\n";
            return;
        }
    }

    cout << "No fee record was found.\n";
}

vector<FeeDefaulter> getDefaulters(const string& currentDate) {
    vector<FeeDefaulter> defaulters;
    if (!validateDate(currentDate)) {
        return defaulters;
    }

    Table rows = readTXT(FEES_FILE);
    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        if (rows[i].size() <= 8 || !validateDate(rows[i][5])) {
            continue;
        }

        double totalFee = atof(rows[i][3].c_str());
        double amountPaid = atof(rows[i][4].c_str());
        double outstanding = totalFee - amountPaid;
        int overdueDays = daysBetween(rows[i][5], currentDate);

        if (outstanding > 0.0 && overdueDays > 0) {
            FeeDefaulter item;
            item.roll = rows[i][1];
            item.name = getStudentName(item.roll);
            item.semester = atoi(rows[i][2].c_str());
            item.outstandingAmount = outstanding;
            item.weeksOverdue = overdueDays / 7;
            defaulters.push_back(item);
        }
    }

    for (int pass = 0; pass < static_cast<int>(defaulters.size()); pass++) {
        for (int i = 0;
             i + 1 < static_cast<int>(defaulters.size()) - pass; i++) {
            if (defaulters[i].outstandingAmount
                < defaulters[i + 1].outstandingAmount) {
                FeeDefaulter temporary = defaulters[i];
                defaulters[i] = defaulters[i + 1];
                defaulters[i + 1] = temporary;
            }
        }
    }

    return defaulters;
}
