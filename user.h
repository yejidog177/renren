#ifndef USER_H
#define USER_H

#include <string>

class User {
protected:
    std::string username;
    std::string password;  // 存储加密后的密码
    std::string role;

    static std::string encode(const std::string& s);
    static std::string decode(const std::string& s);

public:
    User();
    User(const std::string& u, const std::string& p, const std::string& r);
    virtual ~User();

    std::string getUsername() const;
    std::string getRole() const;
    bool checkPassword(const std::string& p) const;
    void setPassword(const std::string& newPassword);

    virtual std::string serialize() const;
    static User* deserialize(const std::string& line);
};

class Student : public User {
private:
    std::string major;
    std::string className;
    std::string studentId;

public:
    Student();
    Student(const std::string& u, const std::string& p,
            const std::string& m, const std::string& c, const std::string& s);

    std::string getMajor() const;
    std::string getClassName() const;
    std::string getStudentId() const;

    std::string serialize() const;
    void displayInfo() const;
};

#endif
