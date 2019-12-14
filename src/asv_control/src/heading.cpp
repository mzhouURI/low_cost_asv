/**
 * subscribe std_msgs::Joy and control the left motor
 sleep is required to generate pwm signal. To avoid interference between two sigals, seperate nodes are used to deal
 with two motors
 */

#include "ros/ros.h"
//#include <std_msgs/Float64MultiArray.h>
#include "std_msgs/Float64.h"
//#include "sensor_msgs/Imu.h"
extern "C" {
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>  // for atoi() and exit()
#include <rc/time.h>
#include <rc/adc.h>
#include <signal.h>
}
float pre_rpm = 0;
float rpm = 0;
float leftrpm = 0;
float rightrpm = 0;
float set_heading_angle = 0;
int Forward = 0;
// get in to the callback when message is received
void rpmCallback(const std_msgs::Float64::ConstPtr& set_rpm)
{
  ROS_INFO("set_rpm: [%f]", set_rpm->data);
  pre_rpm = rpm;
  rpm = set_rpm->data;
  if (pre_rpm == 0 && rpm > 0)
  {
    Forward = 1;
  }
  if (pre_rpm > 0 && rpm > 0)
  {
    Forward = 2;
  }
}

void controlCallback(const std_msgs::Float64::ConstPtr& control_msg_)
{
  ROS_INFO("diff_rpm: [%f]", control_msg_->data);
  if (rpm != 0)
  {
    leftrpm = rpm - pre_rpm + leftrpm + control_msg_->data;
    rightrpm = rpm - pre_rpm + rightrpm - control_msg_->data;
  }
}

void headingCallback(const std_msgs::Float64::ConstPtr& heading_angle)
{
  if (Forward == 1)
  {
    set_heading_angle = heading_angle->data;
  }
}

void MySigintHandler(int sig)
{
  //这里主要进行退出前的数据保存、内存清理、告知其他节点等工作
  ROS_INFO("shutting down!");
  ros::shutdown();
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "headingcontrol");
  std_msgs::Float64 d_leftrpm_msg, d_rightrpm_msg, d_heading_msg;
  // ros::init(argc, argv, "rightmotor");
  ros::NodeHandle heading;
  ros::Publisher pub1 = heading.advertise<std_msgs::Float64>("d_leftrpm", 100);
  ros::Publisher pub2 = heading.advertise<std_msgs::Float64>("d_rightrpm", 100);
  ros::Publisher pub3 = heading.advertise<std_msgs::Float64>("d_heading", 100);
  ros::Rate loop_rate(100);
  signal(SIGINT, MySigintHandler);
  //int count = 1;
  while (ros::ok())
  {
    // subscribe joy joy topic and start callback function  此处名空间要处理清楚
    ros::Subscriber sub1 = heading.subscribe("diff_rpm", 100, controlCallback);  // from heading controller
    ros::Subscriber sub2 = heading.subscribe("joy_rpm", 100, rpmCallback);       // from joymessage node
    ros::Subscriber sub3 = heading.subscribe("heading", 100, headingCallback);   // from imu
    d_leftrpm_msg.data = leftrpm;
    d_rightrpm_msg.data = rightrpm;
    d_heading_msg.data = set_heading_angle;
    pub1.publish(d_leftrpm_msg);   // to left controller
    pub2.publish(d_rightrpm_msg);  // to right controller
    pub3.publish(d_heading_msg);   // to heading controller
    ros::spinOnce();
    loop_rate.sleep();
    //count = ++;
  }
  return 0;
}
