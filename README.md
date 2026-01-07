LKZ Network is a fast, efficient networking solution built on UDP, specifically engineered for high-performance real-time applications. 
It features an advanced asynchronous event-driven system designed to handle complex message flows with minimal overhead and ultra-low latency.

Originally developed as a final-year project, this C++ network engine is currently being integrated into a Unity-based game. 
My next objective is to decouple the core logic into a standalone, engine-agnostic library to make it compatible with any C++ environment, including Unreal Engine.

## Key Features ##

# Core Architecture

Event-Driven Networking : A reactive system ensuring decoupled and efficient message handling.
Asynchronous I/O : Powered by Windows IOCP for high-concurrency and maximum throughput.
Multithreaded Engine : Optimized for modern multi-core processors.

# Data & Optimization

Custom Serialization : High-speed binary (de)serializer for various data types.
Packet Payload Optimization : Aggressive bit-packing and data reduction techniques to minimize bandwidth usage.
External Profiler : Real-time monitoring of network traffic, latency, and performance metrics.

# Gameplay & Netcode (State Sync)

Authoritative Server : Centralized logic to prevent cheating and maintain game state integrity.
Implementation of Prediction, Reconciliation, and Interpolation for seamless player movement.
ECS Integration : Built-in Entity Component System for scalable server-side simulations.
Navigation & Pathfinding : Custom NavMesh implementation powered by the Recast library.
Matchmaking & Lobbies : Built-in client management and session matchmaking systems.

## Demos ## 

Zombie Synchronization (201 Entities)

![Demo](./Demo/dead-protocol-zombie-sync.gif)

Scalability Performance: While the visual demo features 201 fully animated 3D characters, the network layer is benchmarked to handle 1,500+ synchronized entities using simplified primitives (cubes). 
The current limit in the demo is dictated by client-side rendering and animation overhead, not network throughput.

Advanced Player Movement

![Demo](./Demo/dead-protocol-player-sync.gif)

Demonstrating the Prediction, Reconciliation, and Interpolation algorithms in action for smooth character control.


## Roadmap (2026) ##
The project is actively evolving with the following objectives for 2026:

- Cross-Platform Support: Native Linux integration utilizing XDP (eXpress Data Path) for kernel-level performance.
- Security: End-to-end packet encryption.
- Reliability: Implementation of a packet fragmentation and reassembly layer for large data transfers.

##  Built With ##

Language: C++
OS: Windows (IOCP), Linux (Planned XDP soon)
Libraries: Recast (Navigation)
Unity 6 (Client and game engine) 

