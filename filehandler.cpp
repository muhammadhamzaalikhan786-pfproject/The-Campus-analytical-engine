#include "filehandler.h"
#include <fstream>
#include <iostream>

using namespace std;
vector<vector<string>> readTXT(string filename) {
    vector<vector<string>> data;
    ifstream file(filename);
    
    if (!file.is_open()) {
        cout << "Error opening " << filename << endl;
        return data;
    }
    string line;
    bool isFirstLine = true;
    
    while (getline(file, line)) {
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }

        vector<string> row;
        string field = "";
        bool inQuotes = false;

        for (int i = 0; i < line.length(); i++) {
            char c = line[i];
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                row.push_back(field);
                field = "";
            } else {
                field += c;
            }
        }
        row.push_back(field);
        
        if (!row.empty() && row[0] != "") {
            data.push_back(row);
        }
    }
    
    file.close();
    return data;
}

bool appendTXT(string filename, vector<string> row) {
    ofstream file;
    file.open(filename, ios_base::app);
    
    if (!file.is_open()) return false;

    for (int i = 0; i < row.size(); i++) {
        bool hasComma = false;
        for (int j = 0; j < row[i].length(); j++) {
            if (row[i][j] == ',') hasComma = true;
        }

        if (hasComma) file << "\"" << row[i] << "\"";
        else file << row[i];

        if (i < row.size() - 1) file << ",";
    }
    file << "\n";
    
    file.close();
    return true;
}

vector<string> findRow(string filename, int colIndex, string matchValue) {
    vector<vector<string>> data = readTXT(filename);
    vector<string> emptyRow;
    
    for (int i = 0; i < data.size(); i++) {
        if (colIndex < data[i].size() && data[i][colIndex] == matchValue) {
            return data[i];
        }
    }
    return emptyRow;
}

bool rowExists(string filename, int colIndex, string matchValue) {
    vector<string> row = findRow(filename, colIndex, matchValue);
    return !row.empty();
}

bool writeTXT(string filename, vector<string> headers, vector<vector<string>> data) {
    ofstream file(filename);
    if (!file.is_open()) return false;

    for (int i = 0; i < headers.size(); i++) {
        file << headers[i];
        if (i < headers.size() - 1) file << ",";
    }
    file << "\n";
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[i].size(); j++) {
            bool hasComma = false;
            for (int k = 0; k < data[i][j].length(); k++) {
                if (data[i][j][k] == ',') hasComma = true;
            }

            if (hasComma) file << "\"" << data[i][j] << "\"";
            else file << data[i][j];

            if (j < data[i].size() - 1) file << ",";
        }
        file << "\n";
    }
    
    file.close();
    return true;
}