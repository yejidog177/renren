#include "user.h"
#include <sstream>
#include <iostream>

// 简单的 XOR 密码编码（防止明文存储）
std::string User::encode(const std::string& s) {
    std::string result = s;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] ^= 0x5A;
    }
    return result;
}

std::string User::decode(const std::string& s) {
    return encode(s);  // XOR 加密和解密是同一个操作
}

// ==================== User ====================

User::User() : username(""), password(""), role("") {}

User::User(const std::string& u, const std::string& p, const std::string& r)
    : username(u), password(encode(p)), role(r) {}

User::~User() {}

std::string User::getUsername() const { return username; }
std::string User::getRole() const { return role; }

bool User::checkPassword(const std::string& p) const {
    return decode(password) == p;
}

void User::setPassword(const std::string& newPassword) {
    password = encode(newPassword);
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

    // 格式校验：用户名和角色不能为空
    if (u.empty() || r.empty()) {
        return NULL;
    }

    if (r == "student") {
        if (m.empty() || c.empty() || s.empty()) {
            return NULL;  // 学生必须有完整的专业/班级/学号信息
        }
        return new Student(u, decode(p), m, c, s);
    } else {
        return new User(u, decode(p), r);
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
