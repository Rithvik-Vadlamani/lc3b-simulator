# LC-3b processor simulation and virtual memory

Two C simulators for an LC-3b processor: a five-stage pipeline and a separate microcoded processor with virtual memory and exception handling.

The pipeline models instruction and data cache latency, dependency stalls without forwarding, and control-flow changes. The microcoded simulator adds address translation and handles page faults, protection violations, unaligned accesses, and illegal opcodes.

## Build

Requires a C compiler and Make:

```sh
make
```

This builds `build/pipeline` and `build/virtual-memory`. The source files retain their original names so they can be matched to the corresponding control-store files.

## Run

```sh
./build/pipeline ucode6_1 path/to/program.obj
./build/virtual-memory ucode5 path/to/page-table.obj path/to/program.obj
```

Both simulators expose an interactive command prompt. Use `?` to see commands for stepping, running, and inspecting registers or memory.

The repository includes the simulator source and available control-store files. Program images, the virtual-memory page table, and the original reference simulators are not bundled. Compilation alone does not reproduce the reference comparisons described on my resume.

## Where to look

- [lc3bsim6_3.c](lc3bsim6_3.c): pipeline latches, `FETCH_stage`, `DE_stage`, `AGEX_stage`, `MEM_stage`, and `SR_stage`.
- [lc3bsim5.c](lc3bsim5.c): microsequencer, memory cycles, datapath updates, address translation, and exception handling.

My validation work compared timing and final register/memory state against reference simulators, using dependency, load, and branch tests.
