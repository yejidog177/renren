#include "menu.h"
#include <iostream>
#include <sstream>
#include <limits>
#include <iomanip>
#include <cstdlib>

Menu::Menu(DataManager& dm) : dm(dm), currentUser(NULL) {}

std::string Menu::intToString(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

void Menu::clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

void Menu::pauseScreen() {
    std::cout << "\n按回车键继续...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

std::string Menu::readLine(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    if (std::cin.eof()) {
        std::cout << "\n检测到输入结束，程序退出。\n";
        exit(0);
    }
    return input;
}

int Menu::readInt(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (std::cin.eof()) {
            std::cout << "\n检测到输入结束，程序退出。\n";
            exit(0);
        }
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入无效，请输入数字。\n";
        } else if (value < min || value > max) {
            std::cout << "请输入 " << min << "-" << max << " 之间的数字。\n";
        } else {
            return value;
        }
    }
}

// ==================== 主菜单 ====================

void Menu::showMainMenu() {
    while (true) {
        clearScreen();
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════╗\n";
        std::cout << "║      我新我秀 报名系统          ║\n";
        std::cout << "╠══════════════════════════════════╣\n";
        std::cout << "║  1. 用户登录                     ║\n";
        std::cout << "║  2. 学生注册                     ║\n";
        std::cout << "║  0. 退出系统                     ║\n";
        std::cout << "╚══════════════════════════════════╝\n";

        int choice = readInt("\n请选择操作: ", 0, 2);

        switch (choice) {
            case 1: handleLogin(); break;
            case 2: handleRegister(); break;
            case 0: handleExit(); return;
        }
    }
}

void Menu::handleLogin() {
    clearScreen();
    std::cout << "\n========== 用户登录 ==========\n";

    std::string username = readLine("用户名: ");
    std::string password = readLine("密  码: ");

    currentUser = dm.authenticate(username, password);

    if (currentUser == NULL) {
        std::cout << "\n登录失败！用户名或密码错误。\n";
        pauseScreen();
        return;
    }

    std::cout << "\n登录成功！欢迎, " << currentUser->getUsername() << " (";
    std::cout << (currentUser->getRole() == "admin" ? "管理员" : "学生") << ")\n";
    pauseScreen();

    if (currentUser->getRole() == "admin") {
        showAdminMenu();
    } else {
        showStudentMenu();
    }
    currentUser = NULL;
}

void Menu::handleRegister() {
    clearScreen();
    std::cout << "\n========== 学生注册 ==========\n";

    // 用户名
    std::string username;
    while (true) {
        username = readLine("请输入用户名: ");
        if (username.empty()) {
            std::cout << "用户名不能为空！\n";
        } else if (username.find('|') != std::string::npos) {
            std::cout << "用户名不能包含特殊字符 '|'！\n";
        } else if (dm.userExists(username)) {
            std::cout << "该用户名已存在，请换一个。\n";
        } else {
            break;
        }
    }

    // 密码
    std::string password;
    while (true) {
        password = readLine("请输入密码 (至少4位): ");
        if (password.length() < 4) {
            std::cout << "密码至少需要4位字符！\n";
        } else if (password.find('|') != std::string::npos) {
            std::cout << "密码不能包含特殊字符 '|'！\n";
        } else {
            break;
        }
    }

    // 确认密码
    std::string password2;
    while (true) {
        password2 = readLine("请再次输入密码: ");
        if (password2 != password) {
            std::cout << "两次密码输入不一致，请重新输入。\n";
        } else {
            break;
        }
    }

    // 专业选择
    std::cout << "\n可选专业列表:\n";
    const std::vector<std::string>& majorList = dm.getMajors();
    for (size_t i = 0; i < majorList.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << majorList[i] << "\n";
    }
    std::cout << "  0. 返回主菜单\n";

    int majorChoice = readInt(
        "请选择专业 (0-" + intToString((int)majorList.size()) + "): ",
        0, (int)majorList.size());
    if (majorChoice == 0) return;

    std::string major = majorList[majorChoice - 1];

    // 班级选择
    const std::vector<std::string>& classList = dm.getClasses(major);
    if (classList.empty()) {
        std::cout << "该专业暂无可选班级，请联系管理员。\n";
        pauseScreen();
        return;
    }

    std::cout << "\n可选班级列表:\n";
    for (size_t i = 0; i < classList.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << classList[i] << "\n";
    }
    std::cout << "  0. 返回主菜单\n";

    int classChoice = readInt(
        "请选择班级 (0-" + intToString((int)classList.size()) + "): ",
        0, (int)classList.size());
    if (classChoice == 0) return;

    std::string className = classList[classChoice - 1];

    // 学号
    std::string studentId;
    while (true) {
        studentId = readLine("请输入学号: ");
        if (studentId.empty()) {
            std::cout << "学号不能为空！\n";
        } else if (studentId.find('|') != std::string::npos) {
            std::cout << "学号不能包含特殊字符 '|'！\n";
        } else {
            break;
        }
    }

    // 创建学生
    Student* student = new Student(username, password, major, className, studentId);
    if (dm.addUser(student)) {
        std::cout << "\n注册成功！请妥善保管您的账号信息。\n";
    } else {
        delete student;
        std::cout << "\n注册失败，请重试。\n";
    }
    pauseScreen();
}

