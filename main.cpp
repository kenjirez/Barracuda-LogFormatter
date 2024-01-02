#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <map>
#include <windows.h>
#include <filesystem>
#include <thread>
#include <chrono>

bool isValidLogEntry(const std::string& logEntry)
{
    std::regex logEntryRegex(R"(\b\w{3} \d{1,2} \d{2}:\d{2}:\d{2} Allow: .*)");

    return std::regex_match(logEntry, logEntryRegex);
}

std::string extractIP(const std::string& logEntry)
{
    std::regex ipRegex("\\d+\\.\\d+\\.\\d+\\.\\d+");

    std::smatch match;
    if (std::regex_search(logEntry, match, ipRegex))
    {
        return match.str();
    }

    return "";
}

int main()
{
    std::cout << "Please select your log file!" << std::endl;

    OPENFILENAMEA ofn;
    CHAR szFile[260];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        std::ifstream inputFile(ofn.lpstrFile);
        if (!inputFile.is_open())
        {
            std::cerr << "Error opening the file." << std::endl;
            return 1;
        }

        system("cls");

        std::string line;

        std::map<std::string, int> ipList;

        while (std::getline(inputFile, line))
        {
            if (!isValidLogEntry(line)) {
                continue;
            }

            std::istringstream iss(line);
            std::string month, day, time, ignore;
            iss >> month >> day >> time >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore;

            std::string ip = extractIP(line);

            std::string timestamp = time.substr(0, time.size() - 3);

            std::ostringstream oss;
            oss << month << " " << day << " " << timestamp << " - " << ip;

            if (!ip.empty() && ipList.find(oss.str()) != ipList.end()) {
                ipList[oss.str()]++;
            }
            else if (!ip.empty()) {
                ipList[oss.str()] = 1;
            }
            std::cout << "Added Key: " << oss.str() << std::endl;
        }

        system("cls");

        if (ipList.empty()) {
            std::cout << "Error Reading File. Are you sure you selected the right one?" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            return -1;
        }

        system("cls");

        std::cout << "Please create the output file!" << std::endl;

        OPENFILENAMEA outputOfn;
        CHAR outputSzFile[260];

        ZeroMemory(&outputOfn, sizeof(outputOfn));
        outputOfn.lStructSize = sizeof(outputOfn);
        outputOfn.hwndOwner = NULL;
        outputOfn.lpstrFile = outputSzFile;
        outputOfn.lpstrFile[0] = '\0';
        outputOfn.nMaxFile = sizeof(outputSzFile);
        outputOfn.lpstrFilter = "CSV Files (*.csv)\0*.csv\0Text Files (*.txt)\0*.txt\0";
        outputOfn.nFilterIndex = 1;
        outputOfn.nMaxFileTitle = 0;
        outputOfn.lpstrInitialDir = NULL;
        outputOfn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

        if (GetSaveFileNameA(&outputOfn) == TRUE)
        {
            std::string outputFile(outputOfn.lpstrFile);

            std::filesystem::path filePath(outputFile);
            if (filePath.extension().empty()) {
                int filterIndex = outputOfn.nFilterIndex;
                std::string defaultExtension = (filterIndex == 1) ? ".csv" : ".txt";
                outputFile += defaultExtension;
            }

            std::ofstream outfile(outputFile);

            system("cls");

            std::cout << "Please enter the amount of trys that an IP Address can have in a Minute (>=1):" << std::endl;
            int timesPerMinute;
            std::cin >> timesPerMinute;

            for (const auto& entry : ipList)
            {
                if (entry.second >= timesPerMinute)
                {
                    std::string month, day, timestamp, ip;

                    std::istringstream iss(entry.first);

                    iss >> month >> day >> timestamp;
                    std::getline(iss, ip);
                    ip = ip.substr(3);

                    std::cout << "Added Line: " << month << ";" << day << ";" << timestamp << ";" << ip << ";" << entry.second << ";" << std::endl;
                    outfile << month << ";" << day << ";" << timestamp << ";" << ip << ";" << entry.second << ";" << std::endl;
                }
            }
        }
        else {
            std::cerr << "User canceled the operation." << std::endl;
            return 1;
        }

        inputFile.close();
    }
    else {
        std::cerr << "User canceled the operation." << std::endl;
        return 1;
    }

    return 0;
}
