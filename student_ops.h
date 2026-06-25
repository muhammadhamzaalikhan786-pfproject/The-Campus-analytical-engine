#ifndef STUDENT_OPS_H
#define STUDENT_OPS_H
#include <string>
#include <vector>

using namespace std;

bool addStudent(string roll, string name, string dept, string cgpa);
vector<string> searchByRoll(string roll);
vector<vector<string>> searchByName(string nameSubstring);
bool updateStudent(string roll, int colIndex, string newValue);
bool softDelete(string roll);
vector<vector<string>> listActiveStudents();

#endif