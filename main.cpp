#include "menu.h"

int main() {
    DataManager dm("data/");
    Menu menu(dm);
    menu.run();
    return 0;
}