void Menu::handleExit() {
    clearScreen();
    std::cout << "\n感谢使用\"我新我秀\"报名系统，再见！\n\n";
}

// ==================== 学生菜单 ====================

void Menu::showStudentMenu() {
    while (true) {
        clearScreen();
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════╗\n";
        std::cout << "║       学生操作界面              ║\n";
        std::cout << "╠══════════════════════════════════╣\n";
        std::string welcome = "║  欢迎, " + currentUser->getUsername();
        std::cout << welcome;
        for (int i = welcome.length(); i < 35; ++i) std::cout << " ";
        std::cout << "║\n";
        std::cout << "╠══════════════════════════════════╣\n";
        std::cout << "║  1. 查看公告 & 入选名单         ║\n";
        std::cout << "║  2. 编辑报名节目信息            ║\n";
        std::cout << "║  3. 提交材料                    ║\n";
        std::cout << "║  4. 查看个人信息                ║\n";
        std::cout << "║  5. 修改密码                    ║\n";
        std::cout << "║  0. 退出登录                    ║\n";
        std::cout << "╚══════════════════════════════════╝\n";

        int choice = readInt("\n请选择操作: ", 0, 5);

        switch (choice) {
            case 1: handleStudentAnnouncements(); break;
            case 2: handleStudentRegistration(); break;
            case 3: handleStudentMaterials(); break;
            case 4: handleStudentInfo(); break;
            case 5: handleChangePassword(); break;
            case 0: return;
        }
    }
}

