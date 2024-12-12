#include "deribit_api.h"
#include <iostream>

DeribitAPI::DeribitAPI(const std::string& cId, const std::string& cSecret) 
    : clientId(cId), clientSecret(cSecret) {}

size_t DeribitAPI::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string DeribitAPI::sendRequest(const std::string& url, const json& payload, const std::string& accessToken) {
    std::string readBuffer;
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        std::string jsonStr = payload.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!accessToken.empty()) {
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return readBuffer;
}

bool DeribitAPI::authenticate() {
    json payload = {
        {"id", 0},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"scope", "session:apiconsole-c5i26ds6dsr expires:2592000"},
            {"client_id", clientId},
            {"client_secret", clientSecret}
        }},
        {"jsonrpc", "2.0"}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/public/auth", payload);
    auto responseJson = json::parse(response);

    if (responseJson.contains("result") && responseJson["result"].contains("access_token")) {
        accessToken = responseJson["result"]["access_token"];
        return true;
    }
    
    std::cerr << "Failed to retrieve access token." << std::endl;
    return false;
}

std::string DeribitAPI::getOrderIdFromResponse(const std::string& response) {
    try {
        auto responseJson = json::parse(response);
        if (responseJson.contains("result") && 
            responseJson["result"].contains("order") && 
            responseJson["result"]["order"].contains("order_id")) {
            return responseJson["result"]["order"]["order_id"];
        }
    } catch (const json::exception& e) {
        std::cerr << "Error parsing order ID: " << e.what() << std::endl;
    }
    return "";
}

std::string DeribitAPI::placeOrder(const std::string& price, const std::string& amount, const std::string& instrument, bool isBuy) {
    std::string method = isBuy ? "private/buy" : "private/sell";
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", {
            {"instrument_name", instrument},
            {"type", "limit"},
            {"price", price},
            {"amount", amount}
        }},
        {"id", 1}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/" + method, payload, accessToken);
    std::cout << "Place Order Response: " << response << std::endl;
    return getOrderIdFromResponse(response);
}

void DeribitAPI::cancelOrder(const std::string& orderID) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/cancel"},
        {"params", {{"order_id", orderID}}},
        {"id", 6}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/cancel", payload, accessToken);
    std::cout << "Cancel Order Response: " << response << std::endl;
}

void DeribitAPI::modifyOrder(const std::string& orderID, int amount, double price) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/edit"},
        {"params", {
            {"order_id", orderID},
            {"amount", amount},
            {"price", price}
        }},
        {"id", 11}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/edit", payload, accessToken);
    std::cout << "Modify Order Response: " << response << std::endl;
}

void DeribitAPI::getOrderBook(const std::string& instrument) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "public/get_order_book"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 15}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/public/get_order_book", payload, accessToken);
    auto responseJson = json::parse(response);

    std::cout << "Order Book for " << instrument << ":\n\n";
    
    // Print best bid and ask
    std::cout << "Best Bid Price: " << responseJson["result"]["best_bid_price"] 
              << ", Amount: " << responseJson["result"]["best_bid_amount"] << '\n';
    std::cout << "Best Ask Price: " << responseJson["result"]["best_ask_price"] 
              << ", Amount: " << responseJson["result"]["best_ask_amount"] << '\n';

    // Print asks (sell orders)
    std::cout << "\nAsks (Sell Orders):\n";
    for (const auto& ask : responseJson["result"]["asks"]) {
        std::cout << "Price: " << ask[0] << ", Amount: " << ask[1] << '\n';
    }

    // Print bids (buy orders)
    std::cout << "\nBids (Buy Orders):\n";
    for (const auto& bid : responseJson["result"]["bids"]) {
        std::cout << "Price: " << bid[0] << ", Amount: " << bid[1] << '\n';
    }

    // Print additional market information
    std::cout << "\nAdditional Information:\n";
    std::cout << "Mark Price: " << responseJson["result"]["mark_price"] << '\n';
    std::cout << "Open Interest: " << responseJson["result"]["open_interest"] << '\n';
    std::cout << "Last Update Timestamp: " << responseJson["result"]["timestamp"] << '\n';
}

void DeribitAPI::getPosition(const std::string& instrument) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/get_position"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 20}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_position", payload, accessToken);
    auto responseJson = json::parse(response);
    
    if (responseJson.contains("result")) {
        auto result = responseJson["result"];
        std::cout << "Position Details for " << instrument << ":\n\n";
        
        // Position size and direction
        std::cout << "Size: " << result["size"] << '\n';
        std::cout << "Direction: " << result["direction"] << '\n';
        
        // Price information
        std::cout << "Average Price: " << result["average_price"] << '\n';
        std::cout << "Mark Price: " << result["mark_price"] << '\n';
        std::cout << "Index Price: " << result["index_price"] << '\n';
        std::cout << "Estimated Liquidation Price: " << result["estimated_liquidation_price"] << '\n';
        
        // Margin information
        std::cout << "Initial Margin: " << result["initial_margin"] << '\n';
        std::cout << "Maintenance Margin: " << result["maintenance_margin"] << '\n';
        std::cout << "Open Orders Margin: " << result["open_orders_margin"] << '\n';
        
        // PnL information
        std::cout << "Realized PnL: " << result["realized_profit_loss"] << '\n';
        std::cout << "Unrealized PnL: " << result["floating_profit_loss"] << '\n';
        std::cout << "Total PnL: " << result["total_profit_loss"] << '\n';
        
        // Additional information
        std::cout << "Leverage: " << result["leverage"] << '\n';
        std::cout << "Kind: " << result["kind"] << '\n';
    } else {
        std::cerr << "Error: Could not retrieve position data." << std::endl;
    }
}

void DeribitAPI::getOpenOrders() {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/get_open_orders"},
        {"params", {{"kind", "future"}, {"type", "limit"}}},
        {"id", 25}
    };

    std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_open_orders", payload, accessToken);
    auto responseJson = json::parse(response);

    if (responseJson.contains("result")) {
        std::cout << "Open Orders:\n\n";
        for (const auto& order : responseJson["result"]) {
            std::cout << "Order Details:\n";
            std::cout << "Instrument: " << order["instrument_name"] << '\n';
            std::cout << "Order ID: " << order["order_id"] << '\n';
            std::cout << "Direction: " << order["direction"] << '\n';
            std::cout << "Price: " << order["price"] << '\n';
            std::cout << "Amount: " << order["amount"] << '\n';
            std::cout << "Filled Amount: " << order["filled_amount"] << '\n';
            std::cout << "Average Price: " << order["average_price"] << '\n';
            std::cout << "Order Type: " << order["order_type"] << '\n';
            std::cout << "Order State: " << order["order_state"] << '\n';
            std::cout << "--------------------\n";
        }
    } else {
        std::cerr << "Error: Could not retrieve open orders." << std::endl;
    }
}