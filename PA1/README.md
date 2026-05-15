# ChatGPT Process Scheduler (PA1)

**Team Members:** Brooks McKinley, Juan Torres Camacho, Ryan Ramdihal

## Overview

This project implements three classic CPU process scheduling algorithms in Python. The simulator reads a list of processes from an input file, runs the selected scheduling algorithm for a specified duration, and writes a tick-by-tick event log plus per-process metrics (turnaround, wait, and response time) to an output file.

Supported algorithms:

- **FCFS** — First-Come, First-Served (FIFO, non-preemptive)
- **SJF** — Shortest Job First (preemptive)
- **RR** — Round Robin (with configurable time quantum)

## Files

- `scheduler-gpt.py` — main entry point containing the `Process` class, the three scheduler functions (`fifo`, `sjf`, `rr`), and the input parser.
- `pa1-testfiles/` — sample input (`.in`) and reference output (`.out`) files for the `c2`, `c5`, and `c10` test cases across all three algorithms.
- `README.md` — this file.

## Requirements

- Python 3.x (no external dependencies)

## Usage

Run the script with a single input file as its only argument:

```
python scheduler-gpt.py <inputFile>.in
```

The simulator writes results to a file with the same base name and a `.out` extension. For example, `c5-sjf.in` produces `c5-sjf.out` in the same directory.

If the input file is not provided, the script prints:

```
Usage: scheduler-gpt.py <input file>
```

## Input File Format

Input files are plain text and use directives, one per line. Inline comments after `#` are ignored by the parser when they follow valid tokens.

| Directive      | Description                                                            |
|----------------|------------------------------------------------------------------------|
| `processcount` | Number of processes in the list                                         |
| `runfor`       | Total time ticks to simulate                                            |
| `use`          | Algorithm to use: `fcfs`, `sjf`, or `rr`                                |
| `quantum`      | Quantum length in time ticks (required only when `use rr`)              |
| `process`      | Takes named parameters: `name <str> arrival <int> burst <int>`          |
| `end`          | End-of-file marker                                                      |

Example:

```
processcount 3
runfor 20
use sjf
process name A arrival 0 burst 5
process name B arrival 1 burst 4
process name C arrival 4 burst 2
end
```

## Output File Format

The output file contains:

1. The number of processes.
2. The algorithm used (and the quantum on the next line, if Round Robin).
3. A chronological event log: process arrivals, selections, completions, and any idle ticks.
4. The final simulation time.
5. Per-process metrics: wait time, turnaround time, and response time. Processes that did not complete within `runfor` are listed as `<name> did not finish`.

Example (SJF):

```
  3 processes
Using preemptive Shortest Job First
Time   0 : A arrived
Time   0 : A selected (burst   5)
Time   1 : B arrived
Time   4 : C arrived
Time   5 : A finished
...
Finished at time  20

A wait   0 turnaround   5 response   0
B wait   6 turnaround  10 response   6
C wait   1 turnaround   3 response   1
```

## Error Handling

The script exits with an error message in the following cases:

- Missing input file argument → prints the usage message above.
- File not found → `Error: File '<file>' not found.`
- Missing `processcount`, `runfor`, or `use` → `Error: Missing parameter - <parameter>`
- `use rr` without a `quantum` directive → `Error: Missing parameter - quantum`
- A `process` directive missing required fields → `Error: Missing parameter in process directive - <line>`

## Testing

Reference test cases are included in `pa1-testfiles`. To compare your output against the expected output for a given case:

```
python scheduler-gpt.py pa1-testfiles/c5-sjf.in
diff pa1-testfiles/c5-sjf.out c5-sjf.out
```

A clean (empty) `diff` indicates the output matches the reference exactly.

