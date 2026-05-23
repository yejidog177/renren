#include "datamanager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

DataManager::DataManager(const std::string& dir) : dataDir(dir) {
    loadAll();
    if (majors.empty()) {
        initDefaultData();
    }
}

DataManager::~DataManager() {
    saveAll();
    for (size_t i = 0; i < users.size(); ++i) {
        delete users[i];
    }
    users.clear();
}

void DataManager::initDefaultData() {
    majors.push_back("计算机科学与技术");
    majors.push_back("软件工程");
    majors.push_back("网络工程");

    std::vector<std::string> cs;
    cs.push_back("计科2301"); cs.push_back("计科2302");
    cs.push_back("计科2303"); cs.push_back("计科2304");
    classes["计算机科学与技术"] = cs;

    std::vector<std::string> se;
    se.push_back("软件2301"); se.push_back("软件2302");
    se.push_back("软件2303"); se.push_back("软件2304");
    classes["软件工程"] = se;

    std::vector<std::string> ne;
    ne.push_back("网络2301"); ne.push_back("网络2302");
    ne.push_back("网络2303"); ne.push_back("网络2304");
    classes["网络工程"] = ne;

// ==================== User Operations ====================

bool DataManager::addUser(User* user) {
    if (userExists(user->getUsername())) {
        return false;
    }
    users.push_back(user);
    saveAll();
    return true;
}

User* DataManager::findUser(const std::string& username) {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i]->getUsername() == username) return users[i];
    }
    return NULL;
}

bool DataManager::userExists(const std::string& username) {
    return findUser(username) != NULL;
}

User* DataManager::authenticate(const std::string& username, const std::string& password) {
    User* u = findUser(username);
    if (u != NULL && u->checkPassword(password)) return u;
    return NULL;
}

// ==================== Major Operations ====================

const std::vector<std::string>& DataManager::getMajors() const {
    return majors;
}

bool DataManager::majorExists(const std::string& major) const {
    return std::find(majors.begin(), majors.end(), major) != majors.end();
}

// ==================== Class Operations ====================

const std::vector<std::string>& DataManager::getClasses(const std::string& major) const {
    std::map<std::string, std::vector<std::string> >::const_iterator it = classes.find(major);
    if (it != classes.end()) return it->second;
    static std::vector<std::string> empty;
    return empty;
}

bool DataManager::classExists(const std::string& major, const std::string& className) const {
    std::map<std::string, std::vector<std::string> >::const_iterator it = classes.find(major);
    if (it == classes.end()) return false;
    const std::vector<std::string>& clist = it->second;
    return std::find(clist.begin(), clist.end(), className) != clist.end();
}

// ==================== Persistence ====================

void DataManager::loadAll() {
    std::string userFile = dataDir + "users.dat";
    std::string majorFile = dataDir + "majors.dat";
    std::string classFile = dataDir + "classes.dat";

    std::ifstream uf(userFile.c_str());
    if (uf.is_open()) {
        std::string line;
        while (std::getline(uf, line)) {
            if (!line.empty()) {
                User* u = User::deserialize(line);
                if (u != NULL) {
                    users.push_back(u);
                }
            }
        }
        uf.close();
    }

    std::ifstream mf(majorFile.c_str());
    if (mf.is_open()) {
        std::string line;
        while (std::getline(mf, line)) {
            if (!line.empty()) majors.push_back(line);
        }
        mf.close();
    }

    std::ifstream cf(classFile.c_str());
    if (cf.is_open()) {
        std::string line;
        while (std::getline(cf, line)) {
            if (!line.empty()) {
                size_t pos = line.find('|');
                if (pos != std::string::npos) {
                    std::string major = line.substr(0, pos);
                    std::string cls = line.substr(pos + 1);
                    classes[major].push_back(cls);
                }
            }
        }
        cf.close();
    }
}

void DataManager::saveAll() const {
    std::string userFile = dataDir + "users.dat";
    std::string majorFile = dataDir + "majors.dat";
    std::string classFile = dataDir + "classes.dat";

    std::ofstream uf(userFile.c_str());
    if (uf.is_open()) {
        for (size_t i = 0; i < users.size(); ++i) {
            uf << users[i]->serialize() << "\n";
        }
        uf.close();
    }

    std::ofstream mf(majorFile.c_str());
    if (mf.is_open()) {
        for (size_t i = 0; i < majors.size(); ++i) {
            mf << majors[i] << "\n";
        }
        mf.close();
    }

    std::ofstream cf(classFile.c_str());
    if (cf.is_open()) {
        for (std::map<std::string, std::vector<std::string> >::const_iterator it = classes.begin();
             it != classes.end(); ++it) {
            for (size_t i = 0; i < it->second.size(); ++i) {
                cf << it->first << "|" << it->second[i] << "\n";
            }
        }
        cf.close();
    }
}