void Menu::handleStudentAnnouncements() {
    clearScreen();
    std::cout << "\n========== 公告 & 入选名单 ==========\n";

    while (true) {
        std::cout << "\n  1. 查看比赛公告\n";
        std::cout << "  2. 查看到最终入选名单\n";
        std::cout << "  0. 返回\n";

        int choice = readInt("\n请选择: ", 0, 2);
        if (choice == 0) return;

        if (choice == 1) {
            // 查看普通公告
            clearScreen();
            std::cout << "\n========== 比赛公告 ==========\n";
            std::vector<Announcement*> general = dm.getGeneralAnnouncements();
            if (general.empty()) {
                std::cout << "\n暂无公告。\n";
            } else {
                for (size_t i = 0; i < general.size(); ++i) {
                    std::cout << "\n────────────────────────────────\n";
                    std::cout << "标题: " << general[i]->getTitle() << "\n";
                    std::cout << "发布者: " << general[i]->getAuthor() << "\n";
                    std::cout << "时间: " << general[i]->getCreateTime() << "\n";
                    std::cout << "内容:\n" << general[i]->getContent() << "\n";
                    std::cout << "────────────────────────────────\n";
                }
            }
        } else {
            // 查看入选名单
            clearScreen();
            std::cout << "\n========== 最终入选名单 ==========\n";

            // 1. 显示已通过审核的报名
            const std::vector<Registration*>& allRegs = dm.getAllRegistrations();
            bool hasApproved = false;
            std::cout << "\n--- 已通过审核的节目 ---\n";
            for (size_t i = 0; i < allRegs.size(); ++i) {
                if (allRegs[i]->isApproved()) {
                    hasApproved = true;
                    std::cout << "\n  * 节目: " << allRegs[i]->getProgramName()
                              << " | 类型: " << allRegs[i]->getProgramType()
                              << " | 表演者: " << allRegs[i]->getPerformer()
                              << " | 报名人: " << allRegs[i]->getUsername() << "\n";
                }
            }
            if (!hasApproved) {
                std::cout << "（暂无）\n";
            }

            // 2. 显示管理员发布的入选名单公告
            std::cout << "\n--- 管理员公布的入选名单 ---\n";
            std::vector<Announcement*> accepted = dm.getAcceptedLists();
            if (accepted.empty()) {
                std::cout << "（暂无）\n";
            } else {
                for (size_t i = 0; i < accepted.size(); ++i) {
                    std::cout << "\n────────────────────────────────\n";
                    std::cout << "标题: " << accepted[i]->getTitle() << "\n";
                    std::cout << "发布者: " << accepted[i]->getAuthor() << "\n";
                    std::cout << "时间: " << accepted[i]->getCreateTime() << "\n";
                    std::cout << "内容:\n" << accepted[i]->getContent() << "\n";
                    std::cout << "────────────────────────────────\n";
                }
            }
        }
        pauseScreen();
    }
}

void Menu::handleStudentRegistration() {
    while (true) {
        clearScreen();
        std::cout << "\n========== 编辑报名节目信息 ==========\n";

        // 查找已有报名
        Registration* reg = dm.getRegistrationByUser(currentUser->getUsername());

        if (reg != NULL) {
            std::cout << "\n当前报名状态: " << reg->getStatusDisplay() << "\n";
            std::cout << "\n========== 当前报名信息 ==========\n";
            std::cout << "节目名称: " << reg->getProgramName() << "\n";
            std::cout << "节目类型: " << reg->getProgramType() << "\n";
            std::cout << "节目时长: " << reg->getDuration() << " 分钟\n";
            std::cout << "表 演 者: " << reg->getPerformer() << "\n";
            std::cout << "节目描述: " << reg->getDescription() << "\n";
            std::cout << "==================================\n";

            if (reg->isApproved()) {
                std::cout << "\n您的报名已通过审核，无法修改。\n";
                pauseScreen();
                return;
            }
            if (reg->isRejected()) {
                std::cout << "\n审核意见: " << reg->getReviewComment() << "\n";
            }

            std::cout << "\n  1. 修改报名信息\n";
            std::cout << "  0. 返回\n";
            int choice = readInt("\n请选择: ", 0, 1);
            if (choice == 0) return;
        }

        // 新建或修改报名
        if (reg == NULL) {
            std::cout << "\n您还没有提交报名，请填写以下信息。\n";
        }

        // 节目类型选择
        const std::vector<std::string>& types = Registration::getProgramTypes();
        std::cout << "\n可选节目类型:\n";
        for (size_t i = 0; i < types.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << types[i] << "\n";
        }
        int typeChoice = readInt("请选择节目类型 (1-" + intToString((int)types.size()) + "): ",
                                 1, (int)types.size());
        std::string programType = types[typeChoice - 1];

        // 节目名称
        std::string programName;
        while (true) {
            programName = readLine("请输入节目名称: ");
            if (programName.empty()) {
                std::cout << "节目名称不能为空！\n";
            } else if (programName.find('|') != std::string::npos) {
                std::cout << "节目名称不能包含特殊字符 '|'！\n";
            } else {
                break;
            }
        }

        // 节目时长
        int duration = readInt("请输入节目时长 (分钟, 1-30): ", 1, 30);

        // 报名人/表演者
        std::string defaultPerformer = (reg != NULL) ? reg->getPerformer() : currentUser->getUsername();
        std::cout << "请输入表演者姓名 (直接回车使用 \"" << defaultPerformer << "\"): ";
        std::string performer = readLine("");
        if (performer.empty()) {
            performer = defaultPerformer;
        }

        // 节目描述
        std::cout << "请输入节目描述（输入 END 结束）:\n";
        std::string desc, descLine;
        while (true) {
            std::getline(std::cin, descLine);
            if (std::cin.eof()) { std::cout << "\n检测到输入结束，程序退出。\n"; exit(0); }
            if (descLine == "END") break;
            if (!desc.empty()) desc += "\n";
            desc += descLine;
        }

        // 保存
        Registration* newReg = new Registration();
        newReg->setPerformer(performer);  // 用于识别用户
        newReg->setProgramType(programType);
        newReg->setDuration(duration);
        newReg->setProgramName(programName);
        newReg->setDescription(desc);

        if (reg != NULL) {
            newReg = new Registration(reg->getId(), reg->getUsername(),
                                       programType, duration, programName,
                                       desc, performer);
        } else {
            newReg = new Registration(0, currentUser->getUsername(),
                                       programType, duration, programName,
                                       desc, performer);
        }

        if (dm.saveRegistration(newReg)) {
            std::cout << "\n报名信息保存成功！\n";
        } else {
            std::cout << "\n保存失败，请重试。\n";
        }
        pauseScreen();
        return;
    }
}

