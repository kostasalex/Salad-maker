# Overview
This project is a simulation of a chef and salad makers working together to prepare salads. The chef selects ingredients (tomato, green pepper, small onion) and places them on a shared table for the salad makers to retrieve. The salad makers then use the ingredients to make a salad in a random time between a lower bound (lb) and upper bound (ub) specified by the user. The chef and salad makers also log their actions to separate log files, as well as a global log file.

# Compiling the program
Use the command "make" to compile the program. This will create two executables, "chef" and "salad_maker", using separate compilation.

# Running the program
To run the "chef" executable, use the command:

`./chef -n <number_of_salads> -m <mantime>
where -m is the time in milliseconds that the chef spends selecting ingredients.`

To run the "salad_maker" executable, use the command:

`./salad_maker -t1 <lb> -t2 <lu> -s <shmid> -c <cook_num>`
where -t1 and -t2 are the lower and upper bounds of the time in milliseconds that the salad maker takes to make a salad, -s is the shared memory id, and -c is the salad maker number (0, 1, 2) to create the necessary number of salad maker processes.

-The only input validation that is done is to check the number of arguments passed in.

# Functionality
* The chef creates a shared memory segment (shmget) and initializes most of the variables, as well as the following semaphores:
   * sem_t chef: To notify the salad makers that ingredients are available
   * sem_t cooks[3]: To notify each salad maker that ingredients are available
   * sem_t log: To ensure that only one writer can access the log files at a time
   *  sem_t access_table: To ensure that only one process can access the shared table at a time
* The salad makers attach to the shared memory segment and retrieve ingredients from the shared table, using the semaphore access_table. They then proceed to make a salad in a random time between the lower and upper bounds specified by the user.
*  The chef and salad makers also log their actions to separate log files, as well as a global log file using the semaphore log.
*  When all salads are done, the chef sends a notification to each salad maker to terminate its process.
*  The chef then removes the shared memory segment and closes all log files.
#Additional Information
Additional information about the program and its implementation can be found in the source code comments. The code for this project is written in C.
