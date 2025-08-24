#include <iostream>
#include <thread>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring> // Required for memset

using namespace std;

// Global variables for clients and users
map<int, string> clients;
mutex clients_mtx; // Mutex to protect the clients map

// --- User Authentication and Registration ---

// Checks if a user exists in the users.txt file
bool authenticateUser(const string& username, const string& password) {
    ifstream file("users.txt");
    if (!file.is_open()) {
        return false; // No users file means no users to authenticate
    }
    string u, p;
    while (file >> u >> p) {
        if (u == username && p == password) {
            return true;
        }
    }
    return false;
}

// Adds a new user to the users.txt file
bool registerUser(const string& username, const string& password) {
    // Optional: Check if user already exists
    ifstream infile("users.txt");
    string u, p;
    while (infile >> u >> p) {
        if (u == username) {
            return false; // User already exists
        }
    }
    infile.close();

    // Add new user
    ofstream outfile("users.txt", ios::app);
    if (!outfile.is_open()) {
        cerr << "Error: Could not open users.txt for writing." << endl;
        return false;
    }
    outfile << username << " " << password << "\n";
    return true;
}

// --- Message Broadcasting ---

// Sends a message to all connected clients except the sender
void broadcastMessage(const string& message, int senderSocket) {
    lock_guard<mutex> lock(clients_mtx);
    for (auto const& [socket, username] : clients) {
        if (socket != senderSocket) {
            send(socket, message.c_str(), message.length(), 0);
        }
    }
}

// Sends a private message to a specific user
void privateMessage(const string& message, const string& targetUser) {
    lock_guard<mutex> lock(clients_mtx);
    for (auto const& [socket, username] : clients) {
        if (username == targetUser) {
            send(socket, message.c_str(), message.length(), 0);
            return; // Found the user, no need to continue
        }
    }
}

// --- Client Handling ---

void handleClient(int clientSocket) {
    cout << "[Server] New connection. Thread started for socket " << clientSocket << "." << endl;
    char buffer[1024];
    string username;

    // --- Handshake Protocol ---
    // 1. Receive choice (login/register)
    memset(buffer, 0, 1024);
    if (recv(clientSocket, buffer, 1023, 0) <= 0) {
        cerr << "[Server] Error: Client disconnected during handshake (choice)." << endl;
        close(clientSocket);
        return;
    }
    string choice(buffer);

    // 2. Receive username
    memset(buffer, 0, 1024);
    if (recv(clientSocket, buffer, 1023, 0) <= 0) {
        cerr << "[Server] Error: Client disconnected during handshake (username)." << endl;
        close(clientSocket);
        return;
    }
    username = string(buffer);

    // 3. Receive password
    memset(buffer, 0, 1024);
    if (recv(clientSocket, buffer, 1023, 0) <= 0) {
        cerr << "[Server] Error: Client disconnected during handshake (password)." << endl;
        close(clientSocket);
        return;
    }
    string password(buffer);
    
    // --- Process Login/Register ---
    bool authenticated = false;
    if (choice == "login") {
        if (authenticateUser(username, password)) {
            authenticated = true;
            string welcome_msg = "[Server] Welcome back, " + username + "!";
            send(clientSocket, welcome_msg.c_str(), welcome_msg.length(), 0);
        } else {
            string error_msg = "[Server] Error: Authentication failed.";
            send(clientSocket, error_msg.c_str(), error_msg.length(), 0);
        }
    } else if (choice == "register") {
        if (registerUser(username, password)) {
            authenticated = true;
            string welcome_msg = "[Server] Registration successful! Welcome, " + username + "!";
            send(clientSocket, welcome_msg.c_str(), welcome_msg.length(), 0);
        } else {
            string error_msg = "[Server] Error: Registration failed. Username may already exist.";
            send(clientSocket, error_msg.c_str(), error_msg.length(), 0);
        }
    } else {
        string error_msg = "[Server] Error: Invalid command received.";
        send(clientSocket, error_msg.c_str(), error_msg.length(), 0);
    }
    
    if (!authenticated) {
        cout << "[Server] Authentication failed for user '" << username << "'. Closing connection." << endl;
        close(clientSocket);
        return;
    }

    // Add authenticated user to the map
    {
        lock_guard<mutex> lock(clients_mtx);
        clients[clientSocket] = username;
    }

    string join_notification = "[Server] " + username + " has joined the chat.";
    cout << join_notification << endl;
    broadcastMessage(join_notification, clientSocket);

    // --- Message Loop ---
    while (true) {
        memset(buffer, 0, 1024);
        int bytes_received = recv(clientSocket, buffer, 1023, 0);
        if (bytes_received <= 0) {
            break; // Client disconnected
        }
        string msg(buffer);
        
        if (msg.rfind("/msg", 0) == 0) { // Private message
            size_t first_space = msg.find(' ');
            if (first_space != string::npos) {
                size_t second_space = msg.find(' ', first_space + 1);
                if (second_space != string::npos) {
                    string targetUser = msg.substr(first_space + 1, second_space - (first_space + 1));
                    string text = msg.substr(second_space + 1);
                    string private_msg = "[Private from " + username + "]: " + text;
                    privateMessage(private_msg, targetUser);
                }
            }
        } else { // Broadcast message
            string broadcast_msg = "[" + username + "]: " + msg;
            broadcastMessage(broadcast_msg, clientSocket);
        }
    }
    
    // --- Cleanup on Disconnect ---
    string leave_notification = "[Server] " + username + " has left the chat.";
    cout << leave_notification << endl;
    {
        lock_guard<mutex> lock(clients_mtx);
        clients.erase(clientSocket);
    }
    broadcastMessage(leave_notification, -1); // Send to all remaining clients
    close(clientSocket);
}


// --- Main Server Setup ---

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Error: Could not create socket." << endl;
        return 1;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081); // Use a non-standard port like 8081
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error: Could not bind to port 8081." << endl;
        return 1;
    }
    
    if (listen(serverSocket, 5) < 0) {
        cerr << "Error: Listen failed." << endl;
        return 1;
    }
    
    cout << "[Server] Now listening for connections on port 8081..." << endl;
    
    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            cerr << "Error: Accept failed." << endl;
            continue;
        }
        thread(handleClient, clientSocket).detach();
    }
    
    close(serverSocket);
    return 0;
}