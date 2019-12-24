/**
 * subscribe std_msgs::Joy and control the left motor
 sleep is required to generate pwm signal. To avoid interference between two sigals, seperate nodes are used to deal
 with two motors
 */

#include "ros/ros.h"
//#include <std_msgs/Float64MultiArray.h>
#include "std_msgs/Float64.h"
#include <sensor_msgs/Joy.h>
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
float rpm1,rpm2,rpm3,rpm4,l_leftrpm,l_rightrpm,r_rightrpm,r_leftrpm;
int leftTurn=0,rightTurn=0,Start=0;
float leftrpm = 0.0;
float leftrpm_total = 0.0;//sum of the base part and control part
//float test=100.0;
float rightrpm = 0.0;
float rightrpm_total = 0.0;
float set_heading_angle = 0.0;
int Forward = 0;
int pre_Forward=0;
float baserpm=0;
int pre_b[4];
int count=0;
// get in to the callback when message is received
//Totally 4 gears. Y:add speed, A: reduce speed, X: left turn, B: right turn
void rpmCallback(const sensor_msgs::Joy::ConstPtr& joy)
{
  //ROS_INFO("button3: [%d]", joy->buttons[3]);
  if(count>100&&Start==1){   //keep the value 1 of Start for a few loops
	  Start=0;
  }
  pre_Forward=Forward;
  if (pre_b[3]==0 && joy->buttons[3]==1) //activate at the instance when the button is pushed.
  {
    rightTurn=0;
    leftTurn=0;
    Forward ++;
    if(Forward>4)
    {
      Forward=4;
    }
  }
  if (pre_b[0]==0 && joy->buttons[0]==1)
  {
    rightTurn=0;
    leftTurn=0;
    Forward --;
    if (Forward<0)
    {
      Forward=0;
    }
  }
  switch (Forward) {  //set the basic rpm
    case 1:
    baserpm=rpm1;
    if(pre_Forward==0)
    {
      Start=1;//start signal
      count=0;
      ROS_INFO("aaaaaa");
    }
    break;
    case 2:
    baserpm=rpm2;
    break;
    case 3:
    baserpm=rpm3;
    break;
    case 4:
    baserpm=rpm4;
    break;
  }
  if (pre_b[2]==0 &&joy->buttons[2]==1)
  {
    leftTurn=1;
    Forward=0;
    rightTurn=0;
  }
  if (pre_b[1]==0 &&joy->buttons[1]==1)
  {
    rightTurn=1;
    Forward=0;
    leftTurn=0;
  }
  pre_b[0] = joy->buttons[0];
  pre_b[1] = joy->buttons[1];
  pre_b[2] = joy->buttons[2];
  pre_b[3] = joy->buttons[3];
  count++;
  //ROS_INFO("count: [%d]", count);
  //ROS_INFO("Forward: [%d]", Forward);
  //ROS_INFO("leftTurn: [%d]", leftTurn);
  //ROS_INFO("rightTurn: [%d]", rightTurn);
}

void controlCallback(const std_msgs::Float64::ConstPtr& control_msg_)
{
  if (Forward==0 && rightTurn==0 && leftTurn==0)
  {
	  leftrpm_total=rightrpm_total=0;
  }
  if (Forward>0)
  {
	//ROS_INFO("aaaaaa");
    //leftrpm = leftrpm - control_msg_->data;
    //rightrpm = rightrpm + control_msg_->data;
    leftrpm = leftrpm - control_msg_->data;
    rightrpm = rightrpm + control_msg_->data;
    leftrpm_total=leftrpm+baserpm;
    rightrpm_total=rightrpm+baserpm;
    //set the limit of rpm
    if(leftrpm_total<0.0)
    {
  	  leftrpm_total=0.0;
    }
    if(leftrpm_total>1600.0)
    {
  	  leftrpm_total=1600.0;
    }
    if(rightrpm_total<0.0)
    {
  	  rightrpm_total=0.0;
    }
    if(rightrpm_total>1600.0)
    {
  	  rightrpm_total=1600.0;
    }
  }
  if (leftTurn==1)
  {
    leftrpm_total=l_leftrpm;
    rightrpm_total=l_rightrpm;
  }
  if (rightTurn==1)
  {
    leftrpm_total=r_leftrpm;
    rightrpm_total=r_rightrpm;
  }
  //test=test+control_msg_->data;
  //ROS_INFO("test: [%f]", test);
  ROS_INFO("left_rpm: [%f]", leftrpm_total);
  ROS_INFO("right_rpm: [%f]", rightrpm_total);
}

void headingCallback(const std_msgs::Float64::ConstPtr& heading_angle)
{
  if (Start == 1)
  {
    set_heading_angle = heading_angle->data;
  }
  ROS_INFO("Start: [%d]", Start);
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
  ros::NodeHandle node_priv("~");
 // float rpm1,rpm2,rpm3,rpm4,l_leftrpm,l_rightrpm,r_rightrpm,r_leftrpm;
  node_priv.getParam("rpm1", rpm1);
  node_priv.getParam("rpm2", rpm2);
  node_priv.getParam("rpm3", rpm3);
  node_priv.getParam("rpm4", rpm4);
  node_priv.getParam("l_leftrpm", l_leftrpm);
  node_priv.getParam("l_rightrpm", l_rightrpm);
  node_priv.getParam("r_leftrpm", r_leftrpm);
  node_priv.getParam("r_rightrpm", r_rightrpm);
  ros::Publisher pub1 = heading.advertise<std_msgs::Float64>("d_leftrpm", 100);
  ros::Publisher pub2 = heading.advertise<std_msgs::Float64>("d_rightrpm", 100);
  ros::Publisher pub3 = heading.advertise<std_msgs::Float64>("d_heading", 100);
  ros::Rate loop_rate(10);
  signal(SIGINT, MySigintHandler);
  ros::Subscriber sub3 = heading.subscribe("heading_angle", 100, headingCallback);   // from imu
  ros::Subscriber sub1 = heading.subscribe("diff_rpm", 100, controlCallback);  // from heading controller
  ros::Subscriber sub2 = heading.subscribe("joy", 100, rpmCallback);       // from joymessage node
  //int count = 1;
  while (ros::ok())
  {
    // subscribe joy joy topic and start callback function  此处名空间要处理清楚
    d_leftrpm_msg.data = leftrpm_total;
    d_rightrpm_msg.data = rightrpm_total;
    d_heading_msg.data = set_heading_angle;
    pub1.publish(d_leftrpm_msg);   // to left controller
    pub2.publish(d_rightrpm_msg);  // to right controller
    if(set_heading_angle!=0.0)
    {
    pub3.publish(d_heading_msg);   // to heading controller,only when the angle is set.
    }
    ros::spinOnce();
    loop_rate.sleep();
    //count = ++;
  }
  return 0;
}
