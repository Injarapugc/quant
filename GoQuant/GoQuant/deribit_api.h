#ifndef DERIBIT_API_H
#define DERIBIT_API_H

#include <string>
#include <curl/curl.h>
#include "include/json.hpp"

using json = nlohmann::json;

class DeribitAPI {
private:
    std::string accessToken;
    std::string clientId;
    std::string clientSecret;
    
    // Helper functions
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    std::string sendRequest(const std::string& url, const json& payload, const std::string& accessToken = "");
    std::string getOrderIdFromResponse(const std::string& response);
    std::string getInstrumentChoice();

public:
    // Constructor
    DeribitAPI(const std::string& clientId, const std::string& clientSecret);
    
    // Authentication
    bool authenticate();
    
    // Trading functions
    std::string placeOrder(const std::string& price, const std::string& amount, const std::string& instrument, bool isBuy = true);
    void cancelOrder(const std::string& orderID);
    void modifyOrder(const std::string& orderID, int amount, double price);
    
    // Market data functions
    void getOrderBook(const std::string& instrument);
    void getPosition(const std::string& instrument);
    void getOpenOrders();
    
    // UI functions
    void showTradingInterface();
};

#endif // DERIBIT_API_H