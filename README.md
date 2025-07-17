# TunnelMan: 2D Arcade Game

A classic 2D arcade-style game built entirely in C++, focused on object-oriented design, dynamic memory management and real-time game logic.

**Link to Video Demo:** [https://www.youtube.com/watch?v=sm63Nyf_KcI](https://www.youtube.com/watch?v=sm63Nyf_KcI)

## How It's Made:

**Tech used:** C++, freeglut, irrKlang, Object-Oriented Programming

The TunnelMan project demonstrates a deep understanding of core C++ principles without relying on external game engines. The graphics, windowing and keyboard input are all handled from scratch using the **freeglut** library, while sound effects are managed by the **irrKlang** audio engine.

The entire game world is built upon an object-oriented architecture. A central `Actor` base class defines common behaviors, while derived classes like `TunnelMan`, `Protester`, and `Boulder` implement specialized functionality. The main game logic is orchestrated by the `StudentWorld` class, which manages all actors and their interactions within a 2D grid. The game operates on a tick-based loop, where each actor gets a turn to perform its action, showcasing polymorphism.

## Optimizations

While the core game is complete, several features could be added to expand upon the original design:

- **New Enemy Types:** Introduce a "Hardcore Protester" that can move faster or is harder to stun.
- **More Items:** Add new tools for the TunnelMan, such as dynamite to clear larger areas or a sonar to temporarily reveal all items on the map.
- **High Score System:** Implement a system to save and display high scores locally.
- **Varying Difficulty:** Create different level layouts or increase the number of protesters that spawn as the player progresses.

## Lessons Learned:

- **Polymorphism in Action:** The most critical lesson was managing a collection of `Actor` pointers and calling a virtual `doSomething()` method on each one. Seeing different objects (Protesters, Boulders, etc.) exhibit their unique behaviors from a single loop was a powerful demonstration of object-oriented design.
- **Game State Management:** Juggling all the different states—the player's health and inventory, the location of all actors, the number of oil barrels left, and the game's tick count—provided a deep appreciation for the complexity of game development.
- **Dynamic 2D Arrays and Pointers:** Implementing the oil field as a 2D grid of `Earth` objects and managing the interactions between actors within that grid was a major challenge that solidified my understanding of pointers and memory layout.
- **AI and Pathfinding:** Developing the logic for the Protesters—from their initial state to tracking the TunnelMan and finally leaving the oil field—was a fantastic introduction to basic AI and state machine concepts.
