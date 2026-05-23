#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <string>
#include <vector>

class Registration {
private:
    int id;
    std::string username;      // 报名学生用户名
    std::string programType;   // 节目类型
    int duration;              // 节目时长（分钟）
    std::string programName;   // 节目名称
    std::string description;   // 节目描述
    std::string performer;     // 报名人/表演者
    std::string status;        // 审核状态: pending/approved/rejected
    std::string reviewComment; // 审核意见
    std::string materials;     // 提交的材料信息

public:
    Registration();
    Registration(int i, const std::string& u, const std::string& pt,
                 int d, const std::string& pn, const std::string& desc,
                 const std::string& perf);

    // Getters
    int getId() const;
    std::string getUsername() const;
    std::string getProgramType() const;
    int getDuration() const;
    std::string getProgramName() const;
    std::string getDescription() const;
    std::string getPerformer() const;
    std::string getStatus() const;
    std::string getReviewComment() const;
    std::string getMaterials() const;

    // Setters
    void setProgramType(const std::string& pt);
    void setDuration(int d);
    void setProgramName(const std::string& pn);
    void setDescription(const std::string& desc);
    void setPerformer(const std::string& perf);
    void setStatus(const std::string& s);
    void setReviewComment(const std::string& rc);
    void setMaterials(const std::string& m);

    // 状态辅助
    bool isPending() const;
    bool isApproved() const;
    bool isRejected() const;
    std::string getStatusDisplay() const;

    // 序列化
    std::string serialize() const;
    static Registration* deserialize(const std::string& line);

    // 获取支持的节目类型列表
    static const std::vector<std::string>& getProgramTypes();
};

#endif
