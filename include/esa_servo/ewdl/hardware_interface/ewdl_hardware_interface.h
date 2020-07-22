#ifndef ESA_EWDL_HARDWARE_INTERFACE_H
#define ESA_EWDL_HARDWARE_INTERFACE_H
// STL
#include <string>
#include <vector>

// roscpp
#include <ros/ros.h>
#include <ros/console.h>
// std_srvs
#include <std_srvs/Trigger.h>
// controller_manager
#include <controller_manager/controller_manager.h>
// hardware_interface
#include <hardware_interface/actuator_state_interface.h>
#include <hardware_interface/actuator_command_interface.h>
#include <hardware_interface/robot_hw.h>
// xmlrpcpp
#include <XmlRpcValue.h>
#include <XmlRpcException.h>

// esa_servo
#include "esa_servo/ewdl/ethercat/master.h"


namespace esa { namespace ewdl {

static const double POSITION_STEP_FACTOR = 10000.0;
static const double VELOCITY_STEP_FACTOR = 10000.0;


class ServoHW : public hardware_interface::RobotHW {
private:

  esa::ewdl::ethercat::Master ec_master;

protected:

  ros::NodeHandle node;

  double loop_hz;
  std::vector<std::string> joints;
  std::vector<std::string> actuators;

  //
  hardware_interface::ActuatorStateInterface act_state_interface;
  hardware_interface::PositionActuatorInterface pos_act_interface;
  hardware_interface::VelocityActuatorInterface vel_act_interface;

  //
  std::vector<double> a_pos, a_pos_cmd;
  std::vector<double> a_vel, a_vel_cmd;
  std::vector<double> a_eff, a_eff_cmd;

  //
  std::vector<double> joint_lower_limits;
  std::vector<double> joint_upper_limits;

public:

  bool reset_controllers = false;


  ServoHW(ros::NodeHandle &node) : node(node)
  {

  }


  bool init_ethercat(const std::string &ifname, const std::vector<std::string> &slaves)
  {
    ec_master = esa::ewdl::ethercat::Master(ifname, slaves);

    if (ec_master.init())
    {
      ROS_INFO("EtherCAT Master interface: %s", ifname.c_str());
    }
    else
    {
      ROS_FATAL("Failed to initialize EtherCAT master.");
      return false;
    }

    for (int i = 0; i < slaves.size(); i++)
    {
      ROS_INFO("EtherCAT Slave[%d]: %s", 1+i, slaves[i].c_str());
    }

    return true;
  }


  bool config_slaves(XmlRpc::XmlRpcValue &slaves_param)
  {
    for (int i = 0; i < slaves_param.size(); i++)
    {
      try
      {
        int homing_method = slaves_param[i]["homing_method"];
        int homing_speed_to_switch = slaves_param[i]["homing_speed"][0];
        int homing_speed_to_zero = slaves_param[i]["homing_speed"][1];
        int homing_acceleration = slaves_param[i]["homing_acceleration"];

        int home_offset = slaves_param[i]["home_offset"];
        int home_switch = slaves_param[i]["home_switch"];

        int quickstop_deceleration = slaves_param[i]["quickstop_deceleration"];

        const uint16 slave_idx = 1 + i;
        ROS_DEBUG("EtherCAT Slave[%d] Homing Method: %d", slave_idx, homing_method);
        ROS_DEBUG("EtherCAT Slave[%d] Homing Speed: %d %d", slave_idx, homing_speed_to_switch, homing_speed_to_zero);
        ROS_DEBUG("EtherCAT Slave[%d] Homing Acceleration: %d", slave_idx, homing_acceleration);
        ROS_DEBUG("EtherCAT Slave[%d] Home Offset: %d", slave_idx, home_offset);
        ROS_DEBUG("EtherCAT Slave[%d] Home Switch: 0x%.2x", slave_idx, home_switch);
        ROS_DEBUG("EtherCAT Slave[%d] QuickStop Deceleration: %u", slave_idx, quickstop_deceleration);

        ec_master.config_homing(slave_idx, homing_method, homing_speed_to_switch, homing_speed_to_zero, homing_acceleration, home_offset, home_switch);
        ec_master.config_quickstop(slave_idx, quickstop_deceleration);
      }
      catch (const XmlRpc::XmlRpcException &ex)
      {
        auto code = ex.getCode();
        auto message = ex.getMessage();
        ROS_ERROR("Error Code: %d, %s", code, message.c_str());
      }
    }

    return true;
  }


