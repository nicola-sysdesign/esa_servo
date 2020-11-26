#include "esa_servo/ewdl/hardware_interface/ewdl_hardware_interface.h"


bool esa::ewdl::ServoHW::fault_reset()
{
  if (ec_master.fault_reset())
  {
    reset_controllers = true;
    return true;
  }
  else
  {
    return false;
  }
}


bool esa::ewdl::ServoHW::fault_reset(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res)
{
  if (fault_reset())
  {
    res.success = true;
    res.message = "Fault resetted";
  }
  else
  {
    res.success = false;
    res.message = "Failed to reset Fault";
  }

  return true;
}


bool esa::ewdl::ServoHW::ready_to_switch_on()
{
  if (ec_master.ready_to_switch_on())
  {
    reset_controllers = true;
    return true;
  }
  else
  {
    return false;
  }
}


bool esa::ewdl::ServoHW::ready_to_switch_on(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res)
{
  if (ready_to_switch_on())
  {
    res.success = true;
    res.message = "Ready to Switch On";
  }
  else
  {
    res.success = false;
    res.message = "Failed to enter state Ready to Switch On";
  }

  return true;
}


bool esa::ewdl::ServoHW::switch_on()
{
  if (ec_master.switch_on())
  {
    reset_controllers = true;
    return true;
  }
  else
  {
    return false;
  }
}


bool esa::ewdl::ServoHW::switch_on(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res)
{
  if (switch_on())
  {
    res.success = true;
    res.message = "Switched On";
  }
  else
  {
    res.success = false;
    res.message = "Failed to Switch On";
  }

  return true;
}


bool esa::ewdl::ServoHW::switch_off()
{
  if (ec_master.switch_off())
  {
    reset_controllers = true;
    return true;
  }
  else
  {
    return false;
  }
}


bool esa::ewdl::ServoHW::switch_off(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res)
{
  if (switch_off())
  {
    res.success = true;
    res.message = "Switched Off";
  }
  else
  {
    res.success = false;
    res.message = "Failed to Switch Off";
  }

  return true;
}


bool esa::ewdl::ServoHW::enable_operation()
{
  if (ec_master.enable_operation())
  {
    reset_controllers = true;
    return true;
  }
  else
  {
    return false;
  }
}


bool esa::ewdl::ServoHW::enable_operation(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res)
{
  if (enable_operation())
  {
    res.success = true;
    res.message = "Operation enabled";
  }
  else
  {
    res.success = false;
    res.message = "Failed to enable Operation";
  }

  return true;
}


bool esa::ewdl::ServoHW::disable_operation()
{
  if (ec_master.disable_operation())
  {
    reset_controllers = true;
    return true;
  }
  else
  {
    return false;
  }
}


bool esa::ewdl::ServoHW::disable_operation(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res)
{
  if (disable_operation())
  {
    res.success = true;
    res.message = "Operation disabled";
  }
  else
  {
    res.success = false;
    res.message = "Failed to disable Operation";
  }

  return true;
}


bool esa::ewdl::ServoHW::start_homing()
{
  if (ec_master.set_mode_of_operation(esa::ewdl::ethercat::mode_of_operation_t::HOMING))
  {
    ROS_INFO("Switched Mode of Operation to: HOMING");
  }
  else
  {
    ROS_ERROR("Failed to switch Mode of Operation to: HOMING");
    return false;
  }

  if (ec_master.start_homing())
  {
    ROS_INFO("Homing started ...");
  }
  else
  {
    ROS_ERROR("Failed to start Homing procedure.");
    return false;
  }

  return true;
}


bool esa::ewdl::ServoHW::start_homing(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res)
{
  if (start_homing())
  {
    res.success = true;
    res.message = "Homing started ...";
  }
  else
  {
    res.success = false;
    res.message = "Failed to start Homing procedure.";
  }

  return true;
}


bool esa::ewdl::ServoHW::start_motion()
{
  reset_controllers = true;

  if (ec_master.set_mode_of_operation(esa::ewdl::ethercat::mode_of_operation_t::CYCLIC_SYNCHRONOUS_POSITION))
  {
    ROS_INFO("Switched Mode of Operation to: CYCLIC_SYNCHRONOUS_POSITION");
  }
  else
  {
    ROS_ERROR("Failed to switch Mode of Operation to: CYCLIC_SYNCHRONOUS_POSITION");
    return false;
  }

  return true;
}


bool esa::ewdl::ServoHW::start_motion(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res)
{
  if (start_motion())
  {
    res.success = true;
    res.message = "Motion started.";
  }
  else
  {
    res.success = false;
    res.message = "Failed to start motion!";
  }

  return true;
}


bool esa::ewdl::ServoHW::halt()
{
  if (ec_master.halt())
  {
    return true;
  }
  else
  {
    return false;
  }
}


bool esa::ewdl::ServoHW::halt(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res)
{
  if (halt())
  {
    res.success = true;
    res.message = "Halt!";
  }
  else
  {
    res.success = false;
    res.message = "Halt failed!";
  }

  return true;
}


bool esa::ewdl::ServoHW::quick_stop()
{
  if (ec_master.quick_stop())
  {
    return true;
  }
  else
  {
    return false;
  }
}


bool esa::ewdl::ServoHW::quick_stop(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res)
{
  if (quick_stop())
  {
    res.success = true;
    res.message = "Quick Stop!";
  }
  else
  {
    res.success = false;
    res.message = "Quick Stop failed!!!";
  }

  return true;
}


bool esa::ewdl::ServoHW::set_zero_position()
{
  if (ec_master.set_zero_position() > 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}


bool esa::ewdl::ServoHW::set_zero_position(std_srvs::TriggerRequest &req, std_srvs::TriggerResponse &res)
{
  if (set_zero_position())
  {
    res.success = true;
    res.message = "Setted Zero Position.";
  }
  else
  {
    res.success = false;
    res.message = "Failed to set Zero Position.";
  }

  return true;
}


bool esa::ewdl::ServoHW::get_error_code(const uint16 slave_idx, uint16 &error_code)
{
  if (ec_master.get_error_code(slave_idx, error_code) > 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}


bool esa::ewdl::ServoHW::get_alarm_code(const uint16 slave_idx, uint32 &alarm_code)
{
  if (ec_master.get_alarm_code(slave_idx, alarm_code) > 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}
