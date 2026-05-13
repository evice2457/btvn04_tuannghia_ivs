#include <gtest/gtest.h>
#include "ackermann_lifecycle_pkg/ackermann_kinematics.hpp"

TEST(AckermannKinematicsTest, ZeroSteeringAngle) {
  AckermannKinematics kinematics(2.5, 1.5);
  EXPECT_NEAR(kinematics.computeLeftSteeringAngle(0.0), 0.0, 1e-5);
  EXPECT_NEAR(kinematics.computeRightSteeringAngle(0.0), 0.0, 1e-5);
}

TEST(AckermannKinematicsTest, LeftTurn) {
  AckermannKinematics kinematics(2.5, 1.5);
  double phi = 0.1;   // Example steering angle
  EXPECT_GT(kinematics.computeLeftSteeringAngle(phi), kinematics.computeRightSteeringAngle(phi));
}

TEST(AckermannKinematicsTest, Symmetry) {
  AckermannKinematics kinematics(2.5, 1.5);
  double phi = 0.1;
  EXPECT_NEAR(
    kinematics.computeLeftSteeringAngle(phi), (-kinematics.computeRightSteeringAngle(
      -phi)), 1e-5);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
