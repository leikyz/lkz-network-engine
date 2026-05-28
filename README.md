# LKZ Network Engine

An lightweight, high-performance, multithreaded C++ network engine built from scratch. Designed to synchronize thousands of real-time game entities simultaneously with minimal overhead and a strictly authoritative server architecture.

[![GitHub](https://img.shields.io/badge/GitHub-Repository-blue?logo=github)](https://github.com/leikyz/lkz-network-engine)
[![Status](https://img.shields.io/badge/Status-Work--In--Progress-orange)](#)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-00599C?logo=c%2B%2B)](https://github.com/leikyz/lkz-network-engine)

---

## 🚀 Overview

The **LKZ Network Engine** forms the core backbone of the LKZ Multiplayer Ecosystem. Triggered by a desire to master low-level networking, the engine shifted from an early C# prototype into a fully custom C++ architecture utilizing modern asynchronous design patterns and server-side spatial awareness.

### 🗺️ The Ecosystem Journey
1. **[LKZ Network Engine (Core)](https://leikyz.github.io/projects/lkz-network)**: The low-level asynchronous C++/IOCP transport and simulation layers.
2. **[LKZ Online Services](https://leikyz.github.io/projects/lkz-online-services)**: A Go-based backend handling user accounts, session registration, and custom matchmaking.
3. **[LKZ Demo](https://leikyz.github.io/projects/lkz-demo)**: A Unity technical showcase acting as the visualizer and real-time telemetry/metrics monitor.

---

## ✨ Features

- [x] **Event-Driven Networking Layer** for responsive decoupled sub-systems.
- [x] **High-Performance Multi-threading** with thread-safe queue systems handling network I/O.
- [x] **Zero-Allocation I/O Pipeline** ensuring no dynamic memory allocations on the hot path during packet processing.
- [x] **Fully Authoritative Server Simulation** executing player inputs and AI steering entirely on the host.
- [x] **Recast Navmesh Integration** steering multiple independent AI agents natively on the server.
- [x] **Optimized Batching Mechanisms** to group updates and prevent packet fragmentation bottlenecks.
- [x] **Live Metrics Tracker Integration** reporting telemetry directly back to the visualizer for debugging.

---

## 🔮 Roadmap & Future Goals

- [ ] **Data Security:** Encrypted packet payloads for secure over-the-wire data transmissions.
- [ ] **Advanced Transport Logic:** Custom network-layer packet fragmentation and reassembly to securely support large data payloads over UDP.
- [ ] **Cryptographically Secure Authentication:** Token-based connection handshakes validated dynamically against the *LKZ Online Services* backend.
