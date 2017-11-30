#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

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
    uint32_t time;
} header_t;

typedef struct marker_t {
    uint16_t id;
    double x;
    double y;
    double z;
    double roll;
    double pitch;
    double yaw;
} robot_t;

typedef struct obstacle_t {
    uint16_t id;
    shape_t shape;
    point_t p1;
    point_t p2;
    point_t p3;
} obstacle_t;

typedef struct image_t {
    point_t offset;
    uint32_t px_per_meter;
} image_t;

typedef enum cmd_t {
    SNAPSHOT = 0,
    BACKGROUND = 1,
    IMAGE_STREAM_ON = 10,
    IMAGE_STREAM_OFF = 11,
    IMAGE_STREAM_TOGGLE = 12,
    DETECTION_ON = 20,
    DETECTION_OFF = 21,
    DETECTION_TOGGLE = 22,
    DEBUG_MODE_ON = 30,
    DEBUG_MODE_OFF = 31,
    DEBUG_MODE_TOGGLE = 32,
    CALIBRATION_ON = 40,
    CALIBRATION_OFF = 41,
    CALIBRATION_TOGGLE = 42,
    RECORD_ON = 50,
    RECORD_OFF = 51,
    RECORD_TOGGLE = 52
} cmd_t;

}

#endif //PROTOCOL_H
