#include "registration.h"
#include <sstream>

Registration::Registration()
    : id(0), username(""), programType(""), duration(0),
      programName(""), description(""), performer(""),
      status("pending"), reviewComment(""), materials("") {}

Registration::Registration(int i, const std::string& u, const std::string& pt,
                           int d, const std::string& pn, const std::string& desc,
                           const std::string& perf)
    : id(i), username(u), programType(pt), duration(d),
      programName(pn), description(desc), performer(perf),
      status("pending"), reviewComment(""), materials("") {}

int Registration::getId() const { return id; }
std::string Registration::getUsername() const { return username; }
std::string Registration::getProgramType() const { return programType; }
int Registration::getDuration() const { return duration; }
std::string Registration::getProgramName() const { return programName; }
std::string Registration::getDescription() const { return description; }
std::string Registration::getPerformer() const { return performer; }
std::string Registration::getStatus() const { return status; }
std::string Registration::getReviewComment() const { return reviewComment; }

void Registration::setProgramType(const std::string& pt) { programType = pt; }
void Registration::setDuration(int d) { duration = d; }
void Registration::setProgramName(const std::string& pn) { programName = pn; }
void Registration::setDescription(const std::string& desc) { description = desc; }
void Registration::setPerformer(const std::string& perf) { performer = perf; }
void Registration::setStatus(const std::string& s) { status = s; }
void Registration::setReviewComment(const std::string& rc) { reviewComment = rc; }
void Registration::setMaterials(const std::string& m) { materials = m; }
std::string Registration::getMaterials() const { return materials; }

bool Registration::isPending() const { return status == "pending"; }
bool Registration::isApproved() const { return status == "approved"; }
bool Registration::isRejected() const { return status == "rejected"; }

std::string Registration::getStatusDisplay() const {
    if (status == "approved") return "已通过";
    if (status == "rejected") return "未通过";
    return "待审核";
}

std::string Registration::serialize() const {
    std::ostringstream oss;
    oss << id << "|" << username << "|" << programType << "|"
        << duration << "|" << programName << "|" << description << "|"
        << performer << "|" << status << "|" << reviewComment << "|" << materials;
    return oss.str();
}

Registration* Registration::deserialize(const std::string& line) {
    std::istringstream ss(line);
    std::string idStr, u, pt, durStr, pn, desc, perf, st, rc, mat;

    std::getline(ss, idStr, '|');
    std::getline(ss, u, '|');
    std::getline(ss, pt, '|');
    std::getline(ss, durStr, '|');
    std::getline(ss, pn, '|');
    std::getline(ss, desc, '|');
    std::getline(ss, perf, '|');
    std::getline(ss, st, '|');
    std::getline(ss, rc, '|');
    std::getline(ss, mat, '|');

    if (idStr.empty() || u.empty()) return NULL;

    int i = 0, d = 0;
    std::istringstream(idStr) >> i;
    std::istringstream(durStr) >> d;

    Registration* r = new Registration(i, u, pt, d, pn, desc, perf);
    r->setStatus(st.empty() ? "pending" : st);
    r->setReviewComment(rc);
    r->setMaterials(mat);
    return r;
}

const std::vector<std::string>& Registration::getProgramTypes() {
    static std::vector<std::string> types;
    if (types.empty()) {
        types.push_back("歌曲");
        types.push_back("舞蹈");
        types.push_back("小品");
        types.push_back("相声");
        types.push_back("乐器演奏");
        types.push_back("魔术");
        types.push_back("朗诵");
        types.push_back("武术");
        types.push_back("戏剧");
        types.push_back("其他");
    }
    return types;
}
