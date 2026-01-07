# ros2-udp-esp8266-control
A ROS2 based WiFi-controlled RC car using UDP for real-time motor control. A ROS2 Python node converts joystick inputs from the /joy topic into PWM commands and sends them via UDP to an ESP8266, which drives DC motors through an L293D motor driver with low latency.
