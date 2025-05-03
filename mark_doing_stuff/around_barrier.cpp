#include <iostream>
#include <vector>
#include <cmath>
//#include "compute_dist.h"

const double WHEEL_BASE = 120.0; 
const double span = 0.4799655;

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


void move_around(double pixel_x, double pixel_y, bool right){
    std::pair<double, double> target_point =  dist_from_camera(pixel_x, pixel_y); //return in mm
    if(right)target_point.first += 120;
    else target_point.first -= 120;

    if((right && target_point.first < 0) || (!right && target_point.first > 0)){
        //drive forward tartget_point.second amount in MM
        if(right){
            //turn left by ~0.3 radians
        }        
        else{
            //trun right by ~0.3 radians
        }
    }

    else{
        double turn_amount = atan(target_point.first/target_point.second);
        double inner_radius = WHEEL_BASE/sin(span);
        

    }

    
    if((right && target_point.first < 0) || (!right && target_point.first > 0)){
        //drive forward tartget_point.second amount in MM
        if(right){
            //turn left by ~0.3 radians
        }        
        else{
            //trun right by ~0.3 radians
        }
    }

    


}