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
float rpm,pre_rpm;
float pre_rp[4]={0,0,0,0} ;
float rp [4]={0,0,0,0};
float leftrpm = 0.0;
//float test=100.0;
float rightrpm = 0.0;
float set_heading_angle = 0;
int Forward = 0.0;
// get in to the callback when message is received
void rpmCallback(const std_msgs::Float64::ConstPtr& set_rpm)
{
  pre_rp[3]=pre_rp[2];
  pre_rp[2]=pre_rp[1];
  pre_rp[1]=pre_rp[0];
  pre_rp[0] = rp[3];
  rp[3] = rp[2];
  rp[2] = rp[1];
  rp[1] = rp[0];
  rp[0] = set_rpm->data;
  rpm=(rp[0]+rp[1]+rp[2]+rp[3])/4;
  pre_rpm=(pre_rp[0]+pre_rp[1]+pre_rp[2]+pre_rp[3])/4;
  if (pre_rpm == 0.0 && rpm > 0.0)
  {
    Forward = 1;
    sleep(1);
  }
  if (pre_rpm > 0.0 && rpm > 0.0)
  {
    Forward = 2;
  }
  ROS_INFO("rpm: [%f]", rpm);
  ROS_INFO("prerpm: [%f]", pre_rpm);
}

void controlCallback(const std_msgs::Float64::ConstPtr& control_msg_)
{

  if (rpm>0.0)
  {
	//ROS_INFO("aaaaaa");
    //leftrpm = leftrpm - control_msg_->data;
    //rightrpm = rightrpm + control_msg_->data;
    leftrpm = rpm - pre_rpm + leftrpm - control_msg_->data;
    rightrpm = rpm - pre_rpm + rightrpm + control_msg_->data;
  }
  if(leftrpm<0.0)
  {
	  leftrpm=0.0;
  }
  if(leftrpm>1600.0)
  {
	  leftrpm=1600.0;
  }
  if(rightrpm<0.0)
  {
	  rightrpm=0.0;
  }
  if(rightrpm>1600.0)
  {
	  rightrpm=1600.0;
  }
  //test=test+control_msg_->data;
  //ROS_INFO("test: [%f]", test);
  //ROS_INFO("left_rpm: [%f]", leftrpm);
}

void headingCallback(const std_msgs::Float64::ConstPtr& heading_angle)
{
  if (Forward == 1)
  {
    set_heading_angle = heading_angle->data;
  }
  ROS_INFO("Forward: [%d]", Forward);
  ROS_INFO("d_heading: [%f]", set_heading_angle);
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
  ros::Rate loop_rate(10);
  signal(SIGINT, MySigintHandler);
  ros::Subscriber sub3 = heading.subscribe("heading_angle", 100, headingCallback);   // from imu
  ros::Subscriber sub1 = heading.subscribe("diff_rpm", 100, controlCallback);  // from heading controller
  ros::Subscriber sub2 = heading.subscribe("joy_rpm", 100, rpmCallback);       // from joymessage node
  //int count = 1;
  while (ros::ok())
  {
    // subscribe joy joy topic and start callback function  此处名空间要处理清楚
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
