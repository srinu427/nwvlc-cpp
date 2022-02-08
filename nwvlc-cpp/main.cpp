#include <windows.h>

#include <QtWidgets>
#include "PlayerScreen.h"

#include <iostream>
#include <fstream>




int main(int argc, char* argv[])
{
    QApplication app = QApplication(argc, argv);

    std::ifstream stfile("themes/styles.qss", std::ios::ate);

    if (!stfile.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)stfile.tellg();
    char* buffer = (char*)malloc(sizeof(char) * fileSize);

    stfile.seekg(0);
    stfile.read(buffer, fileSize);
    stfile.close();

    app.setStyleSheet(buffer);
    

    PlayerScreen player;
    player.show();
    player.setMinimumSize(640, 480);
    player.resize(960, 600);

    return app.exec();
}