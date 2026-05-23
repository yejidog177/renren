#ifndef ANNOUNCEMENT_H
#define ANNOUNCEMENT_H

#include <string>

class Announcement {
private:
    int id;
    std::string title;
    std::string content;
    std::string author;
    std::string createTime;
    bool acceptedList;  // 是否为入选名单公告

public:
    Announcement();
    Announcement(int i, const std::string& t, const std::string& c,
                 const std::string& a, const std::string& ct, bool al);

    int getId() const;
    std::string getTitle() const;
    std::string getContent() const;
    std::string getAuthor() const;
    std::string getCreateTime() const;
    bool isAcceptedList() const;

    void setTitle(const std::string& t);
    void setContent(const std::string& c);
    void setAcceptedList(bool al);

    std::string serialize() const;
    static Announcement* deserialize(const std::string& line);
};

#endif
