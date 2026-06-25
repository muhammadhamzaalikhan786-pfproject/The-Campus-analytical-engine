#ifndef FEE_TRACKER_H
#define FEE_TRACKER_H

#include <string>
#include <vector>

struct FeeDefaulter {
    std::string roll;
    std::string name;
    int semester;
    double outstandingAmount;
    int weeksOverdue;
};

bool validateDate(const std::string& date);
int daysBetween(const std::string& firstDate,
                const std::string& secondDate);
bool recordPayment(const std::string& roll, int semester,
                   double amount, const std::string& paidDate,
                   const std::string& paymentMethod);
double computeLateFine(const std::string& roll, int semester);
void generateReceipt(const std::string& roll, int semester);
std::vector<FeeDefaulter> getDefaulters(const std::string& currentDate);

#endif
