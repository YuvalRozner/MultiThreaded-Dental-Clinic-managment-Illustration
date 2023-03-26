# Dental Clinic Management System

### This is a multithreaded Unix program that simulates a dental clinic management system.
The program uses separate threads for each dental hygienist and patient. The patients enter the clinic, sit on the sofa, get treated by the hygienist, pay at the cashier, and leave the clinic.

The program is written in C and uses POSIX threads and semaphores for synchronization.

## How to use
1. Clone the repository to your local machine:
```
git clone https://github.com/YuvalRozner/Unix-Dental-Clinic-management-system
```
2. Compile the program using a C compiler:
```
gcc -pthread main.c -o Dental_Clinic
```
3. Run the program:
```
./Dental_Clinic
```

## Preconditions
The program uses the following C libraries:

- `stdio.h`
- `pthread.h`
- `semaphore.h`
- `stdlib.h`

## Implementation
The program simulates a dental clinic with a limited capacity of patients. The maximum number of patients that can enter the clinic simultaneously is defined as a constant `N`.

The program uses the following semaphores for synchronization:

* `cashierDental`: A binary semaphore that represents the dental hygienist's availability.
* `cashierPatient`: A binary semaphore that represents the patient's availability at the cashier.
* `sofa`: A binary semaphore that represents the availability of a seat on the sofa.
* `space`: A counting semaphore that represents the number of available spaces in the clinic.
* `chairDental`: A counting semaphore that represents the number of available dental hygienists.
* `chairPatient`: A counting semaphore that represents the number of available treatment chairs.
* `mutex`: A general mutex used to protect critical sections of the program.

The program uses two linked lists to manage the waiting queues for the sofa and the treatment chairs.

The main thread creates separate threads for each patient and dental hygienist. The patient thread runs an infinite loop that simulates the patient's actions in the clinic. The dental hygienist thread runs an infinite loop that simulates the hygienist's actions in the clinic.

## Contributors
This program was written by Your Yuval Rozner and Lior Zucker.