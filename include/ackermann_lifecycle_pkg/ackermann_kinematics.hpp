#ifndef ACKERMANN_LIFECYCLE_PKG__ACKERMANN_KINEMATICS_HPP_
#define ACKERMANN_LIFECYCLE_PKG__ACKERMANN_KINEMATICS_HPP_

#include <cmath>
#include <limits>

class AckermannKinematics
{
public:
    AckermannKinematics(double wheelbase, double track_width_front)
        : wheelbase_(wheelbase), track_width_front_(track_width_front){}
    double computeLeftSteeringAngle(double phi) const {
        if (std::abs(phi) < 1e-9) {
            return 0.0; // Straight line, no steering angle
        }
        else {
            auto R = wheelbase_ / std::tan(phi);
            return std::atan(wheelbase_ / (R - track_width_front_ / 2.0));
        }
    }
    double computeRightSteeringAngle(double phi) const {
        if (std::abs(phi) < 1e-9) {
            return 0.0; // Straight line, no steering angle
        }
        else {
            auto R = wheelbase_ / std::tan(phi);
            return std::atan(wheelbase_ / (R + track_width_front_ / 2.0));
        }
    }
    double turningRadius(double phi) const {
        if (std::abs(phi) < 1e-9){
            return std::numeric_limits<double>::infinity(); // Straight line, infinite turning radius
        }
        else {
            return wheelbase_ / std::tan(phi);
        }
    }
    double computeCentralAngle(double phi_left, double phi_right) const {
        if (std::abs(phi_left) < 1e-9 && std::abs(phi_right) < 1e-9){
            return 0.0; // Straight line, no steering angle
        }
        else {
            auto R_left = wheelbase_ / std::tan(phi_left);
            auto R_right = wheelbase_ / std::tan(phi_right);
            auto R_center = (R_left + R_right) / 2.0;
            return std::atan(wheelbase_ / R_center);
        }
    }
private:
    double wheelbase_;
    double track_width_front_;
};


#endif  // ACKERMANN_LIFECYCLE_PKG__ACKERMANN_KINEMATICS_HPP_