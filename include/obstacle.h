#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class Obstacle {
    protected:
        cv::Point2f _position;

    public:
        Obstacle(const cv::Point2f& position) : _position(position) {};
        virtual double area() const = 0;
        virtual void draw(cv::Mat& frame, const cv::Matx23f& world2cam_tf) const = 0;
};

class Circle : public Obstacle {
    private:
        double _radius;

    public:
        Circle(const cv::Point2f& position, double radius) :
            Obstacle(position), _radius(radius) {};
        virtual double area() const override;
        virtual void draw(cv::Mat& frame, const cv::Matx23f& world2cam_tf) const override;
};

class Rectangle : public Obstacle {
    private:
        double _orientation;
        double _width;
        double _height;
        std::vector<cv::Point2f> _vertices;

        void pose2vertices(const cv::Point2f& position, double orientation, std::vector<cv::Point2f>& vertices) const;

    public:
        Rectangle(const cv::Point2f& position, double orientation, double width, double height);
        virtual double area() const override;
        virtual void draw(cv::Mat& frame, const cv::Matx23f& world2cam_tf) const override;

};

#endif //OBSTACLE_H
