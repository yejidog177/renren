#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "user.h"
#include <vector>
#include <string>
#include <map>

class DataManager {
private:
    std::vector<User*> users;
    std::vector<std::string> majors;
    std::map<std::string, std::vector<std::string> > classes;

    std::string dataDir;

    void initDefaultData();

public:
    DataManager(const std::string& dir);
    ~DataManager();

    bool addUser(User* user);
    User* findUser(const std::string& username);
    bool userExists(const std::string& username);
    User* authenticate(const std::string& username, const std::string& password);

    const std::vector<std::string>& getMajors() const;
    bool majorExists(const std::string& major) const;

    const std::vector<std::string>& getClasses(const std::string& major) const;
    bool classExists(const std::string& major, const std::string& className) const;

    void loadAll();
    void saveAll() const;
};

#endif
