#include <crow.h>
#include <map>
#include <string>
#include <optional>

// API: interface that allows us to programmatically interface with an application
// Web API: an API that uses HTTP (Hypertext Transfer Protocol) to transport data

// HTTP: A communication protocol used to exchange different media types over a network
// GET: Returns info about the requested resource
// POST: Creates a new resource
// PUT: Performs a full update by replacing a resource
// DELETE: Removes a resource

// Microservices communicate with each other using APIs
// The API documentation tells us how to interact with the microservice

// API documentation: A description of the API
// Following a standard interface description language such as OpenAPI for REST APIs

// Why Crow?
// 1. Lightweight and fast (C++ efficiency)
// 2. Easy to use, similar to Python's Flask and FastAPI
// 3. Supports modern C++ features

// Initialize our API by creating an app object by calling the Crow::SimpleApp constructor
crow::SimpleApp app;

// Define the Item structure with name, price, and optional description
struct Item {
    std::string name;
    double price;
    std::optional<std::string> description;
};

// In-memory data store
std::map<int, crow::json::wvalue> inventory;

// GET /get-item/{item_id}
crow::response get_item(const crow::request& req, int item_id, const std::string& name) {
    if (inventory.find(item_id) == inventory.end()) {
        return crow::response(404, "Item ID not found.");
    }
    if (!name.empty() && inventory[item_id]["name"].s() != name) {
        return crow::response(404, "Item name not found.");
    }
    return crow::response(inventory[item_id].dump());
}

// POST /create-item/{item_id}
crow::response create_item(const crow::request& req, int item_id) {
    auto item = crow::json::load(req.body);
    if (!item) {
        return crow::response(400, "Invalid JSON.");
    }
    if (inventory.find(item_id) != inventory.end()) {
        return crow::response(400, "Item ID already exists.");
    }
    inventory[item_id] = item;
    return crow::response(inventory[item_id].dump());
}

// PUT /update-item/{item_id}
crow::response update_item(const crow::request& req, int item_id) {
    auto item = crow::json::load(req.body);
    if (!item) {
        return crow::response(400, "Invalid JSON.");
    }
    if (inventory.find(item_id) == inventory.end()) {
        return crow::response(404, "Item ID does not exist.");
    }
    if (!item["name"].s().empty()) {
        inventory[item_id]["name"] = item["name"];
    }
    if (!item["price"].t().is_null()) {
        inventory[item_id]["price"] = item["price"];
    }
    if (!item["description"].s().empty()) {
        inventory[item_id]["description"] = item["description"];
    }
    return crow::response(inventory[item_id].dump());
}

// DELETE /delete-item/{item_id}
crow::response delete_item(int item_id) {
    if (inventory.find(item_id) == inventory.end()) {
        return crow::response(404, "Item ID does not exist.");
    }
    inventory.erase(item_id);
    return crow::response(200, "Success: Item deleted!");
}

int main() {
    CROW_ROUTE(app, "/get-item/<int>").methods("GET"_method)([](const crow::request& req, int item_id) {
        auto name = req.url_params.get("name");
        return get_item(req, item_id, name ? std::string(name) : "");
    });

    CROW_ROUTE(app, "/create-item/<int>").methods("POST"_method)([](const crow::request& req, int item_id) {
        return create_item(req, item_id);
    });

    CROW_ROUTE(app, "/update-item/<int>").methods("PUT"_method)([](const crow::request& req, int item_id) {
        return update_item(req, item_id);
    });

    CROW_ROUTE(app, "/delete-item/<int>").methods("DELETE"_method)([](int item_id) {
        return delete_item(item_id);
    });

    // Run the app on port 8080, multithreaded
    app.port(8080).multithreaded().run();
}
