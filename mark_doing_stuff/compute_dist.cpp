#include <iostream>
#include <vector>
#include <cmath>

const double y_dim = 720;
const double x_dim = 1080; 
constexpr double camera_min_vertical_view = 44.2559408  / 180 * M_PI;
const double camera_height = 11.7;
constexpr double fov_horz = 53.5 / 180 * M_PI;
constexpr double fov_vert = 41.41 / 180 * M_PI;

std::pair<double, double> dist_from_camera(double pixel_x, double pixel_y){
    std::pair<double, double> location;
    //std::cout<<fov_vert<<" ";
    double alpha = camera_min_vertical_view + fov_vert/2 + atan(tan(fov_vert/2) * (pixel_y - y_dim/2)/(y_dim/2));
    double beta = atan(tan(fov_horz/2) * (pixel_x - x_dim/2)/(x_dim/2));
    //std::cout<<alpha<<" "<<beta<<" ";
    location.first = tan(beta) * camera_height / cos(alpha);
    location.second = tan(alpha) * camera_height;
    return location;
}



struct Point {
    double x;
    double y;
};

std::pair<double, double> computeRegressionLine(const std::vector<Point>& points) {
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    int n = points.size();

    for (const auto& point : points) {
        sumX += point.x;
        sumY += point.y;
        sumXY += point.x * point.y;
        sumX2 += point.x * point.x;
    }

    double slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    double intercept = (sumY - slope * sumX) / n;

    return {slope, intercept};
}

int main() {







    std::vector<Point> points = {
        {1, 2}, {2, 3}, {3, 5}, {4, 7}, {5, 11}
    }; //
    std::pair<double, double> test = dist_from_camera(100, 360);
    std::cout<<test.first<<" "<<test.second<<"\n";
    std::pair<double, double> res = computeRegressionLine(points);
    double slope = res.first;
    double intercept = res.second;
    std::cout << "Regression Line: y = " << slope << "x + " << intercept << std::endl;

    return 0;
}