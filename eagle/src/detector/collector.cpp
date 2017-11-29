#include "collector.h"

using namespace eagle;

Collector::Collector() : _verbose(0) {

}

void Collector::add(const std::string& peer, const std::vector<marker_t>& robots, std::vector<uint32_t>& timestamps) {
    // only keep most recent robots
    _robot_map[peer].clear();
    _robot_time_map[peer].clear();
    for (int i = 0; i < robots.size(); i++) {
        int j = 0;
        while (true) {
            if (j == _robot_map[peer].size()) {
                _robot_map[peer].push_back(robots[i]);
                _robot_time_map[peer].push_back(timestamps[i]);
                break;
            }
            if (robots[i].id == _robot_map[peer][j].id) {
                _robot_map[peer][j] = robots[i];
                _robot_time_map[peer][j] = timestamps[i];
                break;
            }
            j++;
        }
    }
}

void Collector::add(const std::string& peer, const std::vector<obstacle_t>& obstacles, std::vector<uint32_t>& timestamps) {
    std::vector<Obstacle*> obst;
    for (int k = 0; k < obstacles.size(); k++) {
        obst.push_back(Obstacle::deserialize(obstacles[k]));
    }
    add(peer, obst, timestamps);
}

void Collector::add(const std::string& peer, const std::vector<Obstacle*>& obstacles, std::vector<uint32_t>& timestamps) {
    // only keep most recent obstacles
    _obstacle_map[peer].clear();
    _obstacle_time_map[peer].clear();
    for (int i = 0; i < obstacles.size(); i++) {
        int j = 0;
        while (true) {
            if (j == _obstacle_map[peer].size()) {
                _obstacle_map[peer].push_back(obstacles[i]);
                _obstacle_time_map[peer].push_back(timestamps[i]);
                break;
            }
            if (cv::norm(obstacles[i]->center() - _obstacle_map[peer][j]->center()) < 0.1) {
                _obstacle_map[peer][j] = obstacles[i];
                _obstacle_time_map[peer][j] = timestamps[i];
                break;
            }
            j++;
        }
    }
}

void Collector::verbose(int verbose) {
    _verbose = verbose;
}

void Collector::get(std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles, std::vector<uint32_t>& times_robot, std::vector<uint32_t>& times_obstacles) {
    merge_data(robots, obstacles, times_robot, times_obstacles);
}