void Menu::handleStudentMaterials() {
    clearScreen();
    std::cout << "\n========== 提交材料 ==========\n";

    Registration* reg = dm.getRegistrationByUser(currentUser->getUsername());

    if (reg == NULL) {
        std::cout << "\n请先完成节目报名后再提交材料。\n";
        pauseScreen();
        return;
    }

    if (reg->isApproved()) {
        std::cout << "\n您的报名已通过审核，材料已锁定。\n";
        std::cout << "\n当前材料内容:\n";
        std::cout << (reg->getMaterials().empty() ? "（无）" : reg->getMaterials()) << "\n";
        pauseScreen();
        return;
    }

    // 显示当前材料
    std::cout << "\n当前已提交的材料:\n";
    std::cout << "────────────────────────────────\n";
    if (reg->getMaterials().empty()) {
        std::cout << "（尚未提交材料）\n";
    } else {
        std::cout << reg->getMaterials() << "\n";
    }
    std::cout << "────────────────────────────────\n";

    std::cout << "\n请按以下格式填写材料信息:\n";
    std::cout << "  背景音乐: <文件路径或说明>\n";
    std::cout << "  歌词/剧本: <文件路径或说明>\n";
    std::cout << "  道具清单: <列表>\n";
    std::cout << "  其他材料: <说明>\n";
    std::cout << "\n请输入材料信息（输入 END 结束，空内容将清空材料）:\n";

    std::string newMaterials, line;
    while (true) {
        std::getline(std::cin, line);
        if (std::cin.eof()) { std::cout << "\n检测到输入结束，程序退出。\n"; exit(0); }
        if (line == "END") break;
        if (!newMaterials.empty()) newMaterials += "\n";
        newMaterials += line;
    }

    reg->setMaterials(newMaterials);
    dm.saveAll();
    std::cout << "\n材料提交成功！\n";
    pauseScreen();
}

void Menu::handleStudentInfo() {
    clearScreen();
    Student* student = dynamic_cast<Student*>(currentUser);
    if (student != NULL) {
        student->displayInfo();
    } else {
        std::cout << "\n无法获取学生信息。\n";
    }
    pauseScreen();
}

void Menu::handleChangePassword() {
    clearScreen();
    std::cout << "\n========== 修改密码 ==========\n";

    std::string oldPassword = readLine("请输入当前密码: ");
    if (!currentUser->checkPassword(oldPassword)) {
        std::cout << "\n当前密码错误！\n";
        pauseScreen();
        return;
    }

    std::string newPassword;
    while (true) {
        newPassword = readLine("请输入新密码 (至少4位): ");
        if (newPassword.length() < 4) {
            std::cout << "密码至少需要4位字符！\n";
        } else if (newPassword.find('|') != std::string::npos) {
            std::cout << "密码不能包含特殊字符 '|'！\n";
        } else {
            break;
        }
    }

    std::string newPassword2 = readLine("请再次输入新密码: ");
    if (newPassword2 != newPassword) {
        std::cout << "\n两次密码输入不一致！\n";
        pauseScreen();
        return;
    }

    currentUser->setPassword(newPassword);
    dm.saveAll();
    std::cout << "\n密码修改成功！\n";
    pauseScreen();
}

