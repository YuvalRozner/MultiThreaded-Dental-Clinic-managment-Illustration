#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>

#define N 10 //the amount of patients can enter the clinic in the same time.
sem_t cashierDental,cashierPatient, sofa , space , chairDental ,chairPatient; //semaphores
sem_t mutex; //general mutex
int currentPatients=0; //count the current amount of patient inside the clinic.

//nodes for the linked lists.
typedef struct node
{
    int serialNum;
    struct node* next;
}*PNode;

//maneger struct for a linked list.
typedef struct list
{
    struct node* head;
    struct node* tail;

}*PList;

PList waitToSit; //the queue of the patients waiting to sit on the sofa. 
PList waitTotreat; //the queue of the patients sitting on the sofa and eaiting to get treated. 

void Error(char* msg){fprintf(stderr, "Error:%s\n", msg);  exit(-1);} //error function.

// function to add a value to the linked list.
// the function gets an integer value and a list, it allocates a new node and add it to the end of the list.
void addNode(int serialNum,PList listManeger){
    PNode temp = (PNode)malloc(sizeof(struct node)); //allocates new node.
    if(temp==NULL) //checks the allocation.
        Error("allocation error.");
    temp->serialNum=serialNum;
    temp->next=NULL;
    if(listManeger->head==NULL){ // if the list is still empty.
        listManeger->head=temp;
    }
    else{
        listManeger->tail->next=temp;
    }
    listManeger->tail=temp; // sets the new tail of the list.
}

// function to extract a node from the linked list.
// the function gets a list, it extract the first node in of the list and returns it.
PNode removeNode(PList listManeger){
    if(listManeger->head==NULL){ // if the list is empty.
        return NULL;
    }
    PNode temp=listManeger->head;
    listManeger->head=listManeger->head->next;
    if(listManeger->head==NULL){ // if it was the last node of the list.
        listManeger->tail=NULL;
    }
    return temp;
}

// patient entering the clinic.
void enterClinic(int serial){
	//checks if can enter the clinic:
    sem_wait(&mutex);
    if (currentPatients==N){
        printf("I'm Pacient #%d, I out of the clinic\n",serial);
    }
    sem_post(&mutex);
    sem_wait(&space); //wait untill there is enough place in the clinic.
    sem_wait(&mutex);
    currentPatients++; // increase the counting.
    printf("I'm Pacient #%d, I got into the clinic\n",serial);
    addNode(serial,waitToSit); // add the patient to the queue waiting for the sofa.
    sem_post(&mutex);
    sleep(1);
}

// patient sits on the sofa.
void siting(int serial){
    while(waitToSit->head->serialNum!=serial){sleep(1);} // waits util it is his turn to sit.
    sem_wait(&sofa); // wait for an empty place on the sofa.
    sem_wait(&mutex);
    printf("I'm Pacient #%d, I'm sitting on the sofa\n",serial);
	//move him from the sofa queue to the treatment queue:
    removeNode(waitToSit);
    addNode(serial,waitTotreat);
    sem_post(&mutex);
    sleep(1);
}

// patient getting treatment.
void getTreatment(int serial){
    while(waitTotreat->head->serialNum!=serial){sleep(1);} // waits util it is his turn to get treatment.
    sem_wait(&chairPatient); //wait for a valiable treatment chair.
    sem_wait(&mutex);
    sem_post(&sofa); // makes room on the sofa.
    sem_post(&chairDental); // inform the dental hygienists that he waits to get treated.
    removeNode(waitTotreat);
    printf("I'm Pacient #%d, I'm getting treatment\n",serial);
    sem_post(&mutex);
    sleep(1);
}

// patient gets up forn the chair and goes to the cashier to pay.
void paying(int serial){
    sem_wait(&mutex);
    sem_post(&chairPatient); // make room on his treatment chair.
    printf("I'm Pacient #%d, I'm paying now\n",serial);
    sem_post(&cashierDental);//leave the cashier.
    sem_post(&mutex);
    sleep(1);
}

// patient getting out of the clinic.
void exitClinic(int serial){
    sem_wait(&mutex);
    currentPatients--; // decreace the counting.
    printf("I'm Pacient #%d, I got out from the clinic\n",serial);
    sem_post(&space); //make room in the clinic for another patient.
    sem_post(&mutex);
    sleep(1);
}

