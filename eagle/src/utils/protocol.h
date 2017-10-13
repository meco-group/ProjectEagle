#ifndef PROTOCOL_H
#define PROTOCOL_H

namespace eagle {

    /* helpers */
    typedef enum msg_t {
        MARKER = 0,
        OBSTACLE = 1,
        IMAGE = 2,
        CMD = 3,
    } msg_t;

    typedef enum shape_t {
        TRIANGLE = 0,
        SQUARE = 1,
        RECTANGLE = 2,
        CIRCLE = 3,
        ELLIPSOID = 4
    } shape_t;

    typedef struct point_t {
        double x;
        double y;
    } point_t;

    /* protocol messages */
    typedef struct header_t {
        msg_t id;
        unsigned long time;
    } header_t;

    typedef struct marker_t {
        int id;
        double x;
        double y;
        double z;
        double t;
    } robot_t;

    typedef struct obstacle_t {
        int id;
        shape_t shape;
        point_t p1;
        point_t p2;
        point_t p3;
    } obstacle_t;

    typedef enum cmd_t {
        SNAPSHOT = 0
    } cmd_t;

}

#endif //PROTOCOL_H
