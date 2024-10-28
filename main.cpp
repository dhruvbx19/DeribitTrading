#include <iostream>
#include <string>
#include <cstdlib>
#include <curl/curl.h>
#include "json.hpp" // Include the nlohmann/json header

using json = nlohmann::json;

std::string client_id = "b8SiXcDj";
std::string client_secret = "F1tF20RGqpHI_8GTlb1Iyv7qmDB1F3HQySQVXeRD4vM";
std::string base_url = "https://test.deribit.com/api/v2/";
std::string access_token; // Global variable for the access token

// Function declarations
std::string authenticate();
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
void placeOrder(const std::string& instrumentName, const std::string& side, double amount, double price);
void cancelOrder(const std::string& orderId);
void modifyOrder(const std::string& orderId, double amount, double price);
void getOrderBook(const std::string& instrumentName);
void viewCurrentPositions();

int main() {
    // Authenticate and get the access token
    if (authenticate().empty()) {
        return 1; // Exit if authentication fails
    }

    while (true) {
        std::cout << "\nMenu:\n";
        std::cout << "1. Place Order\n";
        std::cout << "2. Cancel Order\n";
        std::cout << "3. Modify Order\n";
        std::cout << "4. Get Order Book\n";
        std::cout << "5. View Current Positions\n";
        std::cout << "6. Exit\n";
        std::cout << "Select an option (1-6): ";

        int choice;
        std::cin >> choice;

        switch (choice) {
            case 1: {
                std::string instrumentName;
                std::string side;
                double amount, price;

                std::cout << "Enter instrument name: ";
                std::cin >> instrumentName;  // e.g., "BTC-USD"
                std::cout << "Enter side (buy/sell): ";
                std::cin >> side;  // "buy" or "sell"
                std::cout << "Enter amount: ";
                std::cin >> amount;  // Quantity
                std::cout << "Enter price: ";
                std::cin >> price;  // Price per unit

                placeOrder(instrumentName, side, amount, price);
                break;
            }
            case 2: {
                std::string orderId;
                std::cout << "Enter order ID to cancel: ";
                std::cin >> orderId;
                cancelOrder(orderId);
                break;
            }
            case 3: {
                std::string orderId;
                double amount, price;

                std::cout << "Enter order ID to modify: ";
                std::cin >> orderId;
                std::cout << "Enter new amount: ";
                std::cin >> amount;
                std::cout << "Enter new price: ";
                std::cin >> price;

                modifyOrder(orderId, amount, price);
                break;
            }
            case 4: {
                std::string instrumentName;
                std::cout << "Enter instrument name to get order book: ";
                std::cin >> instrumentName;
                getOrderBook(instrumentName);
                break;
            }
            case 5: {
                viewCurrentPositions();
                break;
            }
            case 6:
                std::cout << "Exiting program.\n";
                return 0;
            default:
                std::cout << "Invalid choice. Please select a valid option.\n";
        }
    }
}

// Function to handle cURL response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t newLength = size * nmemb;
    std::string* s = static_cast<std::string*>(userp);
    s->append((char*)contents, newLength);
    return newLength;
}

// Function to authenticate and retrieve access token
std::string authenticate() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        // Prepare the URL and JSON-RPC body
        std::string url = base_url + "public/auth";
        json request_body = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "public/auth"},
            {"params", {
                {"grant_type", "client_credentials"},
                {"client_id", client_id},
                {"client_secret", client_secret}
            }}
        };

        std::string postFields = request_body.dump();

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res == CURLE_OK) {
            try {
                // Parse the JSON response
                json response = json::parse(readBuffer);

                // Check if "result" and "access_token" are present in the response
                if (response.contains("result") && response["result"].contains("access_token") && !response["result"]["access_token"].is_null()) {
                    access_token = response["result"]["access_token"].get<std::string>();
                    std::cout << "Authenticated successfully. Access token: " << access_token << std::endl;
                    return access_token;
                } else {
                    std::cerr << "Authentication failed: Access token not found in the response." << std::endl;
                    return "";
                }
            } catch (const json::exception& e) {
                std::cerr << "JSON Parsing error: " << e.what() << std::endl;
                return "";
            }
        } else {
            std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;
        }
    }
    return "";
}


// Place order function
void placeOrder(const std::string& instrumentName, const std::string& side, double amount, double price) {
    //pointer for sending the HTTP request
    CURL* curl;
    CURLcode res;
    //string to store the response data
    std::string readBuffer;


    //Initialize the cURL Session
    curl = curl_easy_init();
    if (curl) {
        std::string url = base_url + "private/buy";
        //Set Up the Request URL and Body
        json request_body = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "private/buy"},
            {"params", {
                {"instrument_name", instrumentName},
                {"amount", amount},
                {"price", price},
                {"type", "limit"},
                {"side", side}
            }}
        };

        //Convert Request Body to a String
        std::string postFields = request_body.dump();
        //Set Up the Request Headers
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + access_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        //Set the cURL Options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        //perform curl request
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        //handling the response
        if (res == CURLE_OK) {
            json response = json::parse(readBuffer);
            std::cout << "Order placed successfully: " << response.dump(4) << std::endl;
        } else {
            std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;
        }
    }
}

// Cancel order function
void cancelOrder(const std::string& orderId) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        std::string url = base_url + "private/cancel";  // Adjust based on API docs
        json request_body = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "private/cancel"},
            {"params", {
                {"order_id", orderId}
            }}
        };

        std::string postFields = request_body.dump();
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + access_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res == CURLE_OK) {
            json response = json::parse(readBuffer);
            std::cout << "Order cancelled successfully: " << response.dump(4) << std::endl;
        } else {
            std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;
        }
    }
}

// Modify order function
void modifyOrder(const std::string& orderId, double amount, double price) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        std::string url = base_url + "private/edit";  // Adjust based on API docs
        json request_body = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "private/edit"},
            {"params", {
                {"order_id", orderId},
                {"amount", amount},
                {"price", price}
            }}
        };

        std::string postFields = request_body.dump();
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + access_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res == CURLE_OK) {
            json response = json::parse(readBuffer);
            std::cout << "Order modified successfully: " << response.dump(4) << std::endl;
        } else {
            std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;
        }
    }
}

// Get order book function
void getOrderBook(const std::string& instrumentName) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        std::string url = base_url + "public/get_order_book";  // Adjust based on API docs
        json request_body = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "public/get_order_book"},
            {"params", {
                {"instrument_name", instrumentName}
            }}
        };

        std::string postFields = request_body.dump();
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res == CURLE_OK) {
            json response = json::parse(readBuffer);
            std::cout << "Order book retrieved successfully: " << response.dump(4) << std::endl;
        } else {
            std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;
        }
    }
}

// View current positions function
void viewCurrentPositions() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        std::string url = base_url + "private/get_positions";  // Adjust based on API docs
        json request_body = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "private/get_positions"},
            {"params", {}}
        };

        std::string postFields = request_body.dump();
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + access_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res == CURLE_OK) {
            json response = json::parse(readBuffer);
            std::cout << "Current positions retrieved successfully: " << response.dump(4) << std::endl;
        } else {
            std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;
        }
    }
}

