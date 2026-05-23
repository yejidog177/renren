#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "user.h"
#include "announcement.h"
#include "registration.h"
#include <vector>
#include <string>
#include <map>

class DataManager {
private:
    std::vector<User*> users;
    std::vector<std::string> majors;
    std::map<std::string, std::vector<std::string> > classes;
    std::vector<Announcement*> announcements;
    std::vector<Registration*> registrations;
    int nextAnnouncementId;
    int nextRegistrationId;

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

    // 公告操作
    bool addAnnouncement(const std::string& title, const std::string& content,
                         const std::string& author, bool acceptedList);
    bool updateAnnouncement(int id, const std::string& title,
                            const std::string& content, bool acceptedList);
    bool deleteAnnouncement(int id);
    const std::vector<Announcement*>& getAnnouncements() const;
    std::vector<Announcement*> getAcceptedLists() const;
    std::vector<Announcement*> getGeneralAnnouncements() const;
    Announcement* findAnnouncement(int id);

    // 报名操作
    Registration* createRegistration(const std::string& username);
    bool saveRegistration(Registration* reg);
    Registration* getRegistrationByUser(const std::string& username);
    Registration* findRegistration(int id);
    const std::vector<Registration*>& getAllRegistrations() const;

    void loadAll();
    void saveAll() const;
};

#endif
