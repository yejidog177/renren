#include "datamanager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>

DataManager::DataManager(const std::string& dir)
    : dataDir(dir), nextAnnouncementId(1), nextRegistrationId(1) {
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
    for (size_t i = 0; i < announcements.size(); ++i) {
        delete announcements[i];
    }
    announcements.clear();
    for (size_t i = 0; i < registrations.size(); ++i) {
        delete registrations[i];
    }
    registrations.clear();
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

// ==================== Major/Class Management ====================

bool DataManager::addMajor(const std::string& major) {
    if (majorExists(major)) return false;
    majors.push_back(major);
    saveAll();
    return true;
}

bool DataManager::deleteMajor(const std::string& major) {
    for (std::vector<std::string>::iterator it = majors.begin(); it != majors.end(); ++it) {
        if (*it == major) {
            majors.erase(it);
            classes.erase(major);
            saveAll();
            return true;
        }
    }
    return false;
}

bool DataManager::addClass(const std::string& major, const std::string& className) {
    if (!majorExists(major)) return false;
    if (classExists(major, className)) return false;
    classes[major].push_back(className);
    saveAll();
    return true;
}

bool DataManager::deleteClass(const std::string& major, const std::string& className) {
    if (!majorExists(major)) return false;
    std::vector<std::string>& clist = classes[major];
    for (std::vector<std::string>::iterator it = clist.begin(); it != clist.end(); ++it) {
        if (*it == className) {
            clist.erase(it);
            saveAll();
            return true;
        }
    }
    return false;
}

// ==================== Announcement Operations ====================

bool DataManager::addAnnouncement(const std::string& title, const std::string& content,
                                  const std::string& author, bool acceptedList) {
    // 生成时间戳
    time_t now = time(NULL);
    std::string timeStr = std::string(ctime(&now));
    // 移除末尾换行符
    if (!timeStr.empty() && timeStr[timeStr.size() - 1] == '\n') {
        timeStr.erase(timeStr.size() - 1);
    }

    Announcement* a = new Announcement(nextAnnouncementId, title, content,
                                       author, timeStr, acceptedList);
    announcements.push_back(a);
    nextAnnouncementId++;
    saveAll();
    return true;
}

bool DataManager::updateAnnouncement(int id, const std::string& title,
                                     const std::string& content, bool acceptedList) {
    Announcement* a = findAnnouncement(id);
    if (a == NULL) return false;
    a->setTitle(title);
    a->setContent(content);
    a->setAcceptedList(acceptedList);
    saveAll();
    return true;
}

bool DataManager::deleteAnnouncement(int id) {
    for (size_t i = 0; i < announcements.size(); ++i) {
        if (announcements[i]->getId() == id) {
            delete announcements[i];
            announcements.erase(announcements.begin() + i);
            saveAll();
            return true;
        }
    }
    return false;
}

const std::vector<Announcement*>& DataManager::getAnnouncements() const {
    return announcements;
}

std::vector<Announcement*> DataManager::getAcceptedLists() const {
    std::vector<Announcement*> result;
    for (size_t i = 0; i < announcements.size(); ++i) {
        if (announcements[i]->isAcceptedList()) {
            result.push_back(announcements[i]);
        }
    }
    return result;
}

std::vector<Announcement*> DataManager::getGeneralAnnouncements() const {
    std::vector<Announcement*> result;
    for (size_t i = 0; i < announcements.size(); ++i) {
        if (!announcements[i]->isAcceptedList()) {
            result.push_back(announcements[i]);
        }
    }
    return result;
}

Announcement* DataManager::findAnnouncement(int id) {
    for (size_t i = 0; i < announcements.size(); ++i) {
        if (announcements[i]->getId() == id) return announcements[i];
    }
    return NULL;
}

// ==================== Registration Operations ====================

bool DataManager::saveRegistration(Registration* reg) {
    if (reg->getId() == 0) {
        // 新报名，分配ID
        Registration* r = new Registration(
            nextRegistrationId, reg->getUsername(),
            reg->getProgramType(), reg->getDuration(),
            reg->getProgramName(), reg->getDescription(),
            reg->getPerformer()
        );
        registrations.push_back(r);
        nextRegistrationId++;
    } else {
        // 更新已有报名，并重置审核状态
        Registration* existing = findRegistration(reg->getId());
        if (existing != NULL) {
            existing->setProgramType(reg->getProgramType());
            existing->setDuration(reg->getDuration());
            existing->setProgramName(reg->getProgramName());
            existing->setDescription(reg->getDescription());
            existing->setPerformer(reg->getPerformer());
            existing->setStatus("pending");
            existing->setReviewComment("");
        }
    }
    delete reg;
    saveAll();
    return true;
}

Registration* DataManager::getRegistrationByUser(const std::string& username) {
    for (size_t i = 0; i < registrations.size(); ++i) {
        if (registrations[i]->getUsername() == username) {
            return registrations[i];
        }
    }
    return NULL;
}

Registration* DataManager::findRegistration(int id) {
    for (size_t i = 0; i < registrations.size(); ++i) {
        if (registrations[i]->getId() == id) return registrations[i];
    }
    return NULL;
}

const std::vector<Registration*>& DataManager::getAllRegistrations() const {
    return registrations;
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

    // 加载公告
    std::string announceFile = dataDir + "announcements.dat";
    std::ifstream af(announceFile.c_str());
    if (af.is_open()) {
        std::string line;
        while (std::getline(af, line)) {
            if (!line.empty()) {
                Announcement* a = Announcement::deserialize(line);
                if (a != NULL) {
                    announcements.push_back(a);
                    if (a->getId() >= nextAnnouncementId) {
                        nextAnnouncementId = a->getId() + 1;
                    }
                }
            }
        }
        af.close();
    }

    // 加载报名
    std::string regFile = dataDir + "registrations.dat";
    std::ifstream rf(regFile.c_str());
    if (rf.is_open()) {
        std::string line;
        while (std::getline(rf, line)) {
            if (!line.empty()) {
                Registration* r = Registration::deserialize(line);
                if (r != NULL) {
                    registrations.push_back(r);
                    if (r->getId() >= nextRegistrationId) {
                        nextRegistrationId = r->getId() + 1;
                    }
                }
            }
        }
        rf.close();
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

    // 保存公告
    std::string announceFile = dataDir + "announcements.dat";
    std::ofstream af(announceFile.c_str());
    if (af.is_open()) {
        for (size_t i = 0; i < announcements.size(); ++i) {
            af << announcements[i]->serialize() << "\n";
        }
        af.close();
    }

    // 保存报名
    std::string regFile = dataDir + "registrations.dat";
    std::ofstream rf(regFile.c_str());
    if (rf.is_open()) {
        for (size_t i = 0; i < registrations.size(); ++i) {
            rf << registrations[i]->serialize() << "\n";
        }
        rf.close();
    }
}
