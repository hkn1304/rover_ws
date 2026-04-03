#!/usr/bin/env bash

set -euo pipefail

JETSON_HOST="${JETSON_HOST:-192.168.1.150}"
JETSON_USER="${JETSON_USER:-hkn}"
JETSON_PASS="${JETSON_PASS:-}"
JETSON_SSH_OPTS="${JETSON_SSH_OPTS:--o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=5}"

JETSON_CONTAINER="${JETSON_CONTAINER:-wave_rover_robot}"
JETSON_WS="${JETSON_WS:-/root/ros2_ws}"
JETSON_ROS_SETUP="${JETSON_ROS_SETUP:-/opt/ros/humble/install/setup.bash}"
JETSON_WS_SETUP="${JETSON_WS_SETUP:-$JETSON_WS/install/setup.bash}"
JETSON_PARAMS_FILE="${JETSON_PARAMS_FILE:-$JETSON_WS/src/wave_rover_control/config/params.yaml}"

require_sshpass() {
  if ! command -v sshpass >/dev/null 2>&1; then
    cat >&2 <<'EOF'
sshpass is required for password-based SSH from WSL.
Install it with:
  sudo apt-get update && sudo apt-get install -y sshpass
EOF
    exit 1
  fi
}

ssh_cmd() {
  if [[ -n "$JETSON_PASS" ]]; then
    require_sshpass
    sshpass -p "$JETSON_PASS" ssh $JETSON_SSH_OPTS "${JETSON_USER}@${JETSON_HOST}" "$@"
  else
    ssh $JETSON_SSH_OPTS "${JETSON_USER}@${JETSON_HOST}" "$@"
  fi
}

ssh_tty_cmd() {
  if [[ -n "$JETSON_PASS" ]]; then
    require_sshpass
    sshpass -p "$JETSON_PASS" ssh -tt $JETSON_SSH_OPTS "${JETSON_USER}@${JETSON_HOST}" "$@"
  else
    ssh -tt $JETSON_SSH_OPTS "${JETSON_USER}@${JETSON_HOST}" "$@"
  fi
}

ssh_tty_cmd_manual_auth() {
  ssh -tt $JETSON_SSH_OPTS "${JETSON_USER}@${JETSON_HOST}" "$@"
}

container_env() {
  cat <<EOF
source "$JETSON_ROS_SETUP" >/dev/null 2>&1
if [ -f "$JETSON_WS_SETUP" ]; then source "$JETSON_WS_SETUP" >/dev/null 2>&1; fi
export AMENT_PREFIX_PATH="$JETSON_WS/install/wave_rover_control:\${AMENT_PREFIX_PATH:-}"
export CMAKE_PREFIX_PATH="$JETSON_WS/install/wave_rover_control:\${CMAKE_PREFIX_PATH:-}"
export ROS_PACKAGE_PATH="$JETSON_WS/src:\${ROS_PACKAGE_PATH:-}"
EOF
}

container_exec() {
  local cmd="$1"
  local full_cmd
  full_cmd="$(container_env)
$cmd"
  ssh_cmd docker exec "${JETSON_CONTAINER}" bash --noprofile --norc -lc "$(printf '%q' "$full_cmd")"
}

host_exec() {
  local cmd="$1"
  ssh_cmd bash --noprofile --norc -lc "$(printf '%q' "$cmd")"
}

usage() {
  cat <<'EOF'
Usage:
  ./tools/jetson_remote_tests.sh <command> [args]

Commands:
  host-check
    Show Nano host info and Docker containers.

  container-check
    Verify the Humble container, ROS setup, and exported executables.

  topics
    Show current ROS 2 topics inside the test container.

  bridge
    Run json_to_serial_node inside the test container.

  bridge-up
    Start json_to_serial_node in the background inside the test container.

  bridge-log
    Show the background bridge log.

  bridge-down
    Stop the background bridge process if it is running.

  teleop
    Run teleop_twist_keyboard inside the test container.
    Keyboard input stays in your local WSL terminal.

  vel <linear_x> [angular_z] [count]
    Publish a fixed Twist to /cmd_vel inside the test container.

  stop
    Publish a zero Twist to /cmd_vel once.

  echo-cmd
    Echo /cmd_vel inside the test container.

  smoother-smoke
    Run cmd_vel_smoother briefly and publish one test /cmd_vel_raw message.

Environment overrides:
  JETSON_HOST        default: 192.168.1.150
  JETSON_USER        default: hkn
  JETSON_PASS        optional password for sshpass
  JETSON_CONTAINER   default: chatgpt_ros2_humble_test
  JETSON_WS          default: /root/ros2_ws
  JETSON_ROS_SETUP   default: /opt/ros/humble/install/setup.bash
  JETSON_WS_SETUP    default: $JETSON_WS/install/setup.bash
  JETSON_PARAMS_FILE default: $JETSON_WS/src/wave_rover_control/config/params.yaml

Recommended manual control flow:
  1. export JETSON_PASS=1
  2. ./tools/jetson_remote_tests.sh container-check
  3. Start the bridge:
     ./tools/jetson_remote_tests.sh bridge-up
  4. In terminal B: ./tools/jetson_remote_tests.sh teleop
  5. Use the keyboard to drive, then run:
     ./tools/jetson_remote_tests.sh stop
EOF
}

