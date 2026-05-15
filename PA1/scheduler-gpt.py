# ChatGPT Scheduler
# Written by Brooks McKinley, Juan Torres Camacho, and Ryan Ramdihal.
# 


import sys
import os

class Process:
    def __init__(self, name, arrival, burst):
        self.name = name
        self.arrival = arrival
        self.burst = burst
        self.remaining_burst = burst
        self.start_time = None
        self.finish_time = None
        self.wait_time = 0
        self.response_time = None
    def __str__(self):
        return self.name

def sjf(processes, runFor, output_file):
    current_time = 0
    completed_processes = 0  # Use an integer counter
    queue = []
    last_selected_process = None

    with open(output_file, 'w') as file:
        file.write(f"{len(processes):3d} processes\n") 
        file.write("Using preemptive Shortest Job First\n")

        while current_time < runFor:
            # Check for arriving processes and add them to the queue
            for process in processes:
                if process.arrival == current_time:
                    queue.append(process)
                    file.write(f"Time {current_time:3d} : {process.name} arrived\n")

            if last_selected_process != None and last_selected_process.finish_time != None:
                file.write(f"Time {current_time:3d} : {last_selected_process.name} finished\n")
                last_selected_process = None
            
            if not queue:
                file.write(f"Time {current_time:3d} : Idle\n")

            # Select the process with the shortest remaining burst time
            if queue:
                queue.sort(key=lambda x: x.remaining_burst)
                selected_process = queue[0]

                if selected_process.start_time is None:
                    selected_process.start_time = current_time
                    selected_process.response_time = current_time - selected_process.arrival

                if selected_process != last_selected_process:
                    file.write(f"Time {current_time:3d} : {selected_process.name} selected (burst {selected_process.remaining_burst:3d})\n") # Human-fixed off by one error and padding
                last_selected_process = selected_process # Human-fixed re-added setting the last_selected_process here

                selected_process.remaining_burst -= 1

                if selected_process.remaining_burst == 0:
                    selected_process.finish_time = current_time + 1
                    completed_processes += 1
                    queue.remove(selected_process)

            current_time += 1

        if last_selected_process != None and last_selected_process.finish_time != None:
            file.write(f"Time {current_time:3d} : {last_selected_process.name} finished\n")
            last_selected_process = None
        file.write(f"Finished at time {current_time:3d}\n\n") # Human-fixed newline location and padding
        printMetrics(processes, file)

def printMetrics(processes, file):
    for process in processes:
        if process.finish_time is not None:
            process.wait_time = process.finish_time - process.arrival - process.burst
            file.write(f"{process.name} wait {process.wait_time:3d} turnaround {process.finish_time - process.arrival:3d} response {process.response_time:3d}\n")
        else:
            file.write(f"{process.name} did not finish\n")

def fifo(processes, runFor, output_file):
    current_time = 0
    completed_processes = 0
    queue = []
    last_selected_process = None

    with open(output_file, "w") as f:
        f.write(f"{len(processes):3d} processes\n")
        f.write("Using First-Come First-Served\n") 

        while current_time < runFor: 
            # Check for arriving processes and add them to the queue
            for process in processes:
                if process.arrival == current_time:
                    queue.append(process)
                    f.write(f"Time {current_time:3d} : {process.name} arrived\n")

            if last_selected_process != None and last_selected_process.finish_time != None:
                f.write(f"Time {current_time:3d} : {last_selected_process.name} finished\n")
                last_selected_process = None

            if not queue:
                f.write(f"Time {current_time:3d} : Idle\n")

            if queue:
                selected_process = queue[0]

                if selected_process.start_time is None:
                    selected_process.start_time = current_time
                    selected_process.response_time = current_time - selected_process.arrival

                if selected_process != last_selected_process:
                    f.write(f"Time {current_time:3d} : {selected_process.name} selected (burst {selected_process.remaining_burst:3d})\n")

                selected_process.remaining_burst -= 1

                if selected_process.remaining_burst == 0:
                    selected_process.finish_time = current_time + 1
                    completed_processes += 1
                    queue.pop(0)

                last_selected_process = selected_process

            current_time += 1

        if last_selected_process != None and last_selected_process.finish_time != None:
            f.write(f"Time {current_time:3d} : {last_selected_process.name} finished\n")
            last_selected_process = None
        f.write(f"Finished at time {current_time:3d}\n\n")
        printMetrics(processes, f) 

