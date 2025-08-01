# 14 Optimizations 02: Task 1

| Optimization                                  | Render time in seconds |
| --------------------------------------------- | ---------------------: |
| Build in Debug                                |                1612.23 |
| Build in Release                              |                74.3848 |
| Thread per row (implemented prior to lecture) |                 9.5058 |
| Regions rendering                             |                9.33183 |
| Buckets rendering                             |                9.24362 |
| Bounding Box                                  |                2.58989 |
| **KD tree**                                   |           **0.066962** |

> [!NOTE]
> Benchmarks were ran on a Lenovo Legion Slim 5 16AHP9 laptop, with the
> following software and hardware characteristics:

|         | Characteristics of Lenovo Legion Slim 5 16AHP9                               |
| ------- | ---------------------------------------------------------------------------- |
| **CPU** | AMD Ryzen™ 7 8845HS                                                          |
| **GPU** | NVIDIA GeForce RTX™ 4070 Laptop GPU                                          |
| **RAM** | 32 GB LPDDR5-6500                                                            |
| **SSD** | 1 TB M.2 2280 SSD                                                            |
| **OS**  | Fedora Linux 41 (Workstation Edition) 64-bit<br>Linux 6.15.4-100.fc41.x86_64 |
| **FS**  | Btrfs                                                                        |

[![14 Optimizations 02: Task 1: Benchmarking Scene](../results/png/14-01-acceleration-tree-scene1.png)](../results/ppm/14-01-acceleration-tree-scene1.ppm)
[![14 Optimizations 02: Task 1: Debugging Scene](../results/png/14-01-acceleration-tree-scene0.png)](../results/ppm/14-01-acceleration-tree-scene0.ppm)
