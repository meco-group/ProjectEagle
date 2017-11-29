#ifndef COLLECTOR_H
#define COLLECTOR_H

#include "robot.h"
#include "obstacle.h"
#include "obstacle_rectangle.h"
#include "obstacle_circle.h"
#include "pattern_extractor.h"
#include <map>
#include <stdint.h>

namespace eagle {

class Collector {

  private:
    int _verbose;
    std::map<std::string, std::vector<marker_t>> _robot_map;
    std::map<std::string, std::vector<Obstacle*>> _obstacle_map;
    std::map<std::string, std::vector<uint32_t>> _obstacle_time_map;
    std::map<std::string, std::vector<uint32_t>> _robot_time_map;
    void merge_data(std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles, std::vector<uint32_t>& times_robot, std::vector<uint32_t>& times_obstacles);

  public:

    Collector();

    void add(const std::string& peer, const std::vector<marker_t>& robots, std::vector<uint32_t>& timestamps);
    void add(const std::string& peer, const std::vector<obstacle_t>& obstacle, std::vector<uint32_t>& timestamps);
    void add(const std::string& peer, const std::vector<Obstacle*>& obstacle, std::vector<uint32_t>& timestamps);

    void verbose(int verbose);
    void get(std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles, std::vector<uint32_t>& times_robot, std::vector<uint32_t>& times_obstacles);

};

};



#endif //COLLECTOR_H
