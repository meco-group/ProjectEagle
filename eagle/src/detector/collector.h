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
    std::map<std::string, std::vector<unsigned long>> _obstacle_time_map;
    std::map<std::string, std::vector<unsigned long>> _robot_time_map;
    void merge_robots(std::vector<Robot*>& robots, std::vector<unsigned long>& timestamps);
    void merge_obstacles(std::vector<Obstacle*>& obstacles, std::vector<unsigned long>& timestamps);

  public:

    Collector();

    void add(const std::string& peer, const std::vector<marker_t>& robots, std::vector<unsigned long>& timestamps);
    void add(const std::string& peer, const std::vector<obstacle_t>& obstacle, std::vector<unsigned long>& timestamps);
    void add(const std::string& peer, const std::vector<Obstacle*>& obstacle, std::vector<unsigned long>& timestamps);

    void robots(std::vector<Robot*>& robots, std::vector<unsigned long>& timestamps);
    void obstacles(std::vector<Obstacle*>& obstacles, std::vector<unsigned long>& timestamps);

};

};



#endif //COLLECTOR_H
