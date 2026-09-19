#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <cerrno>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
using namespace std;

int main()
{
    // socket creation for client
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        cout << "socket not created: " << strerror(errno) << "\n";
        return -1;
    }
    cout << "socket created successfully, fd = " << socket_fd << "\n";


    // connecting the client to the server
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    int connect_status = connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_status == -1)
    {
        cout << "connect failed:" << strerror(errno) << "\n";
        return -1;
    }
    cout << "connected to server successfully! \n";

    int student_id;
    cout << "Enter your student ID: ";
    cin >> student_id;
    cin.ignore();

    cout << "\nChoose an action: \n";
    cout << "1. List rooms -> type: LIST\n";
    cout << "2. Reserve a room -> type: RESERVE <room_id>\n";
    cout << "3. Cancel a room -> type: CANCEL <room_id>\n";
    cout << "4. Check room status -> type: STATUS <room_id>\n";
    cout << "5. Exit -> type: exit\n\n";
    
    while (true)
    {
        // send message to server
        char message[1024];
        cout << "Enter the message: \n";
        cin.getline(message, 1024);

        //client exit call
        if(strcmp(message, "exit") == 0){
            cout << "exiting...\n";
            break;
        }

        string full_message = string(message);
        stringstream ss(message);
        string cmd;
        ss >> cmd;
        if(cmd == "RESERVE" || cmd == "CANCEL"){
            full_message += " " + to_string(student_id);
        }

        int bytes_sent = send(socket_fd, full_message.c_str(), full_message.size(), 0);
        if (bytes_sent == -1)
        {
            cout << "send failed: " << strerror(errno) << "\n";
            return -1;
        }
        cout << "message sent to server successfully!\n";

        // read message from server
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytes_recv = recv(socket_fd, buffer, 1024, 0);
        if (bytes_recv == -1)
        {
            cout << "recv failed: " << strerror(errno) << "\n";
            return -1;
        }
        if(bytes_recv == 0){
            cout << "server disconnected!\n";
            break;
        }
        buffer[bytes_recv] = '\0';
        cout << "message received: \n" << buffer << "\n";
    }

    close(socket_fd);
    return 0;
}