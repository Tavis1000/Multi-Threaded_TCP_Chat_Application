#include <bits/stdc++.h>

using namespace std;

void receiveMessages(int socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, 1024);
        int bytes_received = recv(socket, buffer, 1023, 0);
        if (bytes_received <= 0) {
            cout << "\n[Client] Disconnected from server." << endl;
            close(socket);
            exit(0); 
        }
        cout << "\n" << buffer << endl;
        cout << "> "; 
        cout.flush(); 
    }
}

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Error: Could not create socket." << endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081); 
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    cout << "[Client] Connecting to server on port 8081..." << endl;
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error: Connection failed. Is the server running?" << endl;
        return 1;
    }
    cout << "[Client] Connection successful." << endl;

    string choice, username, password;
    cout << "Enter 'login' or 'register': ";
    cin >> choice;
    send(clientSocket, choice.c_str(), choice.length(), 0);

    cout << "Enter username: ";
    cin >> username;
    send(clientSocket, username.c_str(), username.length(), 0);

    cout << "Enter password: ";
    cin >> password;
    send(clientSocket, password.c_str(), password.length(), 0);

    thread(receiveMessages, clientSocket).detach();

    this_thread::sleep_for(chrono::milliseconds(100));

    cout << "\n--- Chat started ---" << endl;
    cout << "Type '/msg <user> <message>' for private messages." << endl;
    
    string msg;
    cin.ignore(); 
    
    while (true) {
        cout << "> ";
        getline(cin, msg);
        if (msg.empty()) {
            continue;
        }
        if (send(clientSocket, msg.c_str(), msg.length(), 0) < 0) {
            cerr << "Error: Failed to send message." << endl;
            break;
        }
    }

    close(clientSocket);
    return 0;
}
