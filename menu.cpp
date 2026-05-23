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
            std::vector<Announcement*> accepted = dm.getAcceptedLists();
            if (accepted.empty()) {
                std::cout << "\n暂未公布入选名单。\n";
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
    clearScreen();
    std::cout << "\n========== 编辑报名节目信息 ==========\n";
    std::cout << "\n此功能正在开发中，敬请期待...\n";
    pauseScreen();
}

void Menu::handleStudentMaterials() {
    clearScreen();
    std::cout << "\n========== 提交材料 ==========\n";
    std::cout << "\n此功能正在开发中，敬请期待...\n";
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
                if (newLine == "END") {
                    newContent = target->getContent();
                } else {
                    newContent = newLine;
                    while (true) {
                        std::getline(std::cin, newLine);
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

                if (dm.deleteAnnouncement(delId)) {
                    std::cout << "\n公告删除成功！\n";
                } else {
                    std::cout << "\n未找到该公告！\n";
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
    clearScreen();
    std::cout << "\n========== 审核报名 ==========\n";
    std::cout << "\n此功能正在开发中，敬请期待...\n";
    pauseScreen();
}

// ==================== 运行 ====================

void Menu::run() {
    showMainMenu();
}
