#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "protocol.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace eagle {

    class Obstacle {
        protected:
            cv::Point2f _position;

        public:
            Obstacle(const cv::Point2f& position) : _position(position) {};
            virtual double area() const = 0;
            virtual void draw(cv::Mat& frame, const cv::Matx23f& world2cam_tf) const = 0;
            virtual eagle::obstacle_t serialize() const = 0;
    };

    class Circle : public Obstacle {
        private:
            double _radius;

        public:
            Circle(const cv::Point2f& position, double radius) :
                Obstacle(position), _radius(radius) {};

            virtual double area() const override {
                return M_PI*pow(_radius, 2);
            }

            virtual void draw(cv::Mat& frame, const cv::Matx23f& world2cam_tf) const override {
                cv::Scalar gray(77, 76, 75);
                std::vector<cv::Point2f> position_cam;
                cv::transform(std::vector<cv::Point2f>{_position}, position_cam, world2cam_tf);
                cv::circle(frame, position_cam[0], _radius*world2cam_tf(0,0), gray, 2);
            }

            virtual eagle::obstacle_t serialize() const override {
                eagle::obstacle_t ret;
                ret.id = 0;
                ret.shape = eagle::CIRCLE;
                ret.p1 = {_position.x, _position.y};
                ret.p2 = {_position.x+_radius, _position.y};
                ret.p3 = {_position.x, _position.y+_radius};
                return ret;
            }
    };

    class Rectangle : public Obstacle {
        private:
            double _orientation;
            double _width;
            double _height;
            std::vector<cv::Point2f> _vertices;
            cv::RotatedRect _box;

            void pose2vertices(const cv::Point2f& position, double orientation,
                std::vector<cv::Point2f>& vertices) const {
                cv::Point2f vert[4];
                cv::RotatedRect box(_position, cv::Size2f(_width, _height), (180./M_PI)*_orientation);
                box.points(vert);
                vertices = std::vector<cv::Point2f>(vert, vert+4);
            }

        public:

            Rectangle(const cv::Point2f& position,
                double orientation, double width, double height) :
                Obstacle(position), _orientation(orientation), _width(width), _height(height) {
                pose2vertices(position, orientation, _vertices);
            };

            double area() const {
                return _width*_height;
            }


            void draw(cv::Mat& frame, const cv::Matx23f& world2cam_tf) const {
                cv::Scalar gray(77, 76, 75);
                std::vector<cv::Point2f> vertices_cam;
                cv::transform(_vertices, vertices_cam, world2cam_tf);
                int n = vertices_cam.size();
                for (uint i=0; i<n; i++) {
                    cv::line(frame, vertices_cam[i], vertices_cam[(i+1)%n], gray, 2);
                }
            }

            eagle::obstacle_t serialize() const {
                eagle::obstacle_t ret;
                ret.id = 0;
                ret.shape = eagle::RECTANGLE;
                ret.p1 = {_position.x-0.5*_width*cos(_orientation)+0.5*_height*sin(_orientation),
                    _position.y-0.5*_width*sin(_orientation)-0.5*_height*cos(_orientation)};
                ret.p2 = {_position.x+0.5*_width*cos(_orientation)+0.5*_height*sin(_orientation),
                    _position.y+0.5*_width*sin(_orientation)-0.5*_height*cos(_orientation)};
                ret.p3 = {_position.x+0.5*_width*cos(_orientation)-0.5*_height*sin(_orientation),
                    _position.y+0.5*_width*sin(_orientation)+0.5*_height*cos(_orientation)};
                return ret;
            }
    };

};

#endif //OBSTACLE_H