  bool init(double loop_hz, const std::vector<std::string> &joints, const std::vector<std::string> &actuators)
  {
    this->loop_hz = loop_hz;
    this->joints = joints;
    this->actuators = actuators;

    // EtherCAT
    std::string ifname;
    if (!node.getParam("ethercat/ifname", ifname))
    {
      std::string param_name = node.resolveName("ethercat/ifname");
      ROS_ERROR("Failed to get '%s' parameter.", param_name.c_str());
      return false;
    }

    XmlRpc::XmlRpcValue slaves_param;
    if (!node.getParam("ethercat/slaves", slaves_param))
    {
      std::string param_name = node.resolveName("ethercat/slaves");
      ROS_ERROR("Failed to get '%s' parameter.", param_name.c_str());
      return false;
    }

    std::vector<std::string> slaves;
    for (int i = 0; i < slaves_param.size(); i++)
    {
      try
      {
        std::string device_name = slaves_param[i]["device_name"];
        slaves.push_back(device_name);
      }
      catch (const XmlRpc::XmlRpcException &ex)
      {
        auto code = ex.getCode();
        auto message = ex.getMessage();
        ROS_ERROR("Error Code: %d, %s", code, message.c_str());
      }
    }


    if (!init_ethercat(ifname, slaves))
    {
      ROS_FATAL("Failed to initialize Hardware Interface!");
      close();
      return false;
    }

    if (!config_slaves(slaves_param))
    {
      ROS_FATAL("Failed to initialize Hardware Interface!");
      close();
      return false;
    }


    // Hardware Interface
    const int n_actuators = actuators.size();

    a_pos.resize(n_actuators, 0.0); a_pos_cmd.resize(n_actuators, 0.0);
    a_vel.resize(n_actuators, 0.0); a_vel_cmd.resize(n_actuators, 0.0);
    a_eff.resize(n_actuators, 0.0); a_eff_cmd.resize(n_actuators, 0.0);

    for (int i = 0; i < n_actuators; i++)
    {
      hardware_interface::ActuatorStateHandle act_state_handle(actuators[i], &a_pos[i], &a_vel[i], &a_eff[i]);
      act_state_interface.registerHandle(act_state_handle);

      hardware_interface::ActuatorHandle pos_act_handle(act_state_handle, &a_pos_cmd[i]);
      pos_act_interface.registerHandle(pos_act_handle);

      hardware_interface::ActuatorHandle vel_act_handle(act_state_handle, &a_vel_cmd[i]);
      vel_act_interface.registerHandle(vel_act_handle);
    }

    registerInterface(&act_state_interface);
    registerInterface(&pos_act_interface);
    // registerInterface(&vel_act_interface);

    ROS_INFO("Hardware Interface initialized correctly.");
    return true;
  }


  /* */
  bool start();
  bool start(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res);

  /* */
  bool enable_motion();
  bool enable_motion(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res);

  /* */
  bool start_homing();
  bool start_homing(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res);

  /**/
  bool start_motion();
  bool start_motion(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res);

  /* */
  bool halt();
  bool halt(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res);

  /* */
  bool quick_stop();
  bool quick_stop(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res);

  /* */
  bool set_zero_position();
  bool set_zero_position(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res);


  void read(const ros::Time &time, const ros::Duration &period)
  {
    const int n_actuators = actuators.size();

    for (int i = 0; i < n_actuators; i++)
    {
      const uint16 slave_idx = 1 + i;

      a_pos[i] = ec_master.tx_pdo[slave_idx].position_actual_value / POSITION_STEP_FACTOR;
      // a_vel[i] = ec_master.tx_pdo[slave_idx].velocity_actual_value / VELOCITY_STEP_FACTOR;

      ROS_DEBUG_THROTTLE(1.0, "Slave[%d], status_word: 0x%.4x", slave_idx, ec_master.tx_pdo[slave_idx].status_word);
      ROS_DEBUG_THROTTLE(1.0, "Slave[%d], mode_of_operation display: %d", slave_idx, ec_master.tx_pdo[slave_idx].mode_of_operation_display);
      ROS_DEBUG_THROTTLE(1.0, "Slave[%d], position_actual_value: %d", slave_idx, ec_master.tx_pdo[slave_idx].position_actual_value);
      ROS_DEBUG_THROTTLE(1.0, "Slave[%d], digital_inputs: 0x%.8x", slave_idx, ec_master.tx_pdo[slave_idx].digital_inputs);
    }
  }


  void write(const ros::Time &time, const ros::Duration &period)
  {
    const int n_actuators = actuators.size();

    for (int i = 0; i < n_actuators; i++)
    {
      const uint16 slave_idx = 1 + i;

      ec_master.rx_pdo[slave_idx].target_position = a_pos_cmd[i] * POSITION_STEP_FACTOR;
      ec_master.rx_pdo[slave_idx].touch_probe_function = 0;
      ec_master.rx_pdo[slave_idx].physical_outputs = 0x0000;

      ROS_DEBUG_THROTTLE(1.0, "Slave[%d], control_word: 0x%.4x", slave_idx, ec_master.rx_pdo[slave_idx].control_word);
    }

    ec_master.update();
  }


  void close()
  {
    ec_master.close();
    ROS_INFO("EtherCAT socket closed.");
  }

};

} }  // namespace
#endif
