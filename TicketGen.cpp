#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <random>
#include <chrono>
#include <vector>
#include <ctime>
#include <cstdlib>

// Function to generate a random GUID-like string
std::string GenerateGUID() {
    std::stringstream ss;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    ss << std::hex << std::uppercase;
    for (int i = 0; i < 8; ++i) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; ++i) ss << dis(gen);
    ss << "-4"; // version 4
    for (int i = 0; i < 3; ++i) ss << dis(gen);
    ss << "-";
    ss << dis2(gen); // variant
    for (int i = 0; i < 3; ++i) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; ++i) ss << dis(gen);

    return ss.str();
}

int main() {
    const int numRequests = 100;

    std::mt19937 gen(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<int> priorityDist(1, 4);
    std::uniform_int_distribution<int> statusDist(1, 6);
    std::uniform_int_distribution<int> quantityDist(1, 5);
    std::uniform_int_distribution<int> timeOffsetDist(0, 100000);

    std::vector<std::string> qualities = {"any","500-600", "600-700","700-800", "800-900", "900+"};
    std::vector<std::string> workerIds = {"464542267395538944","332728115430162444", "710847331871883294","195997205864120320"};

    std::time_t now = std::time(nullptr);

    std::cout << "<Requests>\n";

    for (int i = 0; i < numRequests; ++i) {
        std::time_t created = now - timeOffsetDist(gen);
        std::time_t lastEdit = created + timeOffsetDist(gen) / 10;
        std::string guid = GenerateGUID();
        int priority = priorityDist(gen);
        int status = statusDist(gen);
        long long requester = 464542267395538944;
        std::string gameHandle = "aimx_optional";
        std::string description = "request " + std::to_string(i);
        int quantity = quantityDist(gen);
        std::string quality = qualities[i % qualities.size()];
        std::string worker = workerIds[i % workerIds.size()];

        std::cout << "  <Request Created=\"" << created << "\" LastEdit=\"" << lastEdit << "\" GUID=\"{" << guid << "}\" "
                  << "Priority=\"" << priority << "\" Status=\"" << status << "\" Type=\"20\" "
                  << "Worker=\"" << worker << "\" Requester=\"" << requester << "\" "
                  << "GameHandle=\"" << gameHandle << "\" Description=\"" << description << "\" "
                  << "Quantity=\"" << quantity << "\" Quality=\"" << quality << "\"/>\n";
    }

    std::cout << "</Requests>\n";

    return 0;
}