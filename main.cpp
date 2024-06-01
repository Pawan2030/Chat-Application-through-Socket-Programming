#include <iostream>
#include <WS2tcpip.h>
#include <winsock2.h>
#include <tchar.h>
#include<thread>
#include<vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

bool Initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void InteractWithClient(SOCKET clientSocket , vector<SOCKET>& clients) {

    //send / recv client

    cout << "client Connected" << endl;
    char buffer[4096];

    while (1) {

        int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesrecvd <= 0) {
            cout << "client disconnected" << endl;
            break;
        }

        
        string message(buffer, bytesrecvd);
        cout << "Message from client: " << message << endl;

        for (auto client : clients) {
            if (client != clientSocket) {
                send(client, message.c_str(), message.length(), 0);
            }
        }
    }

    auto it = find(clients.begin(), clients.end(), clientSocket);

    if (it != clients.end()) {
        clients.erase(it);
    }

    // Clean up
    closesocket(clientSocket);
}

int main() {
    if (!Initialize()) {
        cout << "Winsock initialization failed" << endl;
        return 1;
    }

    cout << "Server program" << endl;

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (listenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        return 1;
    }

    // Creating address structure
    int port = 12345;
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);

    // Convert the IP address (0.0.0.0) and put it inside the sin_addr in binary form
    if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
        cout << "Setting address structure failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Bind the socket
    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        cout << "Bind failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Listen
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listen failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server has started listening on port: " << port << endl;
    
    vector<SOCKET> clients;

    while (1) {

        // Accept a client socket
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);

        if (clientSocket == INVALID_SOCKET) {
            cout << "Invalid client socket" << endl;
        }

        clients.push_back(clientSocket);
        thread t1(InteractWithClient, clientSocket , std::ref(clients));
        t1.detach();
     }
    

    closesocket(listenSocket);
    WSACleanup();

    return 0;
}
