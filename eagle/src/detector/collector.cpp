#include "collector.h"

using namespace eagle;

Collector::Collector() {

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


void Collector::robots(std::vector<Robot*>& robots, std::vector<uint32_t>& timestamps) {
    merge_robots(robots, timestamps);

}

void Collector::obstacles(std::vector<Obstacle*>& obstacles, std::vector<uint32_t>& timestamps) {
    merge_obstacles(obstacles, timestamps);
}

void Collector::merge_robots(std::vector<Robot*>& robots, std::vector<uint32_t>& timestamps) {
    timestamps.clear();
    timestamps.resize(robots.size());
    std::vector<marker_t> robs;
    std::vector<uint32_t> tims;
    for (std::map<std::string, std::vector<marker_t>>::iterator rob = _robot_map.begin(); rob != _robot_map.end(); ++rob) {
        for (uint k = 0; k < rob->second.size(); k++) {
            robs.push_back(rob->second[k]);
            tims.push_back(_robot_time_map[rob->first][k]);
        }
    }
    std::vector<marker_t> similars;
    std::vector<uint32_t> sim_times;
    for (uint i = 0; i < robs.size(); i++) {
        similars.clear();
        for (uint j = i + 1; j < robs.size(); j++) {
            if (robs[i].id == robs[j].id) {
                similars.push_back(robs[j]);
                sim_times.push_back(tims[j]);
                robs.erase(robs.begin() + j);
                tims.erase(tims.begin() + j);
                j--;
            }
        }
        int n_sim = similars.size();
        cv::Point3f translation(robs[i].x, robs[i].y, robs[i].z);
        cv::Point3f rotation(robs[i].roll, robs[i].pitch, robs[i].yaw);
        translation *= (1. / (n_sim + 1));
        rotation *= (1. / (n_sim + 1));
        uint32_t time = tims[i] / (n_sim + 1);
        for (uint k = 0; k < n_sim; k++) {
            translation += (1. / (n_sim + 1)) * cv::Point3f(similars[k].x, similars[k].y, similars[k].z);
            rotation += (1. / (n_sim + 1)) * cv::Point3f(similars[k].roll, similars[k].pitch, similars[k].yaw);
            time += sim_times[k] / (n_sim + 1);
        }
        for (uint k = 0; k < robots.size(); k++) {
            if (robs[i].id == robots[k]->id()) {
                robots[k]->update(translation, rotation);
                timestamps[k] = time;
            }
        }
    }
}


void Collector::merge_obstacles(std::vector<Obstacle*>& obstacles, std::vector<uint32_t>& timestamps) {
    timestamps.clear();
    obstacles.clear();
    std::vector<Obstacle*> obst;
    std::vector<uint32_t> tims;
    for (std::map<std::string, std::vector<Obstacle*>>::iterator obs = _obstacle_map.begin(); obs != _obstacle_map.end(); ++obs) {
        for (uint k = 0; k < obs->second.size(); k++) {
            obst.push_back(obs->second[k]);
            tims.push_back(_obstacle_time_map[obs->first][k]);
        }
    }
    std::vector<eagle::Obstacle*> similars;
    std::vector<uint32_t> sim_times;
    for (uint i = 0; i < obst.size(); i++) {
        RectangleObstacle* robst_i = dynamic_cast<RectangleObstacle*>(obst[i]);
        CircleObstacle* cobst_i = dynamic_cast<CircleObstacle*>(obst[i]);
        similars.clear();
        for (uint j = i + 1; j < obst.size(); j++) {
            double dst = cv::norm(obst[i]->center() - obst[j]->center());
            if (dst < 0.1) {
                RectangleObstacle* robst_j = dynamic_cast<RectangleObstacle*>(obst[j]);
                if (robst_i != NULL && robst_j != NULL) {
                    similars.push_back(obst[j]);
                    sim_times.push_back(tims[j]);
                    obst.erase(obst.begin() + j);
                    tims.erase(tims.begin() + j);
                    j--;
                }
                CircleObstacle* cobst_j = dynamic_cast<CircleObstacle*>(obst[j]);
                if (cobst_i != NULL && cobst_j != NULL) {
                    similars.push_back(obst[j]);
                    sim_times.push_back(tims[j]);
                    obst.erase(obst.begin() + j);
                    tims.erase(tims.begin() + j);
                    j--;
                }
                // I don't know how to handle other cases ...
            }
        }
        if (robst_i != NULL) {
            int n_sim = similars.size();
            cv::Point2f center = (1. / (n_sim + 1)) * robst_i->center();
            cv::Point2f size(robst_i->width(), robst_i->height());
            size *= (1. / (n_sim + 1));
            double angle = (1. / (n_sim + 1)) * robst_i->angle();
            uint32_t time = tims[i] / (n_sim + 1);
            for (uint k = 0; k < n_sim; k++) {
                RectangleObstacle* robst_j = dynamic_cast<RectangleObstacle*>(similars[k]);
                center += (1. / (n_sim + 1)) * robst_j->center();
                size += (1. / (n_sim + 1)) * cv::Point2f(robst_j->width(), robst_j->height());
                angle += (1. / (n_sim + 1)) * robst_j->angle();
                time += sim_times[k] / (n_sim + 1);
            }
            obstacles.push_back(new RectangleObstacle(center, size, angle));
            timestamps.push_back(time);
        }
        if (cobst_i != NULL) {
            int n_sim = similars.size();
            cv::Point2f center = (1. / (n_sim + 1)) * cobst_i->center();
            double radius = (1. / (n_sim + 1)) * cobst_i->radius();
            uint32_t time = tims[i] / (n_sim + 1);
            for (uint k = 0; k < n_sim; k++) {
                CircleObstacle* cobst_j = dynamic_cast<CircleObstacle*>(similars[k]);
                center += (1. / (n_sim + 1)) * cobst_j->center();
                radius += (1. / (n_sim + 1)) * cobst_j->radius();
                time += sim_times[k] / (n_sim + 1);
            }
            obstacles.push_back(new CircleObstacle(center, radius));
            timestamps.push_back(time);
        }
    }
}