def rr(processes, run_for, quantum, output_file):
    current_time = 0 #counter for time iteration 
    completed_processes = 0
    queue = []
    quantum_counter = 0 #counter for between each time iteration and quantum time
    last_selected_process = None

    with open(output_file, 'w') as output:
        # Write the number of processes and the quantum time to the output file
        output.write(f"{len(processes):3d} processes\n") # Human-fixed add padding
        output.write(f"Using Round-Robin\nQuantum {quantum:3d}\n\n")

        while current_time < run_for: 
            #ARRIVED
            for process in processes:
                if process.arrival == current_time:
                    queue.append(process)
                    # Write a message indicating the arrival of a process to the output file
                    output.write(f"Time {current_time:3d} : {process.name} arrived\n")
            
            if last_selected_process != None and last_selected_process.finish_time != None:
                output.write(f"Time {current_time:3d} : {last_selected_process.name} finished\n")
                last_selected_process = None

            #CASE WHEN PROCESS NEEDS TO STOP and quantum counter reset
            if queue and quantum_counter == quantum:
                preempted_process = queue.pop(0)
                # Move the preempted process to the back of the queue
                queue.append(preempted_process)
                quantum_counter = 0
                # Write a message indicating process preemption to the output file
                # output.write(f"Time {current_time + 1:3d} : {preempted_process.name} preempted\n")
                selected_process = None

            #NO PROCESS IN QUEUE
            if not queue:
                # Write an "Idle" message to the output file when the queue is empty
                output.write(f"Time {current_time:3d} : Idle\n")
            #PROCESS IN QUEUE
            if queue:
                selected_process = queue[0] #SELECT PROCESS FIRST IN QUEUE

                if selected_process.start_time is None: 
                    # Set the start time and response time for the selected process
                    selected_process.start_time = current_time
                    selected_process.response_time = current_time - selected_process.arrival

                # process is actually selected
                if quantum_counter == 0:
                    # Write a message indicating the selection of a process to the output file
                    output.write(f"Time {current_time:3d} : {selected_process.name} selected (burst {selected_process.remaining_burst:3d})\n")

                selected_process.remaining_burst -= 1
                quantum_counter += 1
                last_selected_process = selected_process
                
                #CASE WHEN PROCESS FINISHED
                if selected_process.remaining_burst == 0:
                    # Set the finish time for the selected process
                    selected_process.finish_time = current_time + 1
                    completed_processes += 1
                    # Remove the completed process from the queue
                    queue.pop(0)
                    quantum_counter = 0

            current_time += 1

        if last_selected_process != None and last_selected_process.finish_time != None:
            output.write(f"Time {current_time:3d} : {last_selected_process.name} finished\n")
            last_selected_process = None
        # Write a message indicating the end of the simulation to the output file
        output.write(f"Finished at time {current_time:3d}\n\n") # Human-fixed added extra newline and padding
        

        # Calculate metrics for processes after the simulation
        printMetrics(processes, output) # Human-fixed, replaced metrics calculation function with one used in other schedulers

def generate_output_file(input_file):
    # Generate the output file name based on the input file name
    base_name, ext = os.path.splitext(input_file)
    if ext != '.in':
        base_name += '.out'
    else:
        base_name = base_name + '.out' # Human-fixed the format of the filename here
    return base_name

def main():
    if len(sys.argv) != 2:
        print("Usage: python scheduler-gpt.py <input file>") # Human-fixed set filename
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = generate_output_file(input_file)

    processes = []
    run_for = None # 
    quantum = None # 
    num_processes = None # 
    scheduler_provided = False  # Variable to track if the scheduler parameter was provided
    
    rr_scheduler = lambda processes, run_for, output_file: rr(processes, run_for, quantum, output_file)

    try:
        with open(input_file, 'r') as file:
            for line in file:
                line = line.strip()
                parts = line.split()

                if len(parts) == 0:
                    directive = None
                else:
                    directive = parts[0]

                if directive == "processcount":
                    num_processes = int(parts[1])
                elif directive == "runfor":
                    run_for = int(parts[1])
                elif directive == "use":
                    algorithm = parts[1]
                    scheduler_provided = True #
                    if algorithm == "sjf":
                        scheduler = sjf
                    elif algorithm == "fcfs":
                        scheduler = fifo
                    elif algorithm == "rr":
                        scheduler = rr_scheduler
                elif directive == "quantum":
                    quantum = int(parts[1])  # Set the quantum value
                elif directive == "process":
                    if len(parts) < 7:
                        print(f"Error: Missing parameter in process directive - {line}")
                        sys.exit(1)
                    name = parts[2]
                    arrival = int(parts[4])
                    burst = int(parts[6])
                    processes.append(Process(name, arrival, burst))
    except FileNotFoundError:
        print(f"Error: File '{input_file}' not found.")
        sys.exit(1)

    if not num_processes:
        print("Error: Missing parameter - processcount")
        sys.exit(1)
    if not run_for:
        print("Error: Missing parameter - runfor")
        sys.exit(1)

    if not scheduler_provided:
        print("Error: Missing parameter - use") 
        sys.exit(1)

    if scheduler == rr_scheduler and not quantum: 
        print("Error: Missing parameter - quantum") 
        sys.exit(1)

    if scheduler:
        scheduler(processes, run_for, output_file)
    else:
        print("No valid scheduling algorithm specified.")

if __name__ == "__main__":
    main()
