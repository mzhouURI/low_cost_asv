/**
 * subscribe std_msgs::Joy and control the left motor
 sleep is required to generate pwm signal. To avoid interference between two sigals, seperate nodes are used to deal
 with two motors
 */

#include "ros/ros.h"
#include <sensor_msgs/Joy.h>
extern "C" {
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>  // for atoi() and exit()
#include <rc/time.h>
#include <rc/gpio.h>
#include <rc/adc.h>
#include <signal.h>
#include <rc/dsm.h>
#include <rc/servo.h>
}
int frequency = 50,period;  // set the frequency of the pwm
// get in to the callback when message is received
void ljoyCallback(const sensor_msgs::Joy::ConstPtr& joy)
{
  ROS_INFO("left: [%f]", joy->axes[1]);  // axes[1]left stick updown  axes[3]right stick updown
  float duty;
  //int period = 1000000 / frequency;
  if (joy->axes[1] > 0)
  {
    // set the direction of the motor to be forward
    rc_gpio_set_value(1, 25, 0);
    // rc_gpio_set_value(1, 17, 0);
    // set the duty
    duty = joy->axes[1];
  }
  if (joy->axes[1] < 0)
  {
    // set the direction of the motor to be backward
    rc_gpio_set_value(1, 25, 1);
    // rc_gpio_set_value(1, 17, 1);
    // set the duty
    duty = -joy->axes[1];
  }
  // send pwm adjust command
  rc_servo_send_pulse_us(1, duty * period);
  rc_usleep(period);
  // print the received data
}

void rjoyCallback(const sensor_msgs::Joy::ConstPtr& joy)
{
  ROS_INFO("right: [%f]", joy->axes[3]);  // axes[1]left stick updown  axes[3]right stick updown
  float duty;
  int period = 1000000 / frequency;
  if (joy->axes[3] > 0)
  {
    // set the direction of the motor to be forward
    // rc_gpio_set_value(1, 25, 0);
    rc_gpio_set_value(1, 17, 0);
    // set the duty
    duty = joy->axes[3];
  }
  if (joy->axes[3] < 0)
  {
    // set the direction of the motor to be backward
    // rc_gpio_set_value(1, 25, 1);
    rc_gpio_set_value(1, 17, 1);
    // set the duty
    duty = -joy->axes[3];
  }
  // send pwm adjust command
  rc_servo_send_pulse_us(2, duty * period);
  rc_usleep(period);
  // print the received data
}

void MySigintHandler(int sig)
{
  //这里主要进行退出前的数据保存、内存清理、告知其他节点等工作
  ROS_INFO("shutting down!");
  rc_servo_cleanup();
  rc_gpio_cleanup(1, 25);
  rc_gpio_cleanup(1, 17);
  ros::shutdown();
}

int main(int argc, char** argv)
{
  // initiate pwm;
  if(rc_servo_init()) return -1;
  ROS_INFO("servo_init");
  usleep(100000);
  // initiate 4 gpios
  if (rc_gpio_init(1, 25, GPIOHANDLE_REQUEST_OUTPUT) == -1)
  {
    printf("rc_gpio_init failed\n");
    return -1;
  }
  if (rc_gpio_init(1, 17, GPIOHANDLE_REQUEST_OUTPUT) == -1)
  {
    printf("rc_gpio_init failed\n");
    return -1;
  }
  /*          if(rc_gpio_init(3, 20, GPIOHANDLE_REQUEST_OUTPUT)==-1){
     printf("rc_gpio_init failed\n");
     return -1;
     }
              if(rc_gpio_init(3, 17, GPIOHANDLE_REQUEST_OUTPUT)==-1){
     printf("rc_gpio_init failed\n");
     return -1;
     } */
  ros::init(argc, argv, "motor_joy");
  ros::NodeHandle node_priv("~");
  //ROS_INFO("f111111111: [%d]", frequency);
  //sleep(5);
  node_priv.getParam("frequency", frequency);
  //ROS_INFO("f222222222: [%d]", frequency);
  //sleep(5);
  period = 1000000 / frequency;
  // ros::init(argc, argv, "rightmotor");
  ros::NodeHandle motor_joy;
  signal(SIGINT, MySigintHandler);
  // subscribe joy joy topic and start callback function
  ros::Subscriber lsub = motor_joy.subscribe<sensor_msgs::Joy>("joy", 10, ljoyCallback);
  ros::Subscriber rsub = motor_joy.subscribe<sensor_msgs::Joy>("joy", 10, rjoyCallback);
  ros::spin();
  return 0;
}
