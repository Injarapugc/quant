#include "deribit_api.h"
#include <iostream>

std::string getInstrumentChoice() {
    std::cout << "\nSelect Instrument:\n";
    std::cout << "1. ETH-PERPETUAL\n";
    std::cout << "2. BTC-PERPETUAL\n";
    std::cout << "3. SOL-PERPETUAL\n";
    std::cout << "4. XRP-PERPETUAL\n";
    std::cout << "5. BNB-PERPETUAL\n";
    std::cout << "Enter choice (1-5): ";

    int choice;
    std::cin >> choice;
    std::cin.ignore();

    switch(choice) {
        case 1: return "ETH-PERPETUAL";
        case 2: return "BTC-PERPETUAL";
        case 3: return "SOL-PERPETUAL";
        case 4: return "XRP-PERPETUAL";
        case 5: return "BNB-PERPETUAL";
        default:
            std::cout << "Invalid choice. Defaulting to ETH-PERPETUAL\n";
            return "ETH-PERPETUAL";
    }
}

void showTradingInterface(DeribitAPI& api) {
    int choice;
    while (true) {
        std::cout << "\n=== Trading Interface ===\n";
        std::cout << "1. Place Buy Order\n";
        std::cout << "2. Place Sell Order\n";
        std::cout << "3. Cancel Order\n";
        std::cout << "4. Modify Order\n";
        std::cout << "5. View Order Book\n";
        std::cout << "6. View Open Orders\n";
        std::cout << "7. View Position\n";
        std::cout << "8. Exit\n";
        std::cout << "Enter your choice (1-8): ";
        
        std::cin >> choice;
        std::cin.ignore();

        std::string instrument;
        std::string price;
        std::string amount;
        std::string orderId;

        switch (choice) {
            case 1:
            case 2: {
                instrument = getInstrumentChoice();
                std::cout << "Enter price: ";
                std::getline(std::cin, price);
                std::cout << "Enter amount: ";
                std::getline(std::cin, amount);
                api.placeOrder(price, amount, instrument, choice == 1);
                break;
            }
            case 3: {
                std::cout << "Do you want to see open orders first? (y/n): ";
                char viewOrders;
                std::cin >> viewOrders;
                std::cin.ignore();
                if (viewOrders == 'y' || viewOrders == 'Y') {
                    api.getOpenOrders();
                }
                std::cout << "Enter order ID to cancel: ";
                std::getline(std::cin, orderId);
                api.cancelOrder(orderId);
                break;
            }
            case 4: {
                std::cout << "Do you want to see open orders first? (y/n): ";
                char viewOrders;
                std::cin >> viewOrders;
                std::cin.ignore();
                if (viewOrders == 'y' || viewOrders == 'Y') {
                    api.getOpenOrders();
                }
                std::cout << "Enter order ID to modify: ";
                std::getline(std::cin, orderId);
                std::cout << "Enter new amount: ";
                int newAmount;
                std::cin >> newAmount;
                std::cout << "Enter new price: ";
                double newPrice;
                std::cin >> newPrice;
                api.modifyOrder(orderId, newAmount, newPrice);
                break;
            }
            case 5: {
                std::cout << "Enter instrument (e.g., ETH-PERPETUAL): ";
                std::getline(std::cin, instrument);
                api.getOrderBook(instrument);
                break;
            }
            case 6: {
                api.getOpenOrders();
                break;
            }
            case 7: {
                std::cout << "Enter instrument (e.g., ETH-PERPETUAL): ";
                std::getline(std::cin, instrument);
                api.getPosition(instrument);
                break;
            }
            case 8: {
                std::cout << "Exiting...\n";
                return;
            }
            default:
                std::cout << "Invalid choice. Try again.\n";
        }
    }
}

int main() {
    // Replace with your actual client credentials
    std::string clientId = "G-aj5tZk";
    std::string clientSecret = "rKL0yIQfxLBZu4U0rD7rUgrgMr-Fq_Ta5FG0nnGNepE";

    // Create API instance
    DeribitAPI api(clientId, clientSecret);

    // Authenticate
    if (!api.authenticate()) {
        std::cerr << "Authentication failed. Exiting..." << std::endl;
        return 1;
    }

    // Show trading interface
    showTradingInterface(api);

    return 0;
}