// ==================== 管理员菜单 ====================

void Menu::showAdminMenu() {
    while (true) {
        clearScreen();
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════╗\n";
        std::cout << "║       管理员操作界面            ║\n";
        std::cout << "╠══════════════════════════════════╣\n";
        std::string welcome = "║  欢迎, " + currentUser->getUsername();
        std::cout << welcome;
        for (int i = welcome.length(); i < 35; ++i) std::cout << " ";
        std::cout << "║\n";
        std::cout << "╠══════════════════════════════════╣\n";
        std::cout << "║  1. 编辑和发送公告              ║\n";
        std::cout << "║  2. 审核报名                    ║\n";
        std::cout << "║  3. 修改密码                    ║\n";
        std::cout << "║  0. 退出登录                    ║\n";
        std::cout << "╚══════════════════════════════════╝\n";

        int choice = readInt("\n请选择操作: ", 0, 3);

        switch (choice) {
            case 1: handleAdminAnnouncements(); break;
            case 2: handleAdminReview(); break;
            case 3: handleChangePassword(); break;
            case 0: return;
        }
    }
}

void Menu::handleAdminAnnouncements() {
    while (true) {
        clearScreen();
        std::cout << "\n========== 公告管理 ==========\n";
        std::cout << "\n  1. 查看所有公告\n";
        std::cout << "  2. 发布新公告\n";
        std::cout << "  3. 修改公告\n";
        std::cout << "  4. 删除公告\n";
        std::cout << "  0. 返回\n";

        int choice = readInt("\n请选择操作: ", 0, 4);

        switch (choice) {
            case 1: {
                // 查看所有公告
                clearScreen();
                std::cout << "\n========== 所有公告 ==========\n";
                const std::vector<Announcement*>& all = dm.getAnnouncements();
                if (all.empty()) {
                    std::cout << "\n暂无公告。\n";
                } else {
                    for (size_t i = 0; i < all.size(); ++i) {
                        std::cout << "\n────────────────────────────────\n";
                        std::cout << "ID: " << all[i]->getId();
                        std::cout << "  类型: " << (all[i]->isAcceptedList() ? "[入选名单]" : "[普通公告]") << "\n";
                        std::cout << "标题: " << all[i]->getTitle() << "\n";
                        std::cout << "发布者: " << all[i]->getAuthor() << "\n";
                        std::cout << "时间: " << all[i]->getCreateTime() << "\n";
                        std::cout << "内容:\n" << all[i]->getContent() << "\n";
                        std::cout << "────────────────────────────────\n";
                    }
                }
                pauseScreen();
                break;
            }
            case 2: {
                // 发布新公告
                clearScreen();
                std::cout << "\n========== 发布新公告 ==========\n";

                std::string title = readLine("请输入公告标题: ");
                if (title.empty()) {
                    std::cout << "标题不能为空！\n";
                    pauseScreen();
                    break;
                }

                std::cout << "请输入公告内容（输入 END 结束）:\n";
                std::string content, line;
                while (true) {
                    std::getline(std::cin, line);
                    if (std::cin.eof()) {
                        std::cout << "\n检测到输入结束，程序退出。\n";
                        exit(0);
                    }
                    if (line == "END") break;
                    if (!content.empty()) content += "\n";
                    content += line;
                }

                int typeChoice = readInt("\n公告类型 (1-普通公告, 2-入选名单): ", 1, 2);
                bool isAccepted = (typeChoice == 2);

                if (dm.addAnnouncement(title, content, currentUser->getUsername(), isAccepted)) {
                    std::cout << "\n公告发布成功！\n";
                } else {
                    std::cout << "\n公告发布失败！\n";
                }
                pauseScreen();
                break;
            }
            case 3: {
                // 修改公告
                clearScreen();
                std::cout << "\n========== 修改公告 ==========\n";
                const std::vector<Announcement*>& all = dm.getAnnouncements();
                if (all.empty()) {
                    std::cout << "\n暂无公告可修改。\n";
                    pauseScreen();
                    break;
                }
                for (size_t i = 0; i < all.size(); ++i) {
                    std::cout << "  " << all[i]->getId() << ". " << all[i]->getTitle()
                              << (all[i]->isAcceptedList() ? " [入选名单]" : "") << "\n";
                }
                std::cout << "  0. 返回\n";

                int editId = readInt("\n请选择要修改的公告ID: ", 0, 99999);
                if (editId == 0) break;

                Announcement* target = dm.findAnnouncement(editId);
                if (target == NULL) {
                    std::cout << "未找到该公告！\n";
                    pauseScreen();
                    break;
                }

                std::string newTitle = readLine("新标题 (直接回车保留原标题): ");
                if (newTitle.empty()) newTitle = target->getTitle();

                std::cout << "新内容 (直接输入 END 保留原内容，输入新内容后以 END 结束):\n";
                std::string newContent, newLine;
                std::getline(std::cin, newLine);
                if (std::cin.eof()) { std::cout << "\n检测到输入结束，程序退出。\n"; exit(0); }
                if (newLine == "END") {
                    newContent = target->getContent();
                } else {
                    newContent = newLine;
                    while (true) {
                        std::getline(std::cin, newLine);
                        if (std::cin.eof()) { std::cout << "\n检测到输入结束，程序退出。\n"; exit(0); }
                        if (newLine == "END") break;
                        newContent += "\n" + newLine;
                    }
                }

                int typeChoice = readInt("公告类型 (1-普通公告, 2-入选名单): ", 1, 2);
                bool isAccepted = (typeChoice == 2);

                if (dm.updateAnnouncement(editId, newTitle, newContent, isAccepted)) {
                    std::cout << "\n公告修改成功！\n";
                } else {
                    std::cout << "\n公告修改失败！\n";
                }
                pauseScreen();
                break;
            }
            case 4: {
                // 删除公告
                clearScreen();
                std::cout << "\n========== 删除公告 ==========\n";
                const std::vector<Announcement*>& all = dm.getAnnouncements();
                if (all.empty()) {
                    std::cout << "\n暂无公告可删除。\n";
                    pauseScreen();
                    break;
                }
                for (size_t i = 0; i < all.size(); ++i) {
                    std::cout << "  " << all[i]->getId() << ". " << all[i]->getTitle()
                              << (all[i]->isAcceptedList() ? " [入选名单]" : "") << "\n";
                }
                std::cout << "  0. 返回\n";

                int delId = readInt("\n请选择要删除的公告ID: ", 0, 99999);
                if (delId == 0) break;

                Announcement* target = dm.findAnnouncement(delId);
                if (target == NULL) {
                    std::cout << "\n未找到该公告！\n";
                    pauseScreen();
                    break;
                }
                std::cout << "\n确认删除公告 \"" << target->getTitle() << "\" ? (1-确认, 0-取消): ";
                int confirm = readInt("", 0, 1);
                if (confirm == 0) break;

                if (dm.deleteAnnouncement(delId)) {
                    std::cout << "\n公告删除成功！\n";
                } else {
                    std::cout << "\n公告删除失败！\n";
                }
                pauseScreen();
                break;
            }
            case 0:
                return;
        }
    }
}

