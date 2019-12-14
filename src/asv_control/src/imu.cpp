/**
 * 该例程将发布chatter话题，消息类型String
 */

#include <sstream>
#include "ros/ros.h"
extern "C"  // required when building C code
{
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>  // for atoi() and exit()
#include <rc/mpu.h>
#include <rc/time.h>
}
#include "std_msgs/Float64.h"
//#include "sensor_msgs/MagneticField.h"
#include "sensor_msgs/Imu.h"
#include "geometry_msgs/Quaternion.h"
//#include "geometry_msgs/Vector3.h"
#define I2C_BUS 2
#define GPIO_INT_PIN_CHIP 3
#define GPIO_INT_PIN_PIN 21
rc_mpu_data_t data;

int main(int argc, char** argv)
{
  // initialize a ros node (name used in ros context)
  ros::init(argc, argv, "imu");
  // create a handle of the node (name used in the file)
  ros::NodeHandle dmp;
  // create a Publisher and publish a string topic named heading
  ros::Publisher imu_pub = dmp.advertise<sensor_msgs::Imu>("imu", 1000);
  ros::Publisher heading_pub = dmp.advertise<std_msgs::Float64>("heading", 100);
  // set the frequency. It should be conbined with spinOnce().
  ros::Rate loop_rate(100);
  ////configuration of the imu
  rc_mpu_config_t conf = rc_mpu_default_config();
  conf.i2c_bus = I2C_BUS;
  conf.gpio_interrupt_pin_chip = GPIO_INT_PIN_CHIP;
  conf.gpio_interrupt_pin = GPIO_INT_PIN_PIN;
  conf.dmp_sample_rate = 100;
  conf.enable_magnetometer = 1;
  ////configuration of the imu
  // initialize the imu
  if (rc_mpu_initialize_dmp(&data, conf))
  {
    printf("rc_initialize_imu_failed\n");
    return -1;
  }
  // set the serial number of the output data
  // int count = 0;
  while (ros::ok())
  {
    //// get the heading data and publish it by the publisher
    sensor_msgs::Imu imu_data;
    std_msgs::Float64 heading_data;
    //std::stringstream s1, s2, s3, s4, s5, s6, s7, s8, s9;
    imu_data.orientation.x = data.fused_TaitBryan[TB_PITCH_X] * RAD_TO_DEG;
    imu_data.orientation.y = data.fused_TaitBryan[TB_ROLL_Y] * RAD_TO_DEG;
    imu_data.orientation.z = data.fused_TaitBryan[TB_YAW_Z] * RAD_TO_DEG;
    imu_data.linear_acceleration.x = data.accel[0];
    imu_data.linear_acceleration.y = data.accel[1];
    imu_data.linear_acceleration.z = data.accel[2];
    imu_data.angular_velocity.x = data.gyro[0];
    imu_data.angular_velocity.y = data.gyro[1];
    imu_data.angular_velocity.z = data.gyro[2];
    heading_data.data = data.compass_heading;
    ROS_INFO("imu_data.orientation.z:%f", imu_data.orientation.z);
    ROS_INFO("heading_data.data:%f", heading_data.data);
    imu_pub.publish(imu_data);
    heading_pub.publish(heading_data);
    //// get the heading data and publish it by the publisher
    // circulate at the set rate
    ros::spinOnce();
    loop_rate.sleep();
    //++count;
  }
  return 0;
}
