# Operating-Systems

REPORT: SQUASHBUG HEURISTIC REASONING
			           Team-20

		The "sb" command is a tool for detecting malwares in a system. The command takes a suspected process ID as an argument and displays its parent, grandparent, and so on. Additionally, the command has a flag "-suggest" that, based on a heuristic, detects which process ID is the root of all trouble.

		The heuristic used by the "sb" command to detect the malware is based on two factors: time spent by the process and number of child processes it has spawned. A process is considered suspicious if it meets either of the following conditions:

	1. Time Spent: The process has spent more than 60 seconds of CPU time. This can indicate that the process is doing some resource-intensive work, which could be malicious in nature.

	2. Number of Child Processes: The process has spawned more than 10 child processes. This can indicate that the process is creating multiple processes to hide its activity or to perform multiple tasks in parallel.

	The reasoning behind using these two factors is that a malware often requires a lot of resources, such as CPU time and memory, to perform its malicious activities. Additionally, malwares often try to hide their activities by creating multiple processes that perform different tasks, making it difficult to detect them.


In conclusion, the "sb" command provides a quick and easy way to detect malwares in a system. The heuristic used by the command is based on the assumption that a malware will be resource-intensive and will create multiple processes to hide its activities. While this heuristic may not always correctly identify the malware, it provides a good starting point for further investigation.
