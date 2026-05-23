#include "user.h"
#include <sstream>
#include <iostream>

// ==================== User ====================

User::User() : username(""), password(""), role("") {}

User::User(const std::string& u, const std::string& p, const std::string& r)
    : username(u), password(p), role(r) {}

User::~User() {}

std::string User::getUsername() const { return username; }
std::string User::getPassword() const { return password; }
std::string User::getRole() const { return role; }

bool User::checkPassword(const std::string& p) const {
    return password == p;
}

std::string User::serialize() const {
    return username + "|" + password + "|" + role + "| | | ";
}

User* User::deserialize(const std::string& line) {
    std::istringstream ss(line);
    std::string u, p, r, m, c, s;
    std::getline(ss, u, '|');
    std::getline(ss, p, '|');
    std::getline(ss, r, '|');
    std::getline(ss, m, '|');
    std::getline(ss, c, '|');
    std::getline(ss, s, '|');

    if (r == "student") {
        return new Student(u, p, m, c, s);
    } else {
        return new User(u, p, r);
    }
}

// ==================== Student ====================

Student::Student() : User(), major(""), className(""), studentId("") {}

Student::Student(const std::string& u, const std::string& p,
                 const std::string& m, const std::string& c, const std::string& s)
    : User(u, p, "student"), major(m), className(c), studentId(s) {}

std::string Student::getMajor() const { return major; }
std::string Student::getClassName() const { return className; }
std::string Student::getStudentId() const { return studentId; }

std::string Student::serialize() const {
    return username + "|" + password + "|" + role + "|"
           + major + "|" + className + "|" + studentId;
}

void Student::displayInfo() const {
    std::cout << "\n========== 学生信息 ==========\n";
    std::cout << "用户名: " << username << "\n";
    std::cout << "专  业: " << major << "\n";
    std::cout << "班  级: " << className << "\n";
    std::cout << "学  号: " << studentId << "\n";
    std::cout << "================================\n";
}