void Menu::handleAdminReview() {
    while (true) {
        clearScreen();
        std::cout << "\n========== 审核报名 ==========\n";
        const std::vector<Registration*>& all = dm.getAllRegistrations();

        if (all.empty()) {
            std::cout << "\n暂无报名记录。\n";
            pauseScreen();
            return;
        }

        // 统计
        int pendingCount = 0, approvedCount = 0, rejectedCount = 0;
        for (size_t i = 0; i < all.size(); ++i) {
            if (all[i]->isPending()) pendingCount++;
            else if (all[i]->isApproved()) approvedCount++;
            else rejectedCount++;
        }

        std::cout << "\n报名统计: 总计 " << all.size() << " | 待审核 " << pendingCount
                  << " | 已通过 " << approvedCount << " | 未通过 " << rejectedCount << "\n";

        std::cout << "\n报名列表:\n";
        std::cout << "────────────────────────────────────────────\n";
        for (size_t i = 0; i < all.size(); ++i) {
            std::cout << "ID:" << all[i]->getId()
                      << " | 用户:" << all[i]->getUsername()
                      << " | 节目:" << all[i]->getProgramName()
                      << " | 状态:" << all[i]->getStatusDisplay() << "\n";
        }
        std::cout << "────────────────────────────────────────────\n";

        std::cout << "\n  1. 查看报名详情并审核\n";
        std::cout << "  0. 返回\n";

        int choice = readInt("\n请选择: ", 0, 1);
        if (choice == 0) return;

        // 查看详情并审核
        int regId = readInt("请输入要审核的报名ID: ", 1, 99999);
        Registration* reg = dm.findRegistration(regId);
        if (reg == NULL) {
            std::cout << "\n未找到该报名！\n";
            pauseScreen();
            continue;
        }

        // 显示详情
        clearScreen();
        std::cout << "\n========== 报名详情 ==========\n";
        std::cout << "报名ID: " << reg->getId() << "\n";
        std::cout << "报名用户: " << reg->getUsername() << "\n";

        // 查找学生信息
        User* student = dm.findUser(reg->getUsername());
        Student* stu = dynamic_cast<Student*>(student);
        if (stu != NULL) {
            std::cout << "专    业: " << stu->getMajor() << "\n";
            std::cout << "班    级: " << stu->getClassName() << "\n";
            std::cout << "学    号: " << stu->getStudentId() << "\n";
        }

        std::cout << "节目名称: " << reg->getProgramName() << "\n";
        std::cout << "节目类型: " << reg->getProgramType() << "\n";
        std::cout << "节目时长: " << reg->getDuration() << " 分钟\n";
        std::cout << "表 演 者: " << reg->getPerformer() << "\n";
        std::cout << "节目描述:\n" << reg->getDescription() << "\n";
        std::cout << "────────────────────────────────\n";
        std::cout << "提交材料:\n";
        std::cout << (reg->getMaterials().empty() ? "（未提交材料）" : reg->getMaterials()) << "\n";
        std::cout << "────────────────────────────────\n";
        std::cout << "当前状态: " << reg->getStatusDisplay() << "\n";
        if (!reg->getReviewComment().empty()) {
            std::cout << "审核意见: " << reg->getReviewComment() << "\n";
        }
        std::cout << "================================\n";

        // 审核操作
        std::cout << "\n审核操作:\n";
        std::cout << "  1. 通过\n";
        std::cout << "  2. 驳回\n";
        if (reg->isApproved() || reg->isRejected()) {
            std::cout << "  3. 撤销审核（重置为待审核）\n";
        }
        std::cout << "  0. 返回\n";

        int maxOption = (reg->isApproved() || reg->isRejected()) ? 3 : 2;
        int action = readInt("\n请选择: ", 0, maxOption);
        if (action == 0) continue;

        if (action == 3) {
            reg->setStatus("pending");
            reg->setReviewComment("");
            std::cout << "\n已撤销审核，该报名重置为待审核状态。\n";
            dm.saveAll();
            pauseScreen();
            continue;
        }

        std::string comment;
        if (action == 2) {
            comment = readLine("请输入驳回原因: ");
            if (comment.empty()) {
                comment = "未通过审核";
            }
        }

        if (action == 1) {
            reg->setStatus("approved");
            reg->setReviewComment("审核通过");
            std::cout << "\n已通过该报名！\n";
        } else {
            reg->setStatus("rejected");
            reg->setReviewComment(comment);
            std::cout << "\n已驳回该报名！\n";
        }

        dm.saveAll();
        pauseScreen();
    }
}

// ==================== 运行 ====================

void Menu::run() {
    showMainMenu();
}
