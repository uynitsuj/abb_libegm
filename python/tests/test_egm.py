# python/tests/test_egm.py

import pytest
import abb_libegm_py as egm

def test_base_configuration():
    """Test creation and manipulation of BaseConfiguration"""
    config = egm.BaseConfiguration()
    
    # Test default values
    assert config.axes == egm.RobotAxes.Six
    assert not config.use_demo_outputs
    assert not config.use_velocity_outputs
    assert not config.use_logging
    assert config.max_logging_duration == pytest.approx(60.0)
    
    # Test setting values
    config.axes = egm.RobotAxes.Seven
    config.use_demo_outputs = True
    config.use_velocity_outputs = True
    assert config.axes == egm.RobotAxes.Seven
    assert config.use_demo_outputs
    assert config.use_velocity_outputs

def test_controller_interface():
    """Test creation and basic functionality of EGMControllerInterface"""
    io_service = egm.IOService()
    config = egm.BaseConfiguration()
    
    # Test controller creation
    controller = egm.EGMControllerInterface(io_service, egm.Constants.DEFAULT_PORT_NUMBER, config)
    assert not controller.isConnected()
    assert controller.isInitialized()
    
    # Test configuration get/set
    current_config = controller.getConfiguration()
    assert current_config.axes == config.axes
    assert current_config.use_demo_outputs == config.use_demo_outputs
    
    new_config = egm.BaseConfiguration()
    new_config.use_demo_outputs = True
    controller.setConfiguration(new_config)
    
    updated_config = controller.getConfiguration()
    assert updated_config.use_demo_outputs == True

def test_constants():
    """Test access to constants"""
    assert isinstance(egm.Constants.DEFAULT_PORT_NUMBER, int)
    assert isinstance(egm.Constants.LOWEST_SAMPLE_TIME, float)
    assert egm.Constants.DEFAULT_NUMBER_OF_ROBOT_JOINTS == 6