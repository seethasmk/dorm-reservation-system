#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <cerrno>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include "room.h"
#include "student.h"
#include <vector>
#include <mutex>
#include <sstream>
using namespace std;

vector<Room> rooms;
mutex rooms_mutex;

// Initialize rooms
void add_rooms(vector<Room> &rooms, int start_id, int end_id, int floor, RoomType type)
{
    for (int i = start_id; i <= end_id; i++)
    {
        Room r;
        r.id = i;
        r.floor = floor;
        r.is_booked = false;
        r.type = type;
        r.booked_by_student_id = -1;
        rooms.push_back(r);
    }
}

vector<Room> initializeRooms()
{
    vector<Room> rooms;
    add_rooms(rooms, 101, 110, 1, SINGLE);
    add_rooms(rooms, 201, 210, 2, DOUBLE);
    return rooms;
}

void handle_client(int client_fd)
{
    while (true)
    {
        // Read data from client
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_fd, buffer, 1024, 0);
        if (bytes_read == 0)
        {
            cout << "client disconnected \n";
            break;
        }
        if (bytes_read == -1)
        {
            cout << "recv failed: " << strerror(errno) << "\n";
            return;
        }
        buffer[bytes_read] = '\0';
        cout << "received: " << buffer << "\n";

        stringstream ss(buffer);
        string command;
        ss >> command;

        // sends the list of available rooms to the client
        if (command == "LIST")
        {
            string response = "";
            rooms_mutex.lock();
            for (Room &r : rooms)
            {
                response += "Room " + to_string(r.id) + " | Floor " + to_string(r.floor);
                response += " | " + string(r.type == SINGLE ? "SINGLE" : "DOUBLE");
                response += " | " + string(r.is_booked ? "booked" : "available");
                response += "\n";
            }
            rooms_mutex.unlock();
            send(client_fd, response.c_str(), response.size(), 0);
        }
        // reserves the room requested by the client based on availability and sends a response
        else if (command == "RESERVE")
        {
            string response = "";
            bool found = false;
            int room_id, student_id;
            ss >> room_id >> student_id;
            rooms_mutex.lock();
            for (auto &r : rooms)
            {
                if (r.id == room_id)
                {
                    found = true;
                    if (r.is_booked == false)
                    {
                        r.is_booked = true;
                        r.booked_by_student_id = student_id;
                        response += "Room " + to_string(room_id) + " successfully booked by the student " + to_string(student_id);
                    }
                    else
                    {
                        response += "Room " + to_string(room_id) + " already booked!";
                    }
                }
            }
            if (!found)
            {
                response += "Room " + to_string(room_id) + " not found!";
            }
            rooms_mutex.unlock();
            send(client_fd, response.c_str(), response.size(), 0);
        }
        else if (command == "CANCEL")
        {
            string response = "";
            int room_id, student_id;
            ss >> room_id >> student_id;
            bool found = false;
            rooms_mutex.lock();
            for (auto &r : rooms)
            {
                if (r.id == room_id)
                {
                    found = true;
                    if (!r.is_booked)
                    {
                        response += "Room " + to_string(room_id) + " is not currently booked!";
                    }
                    else if (r.booked_by_student_id == student_id)
                    {
                        r.is_booked = false;
                        r.booked_by_student_id = -1;
                        response += "Room " + to_string(room_id) + " is available now!";
                    }
                    else
                    {
                        response += "Not allowed to cancel the reservation.";
                    }
                }
            }
            if (!found)
            {
                response += "Room " + to_string(room_id) + " not found!";
            }
            rooms_mutex.unlock();
            send(client_fd, response.c_str(), response.size(), 0);
        }
        else if(command == "STATUS"){
            string response = "";
            int room_id;
            ss >> room_id;
            bool found = false;
            rooms_mutex.lock();
            for(auto& r: rooms){
                if(r.id == room_id){
                    found = true;
                    string type_str = (r.type == SINGLE ? "SINGLE" : "DOUBLE");
                    if(r.booked_by_student_id != -1){
                        response += "Room "+ to_string(room_id)+ " | Floor " + to_string(r.floor)+ " | " + type_str + " | booked by student" + to_string(r.booked_by_student_id);
                    }
                    else{
                        response += "Room "+ to_string(room_id)+ " | Floor " + to_string(r.floor)+ " | " + type_str + " is available";  
                    }
                }
            }
            if (!found)
            {
                response += "Room " + to_string(room_id) + " not found!";
            }
            rooms_mutex.unlock();
            send(client_fd, response.c_str(), response.size(), 0);
        }
        else
        {
            // Send data back to client
            int bytes_sent = send(client_fd, buffer, bytes_read, 0);
            if (bytes_sent == -1)
            {
                cout << "send failed: " << strerror(errno) << "\n";
                return;
            }
            cout << "sent: " << bytes_sent << " bytes back to the client \n";
        }
    }
    close(client_fd);
}

int main()
{
    // socket creation
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        cout << "socket not created: " << strerror(errno) << "\n";
        return -1;
    }
    cout << "socket created, fd = " << socket_fd << "\n";

    // Binding the socket to the port
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    int bind_status = ::bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_status == -1)
    {
        cout << "bind failed: " << strerror(errno) << "\n";
        return -1;
    }
    cout << "bind successful\n";

    // Listening for incoming connections
    int listen_status = listen(socket_fd, 5);
    if (listen_status == -1)
    {
        cout << "listen failed: " << strerror(errno) << "\n";
        return -1;
    }
    cout << "listen successful \n";

    rooms = initializeRooms();
    cout << "Available no. of rooms: " << rooms.size() << "\n";

    while (true)
    {
        // Accept a new client with a separate fd
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1)
        {
            cout << "accept failed: " << strerror(errno) << "\n";
            return -1;
        }
        cout << "accept successful, child_fd = " << client_fd << "\n";
        thread t(handle_client, client_fd);
        t.detach();
    }

    close(socket_fd);
    return 0;
}