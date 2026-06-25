#include <iostream>
#include <string>
#include "filehandler.h"
using namespace std;

void displayMainMenu() {
    cout << "CAMPUS ANALYTICS ENGINE\n";
    cout << "1. Student Management\n";
    cout << "2. Course & Enrollment Management\n";
    cout << "3. Attendance Module\n";
    cout << "4. Grades & Evaluation\n";
    cout << "5. Fee Tracking\n";
    cout << "6. Reports Dashboard\n";
    cout << "0. Exit System\n";
    cout << "Select an option: ";
}

void studentManagementMenu() {
    int choice = -1;
    while (choice != 0) {
        cout << "\n--- Student Management ---\n";
        cout << "1. Add New Student\n";
        cout << "2. Search Student by Roll No\n";
        cout << "3. Search Student by Name\n";
        cout << "4. Update Student Details\n";
        cout << "5. Soft Delete Student\n";
        cout << "0. Back to Main Menu\n";
        cout << "Select an option: ";
        cin >> choice;

        if (choice == 1) {
            cout << "Adding student... (Call addStudent() here)\n";
        } else if (choice == 2) {
            cout << "Searching... (Call searchByRoll() here)\n";
        }
    }
}

int main() {
    int mainChoice = -1;

    while (mainChoice != 0) {
        displayMainMenu();
        cin >> mainChoice;

        switch (mainChoice) {
            case 1:
                studentManagementMenu();
                break;
            case 2:
                cout << "Course menu coming soon...\n";
                break;
            case 3:
                cout << "Attendance menu coming soon...\n";
                break;
            case 4:
                cout << "Grades menu coming soon...\n";
                break;
            case 5:
                cout << "Fees menu coming soon...\n";
                break;
            case 6:
                cout << "Reports menu coming soon...\n";
                break;
            case 0:
                cout << "Exiting system. Goodbye!\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    }
    return 0;
}