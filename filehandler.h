#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <vector>

using namespace std;
vector<vector<string>> readTXT(string filename);
bool writeTXT(string filename, vector<string> headers, vector<vector<string>> data);
bool appendTXT(string filename, vector<string> row);
vector<string> findRow(string filename, int colIndex, string matchValue);
bool rowExists(string filename, int colIndex, string matchValue);

#endif