cmd_host_check() {
  host_exec "echo HOST:\$(hostname) && uname -a && echo --- && docker ps --format 'table {{.Names}}\t{{.Status}}\t{{.Image}}'"
}

cmd_container_check() {
  container_exec "echo CONTAINER:\$(hostname) && echo --- && which ros2 && echo --- && ros2 pkg executables wave_rover_control"
}

cmd_topics() {
  container_exec "ros2 topic list"
}

cmd_bridge() {
  container_exec "ros2 run wave_rover_control json_to_serial_node --ros-args --params-file \"$JETSON_PARAMS_FILE\""
}

cmd_bridge_up() {
  ssh_cmd docker exec "${JETSON_CONTAINER}" bash --noprofile --norc -lc "pkill -f 'wave_rover_control json_to_serial_node' >/dev/null 2>&1 || true" || true
  ssh_cmd docker exec -d "${JETSON_CONTAINER}" bash --noprofile --norc -lc "source \"$JETSON_ROS_SETUP\" >/dev/null 2>&1 && if [ -f \"$JETSON_WS_SETUP\" ]; then source \"$JETSON_WS_SETUP\" >/dev/null 2>&1; fi && export AMENT_PREFIX_PATH=\"$JETSON_WS/install/wave_rover_control:\${AMENT_PREFIX_PATH:-}\" && export CMAKE_PREFIX_PATH=\"$JETSON_WS/install/wave_rover_control:\${CMAKE_PREFIX_PATH:-}\" && export ROS_PACKAGE_PATH=\"$JETSON_WS/src:\${ROS_PACKAGE_PATH:-}\" && ros2 run wave_rover_control json_to_serial_node --ros-args --params-file \"$JETSON_PARAMS_FILE\" >/tmp/wave_rover_bridge.log 2>&1"
  sleep 2
  ssh_cmd docker exec "${JETSON_CONTAINER}" bash --noprofile --norc -lc "ps aux | grep json_to_serial_node | grep -v grep || true"
}

cmd_bridge_log() {
  ssh_cmd docker exec "${JETSON_CONTAINER}" bash --noprofile --norc -lc "cat /tmp/wave_rover_bridge.log || true"
}

cmd_bridge_down() {
  ssh_cmd docker exec "${JETSON_CONTAINER}" bash --noprofile --norc -lc "pkill -f 'wave_rover_control json_to_serial_node' >/dev/null 2>&1 || true"
}

cmd_teleop() {
  if [[ -n "$JETSON_PASS" ]]; then
    echo "teleop uses direct ssh authentication to preserve a real TTY; enter the Nano password when prompted." >&2
  fi
  ssh_tty_cmd_manual_auth docker exec -it "${JETSON_CONTAINER}" bash --noprofile --norc -lc "source \"$JETSON_ROS_SETUP\" >/dev/null 2>&1 && if [ -f \"$JETSON_WS_SETUP\" ]; then source \"$JETSON_WS_SETUP\" >/dev/null 2>&1; fi && export AMENT_PREFIX_PATH=\"$JETSON_WS/install/wave_rover_control:\${AMENT_PREFIX_PATH:-}\" && export CMAKE_PREFIX_PATH=\"$JETSON_WS/install/wave_rover_control:\${CMAKE_PREFIX_PATH:-}\" && export ROS_PACKAGE_PATH=\"$JETSON_WS/src:\${ROS_PACKAGE_PATH:-}\" && ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args -r cmd_vel:=/cmd_vel"
}

cmd_vel() {
  local linear="${1:-}"
  local angular="${2:-0.0}"
  local count="${3:-3}"

  if [[ -z "$linear" ]]; then
    echo "linear_x is required" >&2
    usage
    exit 1
  fi

  container_exec "ros2 topic pub /cmd_vel geometry_msgs/msg/Twist '{linear: {x: ${linear}, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: ${angular}}}' -r 5 --times ${count}"
}

cmd_stop() {
  container_exec "ros2 topic pub /cmd_vel geometry_msgs/msg/Twist '{linear: {x: 0.0, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}' --once"
}

cmd_echo_cmd() {
  container_exec "ros2 topic echo /cmd_vel"
}

cmd_smoother_smoke() {
  container_exec "ros2 run wave_rover_control cmd_vel_smoother >/tmp/cmd_vel_smoother.log 2>&1 & NODE_PID=\$!; sleep 3; ros2 topic pub /cmd_vel_raw geometry_msgs/msg/Twist '{linear: {x: 0.10, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.05}}' --once >/tmp/cmd_vel_pub.log 2>&1; timeout 8 ros2 topic echo /cmd_vel --once; kill \$NODE_PID || true; wait \$NODE_PID 2>/dev/null || true"
}

main() {
  local cmd="${1:-help}"
  shift || true

  case "$cmd" in
    host-check) cmd_host_check ;;
    container-check) cmd_container_check ;;
    topics) cmd_topics ;;
    bridge) cmd_bridge ;;
    bridge-up) cmd_bridge_up ;;
    bridge-log) cmd_bridge_log ;;
    bridge-down) cmd_bridge_down ;;
    teleop) cmd_teleop ;;
    vel) cmd_vel "$@" ;;
    stop) cmd_stop ;;
    echo-cmd) cmd_echo_cmd ;;
    smoother-smoke) cmd_smoother_smoke ;;
    help|-h|--help) usage ;;
    *)
      echo "Unknown command: $cmd" >&2
      usage
      exit 1
      ;;
  esac
}

main "$@"
