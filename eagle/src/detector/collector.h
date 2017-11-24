#ifndef COLLECTOR_H
#define COLLECTOR_H

#include "robot.h"
#include "obstacle.h"
#include "obstacle_rectangle.h"
#include "obstacle_circle.h"
#include <map>


namespace eagle {

class Collector {

  private:
    std::map<std::string, std::vector<marker_t>> _robot_map;
    std::map<std::string, std::vector<Obstacle*>> _obstacle_map;
    std::map<std::string, std::vector<uint32_t>> _obstacle_time_map;
    std::map<std::string, std::vector<uint32_t>> _robot_time_map;
    void merge_robots(std::vector<Robot*>& robots, std::vector<uint32_t>& timestamps);
    void merge_obstacles(std::vector<Obstacle*>& obstacles, std::vector<uint32_t>& timestamps);

  public:

    Collector();

    void add(const std::string& peer, const std::vector<marker_t>& robots, std::vector<uint32_t>& timestamps);
    void add(const std::string& peer, const std::vector<obstacle_t>& obstacle, std::vector<uint32_t>& timestamps);
    void add(const std::string& peer, const std::vector<Obstacle*>& obstacle, std::vector<uint32_t>& timestamps);

    void robots(std::vector<Robot*>& robots, std::vector<uint32_t>& timestamps);
    void obstacles(std::vector<Obstacle*>& obstacles, std::vector<uint32_t>& timestamps);

};

};



#endif //COLLECTOR_H
