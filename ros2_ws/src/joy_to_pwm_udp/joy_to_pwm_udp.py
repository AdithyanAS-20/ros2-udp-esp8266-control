import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy
import socket

class JoyToPWM(Node):
    def __init__(self):
        super().__init__('joy_to_pwm_udp')

        self.declare_parameter("target_ip", "10.153.253.51")  # ESP IP
        self.declare_parameter("target_port", 4210)

        self.target_ip = self.get_parameter("target_ip").get_parameter_value().string_value
        self.target_port = self.get_parameter("target_port").get_parameter_value().integer_value

        self.get_logger().info(f"🌐 Target: {self.target_ip}:{self.target_port}")

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        self.subscription = self.create_subscription(
            Joy,
            '/joy',
            self.joy_callback,
            10
        )
        self.get_logger().info("🎮 Subscribed to /joy")

    def joy_callback(self, msg):
        try:
            linear = msg.axes[1]   # forward/backward
            angular = msg.axes[3]  # turning
            self.get_logger().info(f"LINEAR: {linear} ANGULAR: {angular}")
            deadzone = 0.1

            if abs(linear) < deadzone: linear = 0.0
            if abs(angular) < deadzone: angular = 0.0
            left_pwm_raw  = (linear + angular)
            right_pwm_raw = (linear - angular)

            # Clamp to [-1, 1]
            left_pwm_raw = max(-1.0 ,min(left_pwm_raw, 1.0))
            right_pwm_raw = max(-1.0 , min(right_pwm_raw, 1.0))

            # Convert to 0–255 PWM
            left_pwm  = int((left_pwm_raw ) * 255)
            right_pwm = int((right_pwm_raw ) * 255)

            data = f"{left_pwm},{right_pwm}"
            self.sock.sendto(data.encode(), (self.target_ip, self.target_port))

            self.get_logger().info(f"📤 Sent PWM -> L: {left_pwm}, R: {right_pwm}")
        except Exception as e:
            self.get_logger().error(f"❌ joy_callback error: {e}")

def main(args=None):
    rclpy.init(args=args)
    node = JoyToPWM()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("🛑 KeyboardInterrupt, shutting down.")
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
