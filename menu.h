#ifndef MENU_H
#define MENU_H

#include "datamanager.h"
#include <string>

class Menu {
private:
    DataManager& dm;
    User* currentUser;

    void showMainMenu();
    void handleLogin();
    void handleRegister();
    void handleExit();

    void showStudentMenu();
    void handleStudentAnnouncements();
    void handleStudentRegistration();
    void handleStudentMaterials();

    void showAdminMenu();
    void handleAdminAnnouncements();
    void handleAdminReview();

    void clearScreen();
    void pauseScreen();
    std::string readLine(const std::string& prompt);
    int readInt(const std::string& prompt, int min, int max);
    std::string intToString(int n);

public:
    Menu(DataManager& dm);
    void run();
};

#endif
