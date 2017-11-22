#include "utils/rapidxml.hpp"
#include "utils/rapidxml_print.hpp"
#include "communicator/message.h"
#include "communicator/communicator.h"
#include "detector/pnp_pattern_extractor3.h"
#include "detector/collector.h"
#include "detector/transform.h"
#include "detector/robot.h"
#include "detector/obstacle_rectangle.h"
#include "detector/pattern_interface.h"
#include "detector/pattern.h"
#include "detector/fuse.h"
#include "detector/chessboard.h"
#include "detector/calibration_pattern.h"
#include "detector/projection.h"
#include "detector/obstacle_circle.h"
#include "detector/pattern_extractor.h"
#include "detector/obstacle.h"
#include "detector/circle_triangle.h"
#include "detector/pattern_extractor3.h"
#include "detector/detector.h"
#include "detector/planar_pattern_extractor3.h"
#include "utils/protocol.h"
#include "utils/utils.h"
#include "utils/config_helper.h"
#include "utils/eagle_cmd_helper.h"
#include "camera/see3_camera.h"
#include "camera/camera.h"
#include "camera/camera_interface.h"
#include "camera/v4l2_camera.h"
#include "camera/latitude_camera.h"
#include "camera/odroid_camera.h"
#include "camera/opi_camera.h"
#include "camera/pi_camera.h"