// the function which runs for each patient's thread.
// it runs all the actions a patient is able to do in an infiny loop.
// the order of actions is: enter the clinic -> sitting on sofa -> get treated -> pay -> get out of clinic...
void* funcPatient(void* arg){
    int serial=*(int *)arg;
    while(1){
        enterClinic(serial);
        siting(serial);
        getTreatment(serial);
        paying(serial);
        sem_wait(&cashierPatient); //wait for a dantal hygienist to take payment and than gets out of the clinic.
        exitClinic(serial);
        sleep(1);
    }
}

// dental hygienist is treating a patient.
void treat(int serial){
    sem_wait(&chairDental); // wait for a patient to sit on a treatment chair.
    printf("I'm Dental Hygienist #%d, I'm working now \n",serial);
    sleep(1);
}

// dental hygienist is getting payment.
void getPay(int serial){
    sem_wait(&cashierDental); // wait till there is a patient waiting at the cashier.
    sem_wait(&mutex);
    printf("I'm Dental Hygienist #%d, I'm getting a payment\n",serial);
    sem_post(&cashierPatient);// inform the patient, she is at the cashier and he can gets out of the clinic.
    sem_post(&mutex);
    sleep(1);
}

// dental hygienist sleeps until there is a patient waits for a treatment or a payment.
void sleeping(int serial){
    sem_wait(&mutex);
    if(currentPatients<3){//if there is less patients than dental hygienist , dentel Hygienist go to sleep.
        printf("I'm Dental Hygienist #%d, I'm going to sleep\n",serial);
    }
    sem_post(&mutex);
    sleep(1);
}

// the function which runs for each dantal hygienist's thread.
// it runs all the actions a dantal hygienist is able to do in an infiny loop.
// the order of actions is: treat a patient -> get payment -> go to sleep...
void* funcDental(void* arg){
    int serial=*(int *)arg;
    while(1){
        treat(serial);
        getPay(serial);
        sleeping(serial);
    }
}

int main(int argc, char * argv[]){// Initialize variables and semaphores
	pthread_t dental[3],patient[N+2];
	int i=0; //for the loops.
	int dentalSerial[3], patientSerial[N+2]; //for sending the serial of each thread to the functions.
	// setes the semaphores:
	sem_init( &cashierDental, 0, 0 ); // indicates if the cashier is free for one of the dental hygienist (there is only 1 cashier).
    sem_init( &cashierPatient, 0, 0 ); // indicates if there is a dental hygienist behind the cashier so the patient will left the clinic.
    sem_init( &space, 0, N ); // indicates if the clinic is full or not, so patients could know if they are allowed to enter.
    sem_init( &sofa, 0, 4 ); // indicates if there is am empty place on the sofa.
    sem_init( &mutex, 0, 1 ); // semaphore to lock and safe a critical parts of the code.
    sem_init( &chairDental, 0, 0 ); // indicates if there is a patient waitting for a dental hygienist to treat him.
    sem_init( &chairPatient, 0, 3 ); // indicates if there is an empty chair for patient to come get treated.
	//allocates and resets the manager of the queue for sitting:
    waitToSit = (PList)malloc(sizeof(struct list)); 
    if(waitToSit==NULL) //checks the allocation.
            Error("allocation error.\n");
    waitToSit->head=NULL;
    waitToSit->tail=NULL;
	//allocates and resets the manager of the queue for get treated:
    waitTotreat = (PList)malloc(sizeof(struct list));
    if(waitTotreat==NULL) //checks the allocation.
            Error("allocation error.\n");
    waitTotreat->head=NULL;
    waitTotreat->tail=NULL;
	for(i=0; i<3; i++){ //dental hygienist threads creation.
        dentalSerial[i]=i+1;
	    pthread_create(&dental[i], NULL,  funcDental    , (void*)&dentalSerial[i]);
    }
    for(i=0;i<N+2; i++){ //patient threads creation.
	    patientSerial[i]=i+1;
	    pthread_create(&patient[i], NULL, funcPatient , (void*)&patientSerial[i]);
    }
    // the program (threads) run untill force stoped by (ctr+C).
    for(i=0; i<3; i++)
	    pthread_join(dental[i], NULL);
    for(i=0; i<N+2; i++)
	    pthread_join(patient[i], NULL);
	return 0;
}