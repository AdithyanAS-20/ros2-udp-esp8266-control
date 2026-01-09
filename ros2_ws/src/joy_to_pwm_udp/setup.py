from setuptools import setup
import os
from glob import glob

package_name = 'joy_to_pwm_udp'

setup(
    name=package_name,
    version='0.0.0',
    packages=[package_name],  # ensure there's an __init__.py in your package directory
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        # If you have launch files, include them as well:
        *(
            [(os.path.join('share', package_name, 'launch'), glob('launch/*.launch.py'))]
        )
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Your Name',
    maintainer_email='your.email@example.com',
    description='ROS2 node sending PWM commands via UDP from /joy topic',
    license='Apache License 2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
        'joy_to_pwm_udp = joy_to_pwm_udp.joy_to_pwm_udp:main',
       ],
       },

)
