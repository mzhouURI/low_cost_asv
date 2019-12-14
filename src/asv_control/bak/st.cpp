/**
 * 该例程将发布chatter话题，消息类型String
 */

#include <sstream>
#include "ros/ros.h"
#include "std_msgs/Int8.h"

int main(int argc, char** argv)
{
  // ROS节点初始化
  ros::init(argc, argv, "swichtest");

  // 创建节点句柄
  ros::NodeHandle n;

  // 创建一个Publisher，发布名为chatter的topic，消息类型为std_msgs::String
  ros::Publisher chatter_pub = n.advertise<std_msgs::Int8>("powerswitch", 8);

  // 设置循环的频率
  ros::Rate loop_rate(10);

  int count = 0;
  while (ros::ok())
  {
    // 初始化std_msgs::String类型的消息
    std_msgs::Int8 msg;
    msg.data = 1;

    // 发布消息
    ROS_INFO("%d", msg.data);
    chatter_pub.publish(msg);

    // 循环等待回调函数
    ros::spinOnce();

    // 按照循环频率延时
    loop_rate.sleep();
    ++count;
  }

  return 0;
}