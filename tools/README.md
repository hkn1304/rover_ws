# Jetson Remote Tests

`jetson_remote_tests.sh` is now targeted at the fresh Nano test container:

- Nano host: `192.168.1.150`
- Nano user: `hkn`
- robot container: `wave_rover_robot`
- workspace in container: `/root/ros2_ws`
- ROS setup in container: `/opt/ros/humble/install/setup.bash`

Run it from WSL in the repo root:

```bash
sudo apt-get update && sudo apt-get install -y sshpass
chmod +x ./tools/jetson_remote_tests.sh
export JETSON_PASS=1
./tools/jetson_remote_tests.sh container-check
```

Manual drive flow from this PC:

```bash
export JETSON_PASS=1
./tools/jetson_remote_tests.sh container-check
```

In terminal A:

```bash
export JETSON_PASS=1
./tools/jetson_remote_tests.sh bridge-up
```

In terminal B:

```bash
export JETSON_PASS=1
./tools/jetson_remote_tests.sh teleop
```

Safety helpers:

```bash
export JETSON_PASS=1
./tools/jetson_remote_tests.sh stop
./tools/jetson_remote_tests.sh bridge-log
./tools/jetson_remote_tests.sh bridge-down
./tools/jetson_remote_tests.sh vel 0.10 0.0 3
./tools/jetson_remote_tests.sh echo-cmd
./tools/jetson_remote_tests.sh smoother-smoke
```

Notes:

- `teleop` runs `teleop_twist_keyboard` inside the Nano container, but your keyboard remains in the local WSL terminal.
- `bridge` runs `json_to_serial_node` inside the Nano container with the package config file from the fresh workspace.
- `bridge-up` is the easiest way to start the motor bridge and leave it running while you drive from another terminal.
- A ready-to-use two-container compose file for robot + radar on the Nano is at [deploy/nano/docker-compose.jetson.yml](/c:/Users/Hkn_1/chatgpt/rover_ws/rover_ws/deploy/nano/docker-compose.jetson.yml).
