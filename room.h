#ifndef ROOM_H
#define ROOM_H

enum RoomType {SINGLE, DOUBLE};

struct Room
{
    int id;
    int floor;
    RoomType type;
    bool is_booked;
    int booked_by_student_id;
};


#endif