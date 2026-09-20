# Autonomous AI Search & Assistance Robot

## Project Overview

A simulation-first autonomous mobile robotics project developed using ROS 2.

The goal is to design and implement a virtual mobile robot capable of autonomous navigation, perception, object/person search, mission execution, and assistance tasks inside a simulated environment.

The project will progressively integrate robotics software, simulation, perception, navigation, and AI into a single autonomous system.

## Long-Term Goal

The final system should be capable of receiving a high-level mission such as:

> "Navigate to Room B, search for a person or specified object, report the detection and approximate location, and return to the designated base."

## Planned Capabilities

- Autonomous mobile robot simulation
- Simulated LiDAR, camera and IMU
- Odometry and coordinate transformations
- Environment mapping and localization
- SLAM
- Autonomous navigation
- Obstacle avoidance
- Computer vision
- YOLO-based object/person detection
- Object localization
- Mission-level planning
- Search behavior
- Detection reporting
- Return-to-base behavior

## Technology Stack

### Robotics
- C++
- Python
- Linux / Ubuntu
- ROS 2
- TF2
- URDF / Xacro
- ros2_control

### Simulation
- Gazebo
- RViz2

### Navigation
- SLAM
- Nav2
- Behavior Trees

### AI / Perception
- OpenCV
- YOLO

### Development
- Git
- GitHub
- VS Code

## Development Philosophy

This project is being developed incrementally alongside a robotics software learning roadmap.

The system will begin with fundamental software components and progressively evolve into a complete autonomous robotics architecture.

The project will prioritize simulation before physical hardware so that the software architecture can later be transferred to a real robot.

## Current Status

**Phase 0 — Project Planning & Development Environment**

- [x] Hardware assessment
- [x] WSL2 environment verified
- [x] Ubuntu 24.04 LTS installed
- [x] Project repository created
- [x] Git initialized
- [x] VS Code + WSL2 development workflow configured
- [ ] ROS 2 development environment
- [ ] Robot software architecture
- [ ] Robot description
- [ ] Simulation
- [ ] Sensors
- [ ] SLAM
- [ ] Autonomous navigation
- [ ] AI perception
- [ ] Mission system

## Project Roadmap

```text
C++ / Linux
     ↓
Git / GitHub
     ↓
ROS 2
     ↓
Robot Software Architecture
     ↓
URDF / Xacro
     ↓
RViz2
     ↓
Gazebo
     ↓
Sensors
     ↓
TF2 + Odometry
     ↓
SLAM
     ↓
Nav2
     ↓
YOLO + OpenCV
     ↓
Perception + Navigation
     ↓
Mission Planning
     ↓
Autonomous Search
     ↓
Testing & Evaluation