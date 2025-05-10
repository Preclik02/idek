#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <conio.h>
#include <windows.h>

void hideCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;  // Hide the thick cursor
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// Move cursor to a specific position
void moveCursor(int row, int col) {
    COORD coord = { static_cast<SHORT>(col), static_cast<SHORT>(row) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Clear screen
void clearScreen() {
    system("cls");
}

// Set console screen buffer size to allow long lines
void setupConsole() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(hOut, &info);
    COORD newSize = { 200, info.dwSize.Y };
    SetConsoleScreenBufferSize(hOut, newSize);
}

// Load a file into the buffer; create a new one if not found
void loadFile(const std::string& filename, std::vector<std::string>& buffer) {
    std::ifstream file(filename);
    std::string line;
    if (file.is_open()) {
        while (std::getline(file, line)) {
            buffer.push_back(line);
        }
        file.close();
    }
    if (buffer.empty()) {
        buffer.push_back("");
    }
}

// Save buffer content to file
void saveFile(const std::string& filename, const std::vector<std::string>& buffer) {
    std::ofstream file(filename);
    for (const auto& line : buffer) {
        file << line << "\n";
    }
    file.close();
}

// Compile and run the file (Windows only)
void compileAndRun(const std::string& filename) {
    std::string command = "g++ \"" + filename + "\" -o temp.exe && temp.exe";
    system(command.c_str());
}

// Render buffer and place cursor, with scrolling
void render(const std::vector<std::string>& buffer, int cursorRow, int cursorCol, int scrollOffset, int viewportHeight) {
    clearScreen();
    int endLine = std::min(scrollOffset + viewportHeight, static_cast<int>(buffer.size()));
    for (int i = scrollOffset; i < endLine; ++i) {
        if (i == cursorRow) {
            std::cout << "\033[1;32m" << buffer[i].substr(0, cursorCol)
                      << "\033[1;31m" << buffer[i].substr(cursorCol)
                      << "\033[0m" << std::endl;
        } else {
            std::cout << buffer[i] << std::endl;
        }
    }
    moveCursor(cursorRow - scrollOffset, cursorCol);
}

int main() {
    setupConsole();
    hideCursor();

    std::string filename;
    std::cout << "Filename: ";
    std::getline(std::cin, filename);

    std::vector<std::string> buffer;
    loadFile(filename, buffer);

    int row = 0;
    int col = buffer[row].size();

    int viewportHeight = 25;
    int scrollOffset = 0;

    render(buffer, row, col, scrollOffset, viewportHeight);

    while (true) {
        int ch = _getch();

        if (ch == 27) { // ESC key
            saveFile(filename, buffer);
            clearScreen();
            std::cout << "Saved. Compiling...\n\n";
            compileAndRun(filename);
            std::cout << "\n\nDone. Press any key to exit.\n";
            _getch();
            system("cls");
            break;
        } else if (ch == 0 || ch == 224) { // Arrow keys
            int arrow = _getch();
            switch (arrow) {
                case 72: if (row > 0) row--; break; // Up
                case 80: if (row < buffer.size() - 1) row++; break; // Down
                case 75: if (col > 0) col--; break; // Left
                case 77: if (col < buffer[row].size()) col++; break; // Right
            }

            // Adjust scroll only if near top or bottom of viewport
            if (row < scrollOffset) scrollOffset = row;
            if (row >= scrollOffset + viewportHeight) scrollOffset = row - viewportHeight + 1;

        } else if (ch == 8) { // Backspace
            if (col > 0) {
                buffer[row].erase(col - 1, 1);
                col--;
            } else if (row > 0) {
                col = buffer[row - 1].size();
                buffer[row - 1] += buffer[row];
                buffer.erase(buffer.begin() + row);
                row--;
            }
        } else if (ch == '\r') { // Enter
            std::string newLine = buffer[row].substr(col);
            buffer[row] = buffer[row].substr(0, col);
            buffer.insert(buffer.begin() + row + 1, newLine);
            row++;
            col = 0;
        } else if (isprint(ch)) { // Printable characters
            buffer[row].insert(buffer[row].begin() + col, ch);
            col++;
        }

        // Bounds check
        if (row >= buffer.size()) row = buffer.size() - 1;
        if (col > buffer[row].size()) col = buffer[row].size();

        render(buffer, row, col, scrollOffset, viewportHeight);
    }

    return 0;
}