void Collector::merge_data(std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles, std::vector<uint32_t>& times_robot, std::vector<uint32_t>& times_obstacles) {
    // reset
    times_robot.clear();
    times_robot.resize(robots.size());
    for (uint k = 0; k < robots.size(); k++) {
        robots[k]->reset();
    }
    times_obstacles.clear();
    obstacles.clear();
    // put everything together
    std::vector<marker_t> robs;
    std::vector<uint32_t> t_robs;
    for (std::map<std::string, std::vector<marker_t>>::iterator rob = _robot_map.begin(); rob != _robot_map.end(); ++rob) {
        for (uint k = 0; k < rob->second.size(); k++) {
            robs.push_back(rob->second[k]);
            t_robs.push_back(_robot_time_map[rob->first][k]);
        }
    }
    std::vector<Obstacle*> obst;
    std::vector<uint32_t> t_obst;
    for (std::map<std::string, std::vector<Obstacle*>>::iterator obs = _obstacle_map.begin(); obs != _obstacle_map.end(); ++obs) {
        for (uint k = 0; k < obs->second.size(); k++) {
            obst.push_back(obs->second[k]);
            t_obst.push_back(_obstacle_time_map[obs->first][k]);
        }
    }
    // find similar robots and average them
    std::vector<marker_t> sim_robs;
    std::vector<uint32_t> sim_t_robs;
    for (uint i = 0; i < robs.size(); i++) {
        sim_robs.clear();
        for (uint j = i + 1; j < robs.size(); j++) {
            if (robs[i].id == robs[j].id) {
                sim_robs.push_back(robs[j]);
                sim_t_robs.push_back(t_robs[j]);
                robs.erase(robs.begin() + j);
                t_robs.erase(t_robs.begin() + j);
                j--;
            }
        }
        int n_sim = sim_robs.size();
        cv::Point3f translation(robs[i].x, robs[i].y, robs[i].z);
        cv::Point3f rotation(robs[i].roll, robs[i].pitch, robs[i].yaw);
        translation *= (1. / (n_sim + 1));
        rotation *= (1. / (n_sim + 1));
        uint32_t time = t_robs[i] / (n_sim + 1);
        if (_verbose >= 1 && n_sim > 0) {
            std::cout << "merging " << n_sim + 1 << " robots with id " << robs[i].id;
            std::cout << " (" << t_robs[i];
            for (uint k = 0; k < n_sim; k++) {
                std::cout << "," << sim_t_robs[k];
            }
            std::cout << ")" << std::endl;
        }
        for (uint k = 0; k < n_sim; k++) {
            translation += (1. / (n_sim + 1)) * cv::Point3f(sim_robs[k].x, sim_robs[k].y, sim_robs[k].z);
            rotation += (1. / (n_sim + 1)) * cv::Point3f(sim_robs[k].roll, sim_robs[k].pitch, sim_robs[k].yaw);
            time += sim_t_robs[k] / (n_sim + 1);
        }
        for (uint k = 0; k < robots.size(); k++) {
            if (robs[i].id == robots[k]->id()) {
                robots[k]->update(translation, rotation);
                times_robot[k] = time;
            }
        }
    }
    // find similar obstacles and average/discard them
    std::vector<eagle::Obstacle*> sim_obst;
    std::vector<uint32_t> sim_t_obst;
    for (uint i = 0; i < obst.size(); i++) {
        cloud2_t robot_points2;
        bool proceed = true;
        for (uint j = 0; j < robots.size(); j++) {
            if (robots[j]->detected()) {
                robot_points2 = dropz(robots[j]->vertices());
                if (cv::pointPolygonTest(robot_points2, obst[i]->center(), false) >= 0) {
                    if (_verbose >= 1) {
                        std::cout << "discarding obstacle in robot." << std::endl;
                        obst.erase(obst.begin() + i);
                        t_obst.erase(t_obst.begin() + i);
                        i--;
                        proceed = false;
                    }
                }
            }
        }
        if (proceed) {
            cloud2_t obstacle_points2;
            for (uint j = 0; j < obst.size(); j++) {
                if (i != j) {
                    double dst = cv::norm(obst[i]->center() - obst[j]->center());
                    obstacle_points2 = dropz(obst[j]->points());
                    if (dst >= 0.1 && cv::pointPolygonTest(obstacle_points2, obst[i]->center(), false) >= 0) {
                        if (_verbose >= 1) {
                            std::cout << "discarding obstacle in obstacle." << std::endl;
                            obst.erase(obst.begin() + i);
                            t_obst.erase(t_obst.begin() + i);
                            i--;
                            proceed = false;
                        }
                    }
                }
            }
        }
        if (proceed) {
            RectangleObstacle* robst_i = dynamic_cast<RectangleObstacle*>(obst[i]);
            CircleObstacle* cobst_i = dynamic_cast<CircleObstacle*>(obst[i]);
            sim_obst.clear();
            for (uint j = i + 1; j < obst.size(); j++) {
                double dst = cv::norm(obst[i]->center() - obst[j]->center());
                if (dst < 0.1) {
                    RectangleObstacle* robst_j = dynamic_cast<RectangleObstacle*>(obst[j]);
                    if (robst_i != NULL && robst_j != NULL) {
                        sim_obst.push_back(obst[j]);
                        sim_t_obst.push_back(t_obst[j]);
                        obst.erase(obst.begin() + j);
                        t_obst.erase(t_obst.begin() + j);
                        j--;
                    }
                    CircleObstacle* cobst_j = dynamic_cast<CircleObstacle*>(obst[j]);
                    if (cobst_i != NULL && cobst_j != NULL) {
                        sim_obst.push_back(obst[j]);
                        sim_t_obst.push_back(t_obst[j]);
                        obst.erase(obst.begin() + j);
                        t_obst.erase(t_obst.begin() + j);
                        j--;
                    }
                    // I don't know how to handle other cases ...
                }
            }
            if (robst_i != NULL) {
                int n_sim = sim_obst.size();
                cv::Point2f center = (1. / (n_sim + 1)) * robst_i->center();
                cv::Point2f size(robst_i->width(), robst_i->height());
                size *= (1. / (n_sim + 1));
                double angle = (1. / (n_sim + 1)) * robst_i->angle();
                uint32_t time = t_obst[i] / (n_sim + 1);
                if (_verbose >= 1 && n_sim > 0) {
                    std::cout << "merging " << n_sim + 1 << " rectangular obstacles";
                    std::cout << " (" << t_obst[i];
                    for (uint k = 0; k < n_sim; k++) {
                        std::cout << "," << sim_t_obst[k];
                    }
                    std::cout << ")" << std::endl;
                }
                for (uint k = 0; k < n_sim; k++) {
                    RectangleObstacle* robst_j = dynamic_cast<RectangleObstacle*>(sim_obst[k]);
                    center += (1. / (n_sim + 1)) * robst_j->center();
                    size += (1. / (n_sim + 1)) * cv::Point2f(robst_j->width(), robst_j->height());
                    angle += (1. / (n_sim + 1)) * robst_j->angle();
                    time += sim_t_obst[k] / (n_sim + 1);
                }
                obstacles.push_back(new RectangleObstacle(center, size, angle));
                times_obstacles.push_back(time);
            }
            if (cobst_i != NULL) {
                int n_sim = sim_obst.size();
                cv::Point2f center = (1. / (n_sim + 1)) * cobst_i->center();
                double radius = (1. / (n_sim + 1)) * cobst_i->radius();
                uint32_t time = t_obst[i] / (n_sim + 1);
                if (_verbose >= 1 && n_sim > 0) {
                    std::cout << "merging " << n_sim + 1 << " circular obstacles";
                    std::cout << " (" << t_obst[i];
                    for (uint k = 0; k < n_sim; k++) {
                        std::cout << "," << sim_t_obst[k];
                    }
                    std::cout << ")" << std::endl;
                }
                for (uint k = 0; k < n_sim; k++) {
                    CircleObstacle* cobst_j = dynamic_cast<CircleObstacle*>(sim_obst[k]);
                    center += (1. / (n_sim + 1)) * cobst_j->center();
                    radius += (1. / (n_sim + 1)) * cobst_j->radius();
                    time += sim_t_obst[k] / (n_sim + 1);
                }
                obstacles.push_back(new CircleObstacle(center, radius));
                times_obstacles.push_back(time);
            }
        }
    }
}
