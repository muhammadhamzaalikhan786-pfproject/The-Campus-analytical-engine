#include "student_ops.h"
#include "filehandler.h"
#include <iostream>

using namespace std;

bool hasDigits(string s) {
    for(int i = 0; i < s.length(); i++) {
        if(s[i] >= '0' && s[i] <= '9') return true;
    }
    return false;
}
bool containsSubstring(string mainStr, string subStr) {
    if (subStr.length() > mainStr.length()) return false;
    for (int i = 0; i <= mainStr.length() - subStr.length(); i++) {
        bool match = true;
        for (int j = 0; j < subStr.length(); j++) {
            if (mainStr[i + j] != subStr[j]) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    return false;
}
bool addStudent(string roll, string name, string dept, string cgpa) {
    if(roll.length() != 11 || roll.substr(0, 5) != "BSAI-" || roll[7] != '-') {
        cout << "Error: Invalid Roll Number format.\n";
        return false;
    }
    for(int i=5; i<=6; i++) if(roll[i] < '0' || roll[i] > '9') return false;
    for(int i=8; i<=10; i++) if(roll[i] < '0' || roll[i] > '9') return false;

    if(rowExists("students.txt", 0, roll)) {
        cout << "Error: Student already exists.\n";
        return false;
    }

    if(hasDigits(name)) {
        cout << "Error: Name cannot contain digits.\n";
        return false;
    }

    double gpa = stod(cgpa);
    if(gpa < 0.0 || gpa > 4.0) {
        cout << "Error: CGPA must be between 0.0 and 4.0.\n";
        return false;
    }

    vector<string> newRow = {roll, name, dept, cgpa, "active"};
    return appendTXT("students.txt", newRow);
}

vector<string> searchByRoll(string roll) {
    return findRow("students.txt", 0, roll);
}

vector<vector<string>> searchByName(string nameSubstring) {
    vector<vector<string>> allStudents = readTXT("students.txt");
    vector<vector<string>> matches;
    for(int i=0; i<allStudents.size(); i++) {
        if(allStudents[i].size() > 1 && containsSubstring(allStudents[i][1], nameSubstring)) {
            matches.push_back(allStudents[i]);
        }
    }
    return matches;
}

bool updateStudent(string roll, int colIndex, string newValue) {
    if(colIndex == 0) {
        cout << "Error: Cannot update roll number.\n";
        return false;
    }
    vector<vector<string>> allStudents = readTXT("students.txt");
    bool updated = false;
    for(int i=0; i<allStudents.size(); i++) {
        if(allStudents[i][0] == roll) {
            allStudents[i][colIndex] = newValue;
            updated = true;
            break;
        }
    }
    if(updated) {
        vector<string> headers = {"roll","name","dept","cgpa","status"};
        return writeTXT("students.txt", headers, allStudents);
    }
    return false;
}

bool softDelete(string roll) {
    return updateStudent(roll, 4, "inactive");
}

vector<vector<string>> listActiveStudents() {
    vector<vector<string>> allStudents = readTXT("students.txt");
    vector<vector<string>> activeStudents;
    
    for(int i=0; i<allStudents.size(); i++) {
        if(allStudents[i].size() > 4 && allStudents[i][4] == "active") {
            activeStudents.push_back(allStudents[i]);
        }
    }

    for(int i=0; i < activeStudents.size(); i++) {
        int min_idx = i;
        for(int j = i+1; j < activeStudents.size(); j++) {
            if(activeStudents[j][0] < activeStudents[min_idx][0]) {
                min_idx = j;
            }
        }
        if(min_idx != i) {
            vector<string> temp = activeStudents[i];
            activeStudents[i] = activeStudents[min_idx];
            activeStudents[min_idx] = temp;
        }
    }
    return activeStudents;
}