# python/examples/simple_controller.py

import time
import threading
import abb_libegm as egm

def run_io_service(io_service):
    """Run the IO service in a separate thread"""
    io_service.run()

def main():
    # Create and start IO service
    io_service = egm.IOService()
    io_thread = threading.Thread(target=run_io_service, args=(io_service,))
    io_thread.daemon = True
    io_thread.start()

    # Create configuration
    config = egm.BaseConfiguration()
    config.use_velocity_outputs = True
    config.axes = egm.RobotAxes.Seven

    try:
        # Create controller
        controller = egm.EGMControllerInterface(io_service, 
                                              egm.Constants.DEFAULT_PORT_NUMBER,
                                              config)
        
        print(f"Controller initialized: {controller.isInitialized()}")
        print("Waiting for connection...")
        
        # Main control loop
        while not controller.isConnected():
            time.sleep(1)
        
        print("Connected to robot!")
        
        while controller.isConnected():
            # Wait for new message with 500ms timeout
            if controller.waitForMessage(500):
                # Read inputs
                inputs = egm.wrapper.Input()  # Assuming wrapper.Input is properly bound
                controller.read(inputs)
                
                # Process inputs and create outputs
                outputs = egm.wrapper.Output()  # Assuming wrapper.Output is properly bound
                # ... process and fill outputs ...
                
                # Send outputs
                controller.write(outputs)
            else:
                print("Timeout waiting for message")
    
    except KeyboardInterrupt:
        print("\nShutting down...")
    finally:
        # Clean up
        io_service.stop()
        io_thread.join()

if __name__ == "__main__":
    main()