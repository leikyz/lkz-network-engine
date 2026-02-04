# LKZ Network

LKZ Network is a fast, efficient networking solution built on **UDP**, specifically engineered for **high-performance real-time applications**.
It features an advanced **asynchronous event-driven system** designed to handle complex message flows with minimal overhead and **ultra-low latency**.

This C++ network engine is currently being integrated into a **Unity-based game**.

---

## Key Features

### Core Architecture

* **Event-Driven Networking**
  A reactive system ensuring decoupled and efficient message handling.

* **Asynchronous I/O**
  Powered by **Windows IOCP** for high concurrency and maximum throughput.

* **Multithreaded Engine**
  Optimized for modern multi-core processors.

---

### Data & Optimization

* **Custom Serialization**
  High-speed binary serializer/deserializer supporting various data types.

* **Packet Payload Optimization**
  Aggressive bit-packing and data reduction techniques to minimize bandwidth usage.

* **External Profiler**
  Real-time monitoring of network traffic, latency, and performance metrics.

---

### Gameplay & Netcode (State Synchronization)

* **Authoritative Server**
  Centralized logic to prevent cheating and maintain game state integrity.

* **Advanced Movement Netcode**
  Implementation of **Prediction**, **Reconciliation**, and **Interpolation** for smooth player movement.

* **ECS Integration**
  Built-in **Entity Component System** for scalable server-side simulations.

* **Navigation & Pathfinding**
  Custom **NavMesh** implementation powered by the **Recast** library.

* **Matchmaking & Lobbies**
  Integrated client management and session matchmaking systems.

---

## Demos

### Zombie Synchronization (201 Entities)

![Demo](./Demo/dead-protocol-zombie-sync.gif)

**Scalability Performance**
While the visual demo features **201 fully animated 3D characters**, the network layer is benchmarked to handle **1,500+ synchronized entities** using simplified primitives (cubes).

> The current limitation in the demo is caused by **client-side rendering and animation overhead**, not network throughput.

---

### Advanced Player Movement

![Demo](./Demo/dead-protocol-player-sync.gif)

Demonstrates **Prediction**, **Reconciliation**, and **Interpolation** algorithms in action for responsive and smooth character control.

---

## Roadmap (2026)

* **Cross-Platform Support**
  Native Linux integration using **XDP (eXpress Data Path)** for kernel-level performance.

* **Security**
  End-to-end packet encryption.

* **Reliability**
  Implementation of a packet **fragmentation and reassembly layer** for large data transfers.

---

## Built With

* **Language**: C++
* **Operating Systems**:

  * Windows (IOCP)
  * Linux (Planned – XDP)
* **Libraries**:

  * Recast (Navigation)
* **Game Engine / Client**:

  * Unity 6
