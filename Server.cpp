#include <bits/stdc++.h>

using namespace std;

map<int, string> clients;
mutex clients_mtx;

bool authenticateUser(const string& username, const string& password) {
    ifstream file("users.txt");
    if (!file.is_open()) {
        return false;
    }
    string u, p;
    while (file >> u >> p) {
        if (u == username && p == password) {
            return true;
        }
    }
    return false;
}

bool registerUser(const string& username, const string& password) {
    ifstream infile("users.txt");
    string u, p;
    while (infile >> u >> p) {
        if (u == username) {
            return false;
        }
    }
    infile.close();

    ofstream outfile("users.txt", ios::app);
    if (!outfile.is_open()) {
        cerr << "Error: Could not open users.txt for writing." << endl;
        return false;
    }
    outfile << username << " " << password << "\n";
    return true;
}


void broadcastMessage(const string& message, int senderSocket) {
    lock_guard<mutex> lock(clients_mtx);
    for (auto const& [socket, username] : clients) {
        if (socket != senderSocket) {
            send(socket, message.c_str(), message.length(), 0);
        }
    }
}

void privateMessage(const string& message, const string& targetUser) {
    lock_guard<mutex> lock(clients_mtx);
    for (auto const& [socket, username] : clients) {
        if (username == targetUser) {
            send(socket, message.c_str(), message.length(), 0);
            return; 
        }
    }
}

void handleClient(int clientSocket) {
    cout << "[Server] New connection. Thread started for socket " << clientSocket << "." << endl;
    char buffer[1024];
    string username;

    memset(buffer, 0, 1024);
    if (recv(clientSocket, buffer, 1023, 0) <= 0) {
        cerr << "[Server] Error: Client disconnected during handshake (choice)." << endl;
        close(clientSocket);
        return;
    }
    string choice(buffer);

    memset(buffer, 0, 1024);
    if (recv(clientSocket, buffer, 1023, 0) <= 0) {
        cerr << "[Server] Error: Client disconnected during handshake (username)." << endl;
        close(clientSocket);
        return;
    }
    username = string(buffer);

    memset(buffer, 0, 1024);
    if (recv(clientSocket, buffer, 1023, 0) <= 0) {
        cerr << "[Server] Error: Client disconnected during handshake (password)." << endl;
        close(clientSocket);
        return;
    }
    string password(buffer);
    
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

    {
        lock_guard<mutex> lock(clients_mtx);
        clients[clientSocket] = username;
    }

    string join_notification = "[Server] " + username + " has joined the chat.";
    cout << join_notification << endl;
    broadcastMessage(join_notification, clientSocket);

    while (true) {
        memset(buffer, 0, 1024);
        int bytes_received = recv(clientSocket, buffer, 1023, 0);
        if (bytes_received <= 0) {
            break; 
        }
        string msg(buffer);
        
        if (msg.rfind("/msg", 0) == 0) { 
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
        } else { 
            string broadcast_msg = "[" + username + "]: " + msg;
            broadcastMessage(broadcast_msg, clientSocket);
        }
    }
    
    string leave_notification = "[Server] " + username + " has left the chat.";
    cout << leave_notification << endl;
    {
        lock_guard<mutex> lock(clients_mtx);
        clients.erase(clientSocket);
    }
    broadcastMessage(leave_notification, -1); 
    close(clientSocket);
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Error: Could not create socket." << endl;
        return 1;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081); 
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
