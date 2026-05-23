#include "announcement.h"
#include <sstream>

Announcement::Announcement()
    : id(0), title(""), content(""), author(""), createTime(""), acceptedList(false) {}

Announcement::Announcement(int i, const std::string& t, const std::string& c,
                           const std::string& a, const std::string& ct, bool al)
    : id(i), title(t), content(c), author(a), createTime(ct), acceptedList(al) {}

int Announcement::getId() const { return id; }
std::string Announcement::getTitle() const { return title; }
std::string Announcement::getContent() const { return content; }
std::string Announcement::getAuthor() const { return author; }
std::string Announcement::getCreateTime() const { return createTime; }
bool Announcement::isAcceptedList() const { return acceptedList; }

void Announcement::setTitle(const std::string& t) { title = t; }
void Announcement::setContent(const std::string& c) { content = c; }
void Announcement::setAcceptedList(bool al) { acceptedList = al; }

std::string Announcement::serialize() const {
    std::ostringstream oss;
    oss << id << "|" << title << "|" << content << "|"
        << author << "|" << createTime << "|" << (acceptedList ? "1" : "0");
    return oss.str();
}

Announcement* Announcement::deserialize(const std::string& line) {
    std::istringstream ss(line);
    std::string idStr, t, c, a, ct, alStr;

    std::getline(ss, idStr, '|');
    std::getline(ss, t, '|');
    std::getline(ss, c, '|');
    std::getline(ss, a, '|');
    std::getline(ss, ct, '|');
    std::getline(ss, alStr, '|');

    if (idStr.empty() || t.empty()) return NULL;

    int i = 0;
    std::istringstream idStream(idStr);
    idStream >> i;

    bool al = (alStr == "1");

    return new Announcement(i, t, c, a, ct, al);
}
