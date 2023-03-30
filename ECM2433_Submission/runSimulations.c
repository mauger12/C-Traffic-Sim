#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

struct node {
    int time;
    struct node *nextNode;
};
typedef struct node NODE;

struct queue {
    int count;
    NODE *front;
    NODE *rear;
};
typedef struct queue QUEUE;

struct stats {
    int leftVehicleNum;
    int leftAvgWait;
    int leftMaxWait;
    int leftClearTime;

    int rightVehicleNum;
    int rightAvgWait;
    int rightMaxWait;
    int rightClearTime;
};
typedef struct stats STATS;



STATS runOneSimulation(float leftRate, float rightRate, int leftTime, int rightTime);
void enqueue(QUEUE *q, int value);
int dequeue(QUEUE *q);
void initialise(QUEUE *q);
void initialiseStats(STATS *s);
int isEmpty(QUEUE *q);
void display(NODE *head);



int main(int argc, char **argv){
    STATS *cumulStat;
    if(!(cumulStat = malloc(sizeof(STATS)))){
        printf("out of memory");
        exit(1);
    }
    initialiseStats(cumulStat);
    STATS singleStat;
    initialiseStats(cumulStat);
    float leftRate = atof(argv[1]);
    float rightRate = atof(argv[2]);
    int leftTime = atoi(argv[3]);
    int rightTime = atoi(argv[4]);
    printf("Parameter values:\n");
    printf("   from left:\n");
    printf("      traffic arrival rate: %.2f\n",leftRate);
    printf("      traffic light period: %d\n",leftTime);
    printf("   from right:\n");
    printf("      traffic arrival rate: %.2f\n",rightRate);
    printf("      traffic light period: %d\n",rightTime);

    int j = 0;
    for(j=0;j<100;j++){
        singleStat = runOneSimulation(leftRate, rightRate, leftTime, rightTime);
        cumulStat->leftVehicleNum += singleStat.leftVehicleNum;
        cumulStat->leftAvgWait += singleStat.leftAvgWait;
        cumulStat->leftMaxWait += singleStat.leftMaxWait;
        cumulStat->leftClearTime += singleStat.leftClearTime;
         
        cumulStat->rightVehicleNum += singleStat.rightVehicleNum;
        cumulStat->rightAvgWait += singleStat.rightAvgWait;
        cumulStat->rightMaxWait += singleStat.rightMaxWait;
        cumulStat->rightClearTime += singleStat.rightClearTime;
    }

    printf("Results (averaged over 100 runs):\n");
    printf("   from left:\n");
    printf("      number of vehicles:   %d\n",cumulStat->leftVehicleNum/100);
    printf("      average waiting time: %d\n",cumulStat->leftAvgWait/100);
    printf("      maximum waiting time: %d\n",cumulStat->leftMaxWait/100);
    printf("      clearance time:       %d\n",cumulStat->leftClearTime/100);
     
    printf("   from right:\n");
    printf("      number of vehicles:   %d\n",cumulStat->rightVehicleNum/100);
    printf("      average waiting time: %d\n",cumulStat->rightAvgWait/100);
    printf("      maximum waiting time: %d\n",cumulStat->rightMaxWait/100);
    printf("      clearance time:       %d\n",cumulStat->rightClearTime/100);
    free(cumulStat);
    return 0;
}




void display(NODE *head){
    /*recursively print all of the items in the queue from front to back*/
    if(head == NULL){
        printf("Back of the queue\n\n");
    }
    else {
        printf("%d\n", head -> time);
        display(head->nextNode);
    }
}

int isEmpty(QUEUE *q){
    /*check if queue is empty or not, returns true/false*/
    return (q->rear ==NULL);
}

void initialise(QUEUE *q){
    /*set initial values of the queue*/
    q->count = 0;
    q->front = NULL;
    q->rear = NULL;
}

void initialiseStats(STATS *s){
    /*set initial values of the stats*/
    s->leftVehicleNum = 0;
    s->leftAvgWait = 0;
    s->leftMaxWait = 0;
    s->leftClearTime = 0;

    s->rightVehicleNum = 0;
    s->rightAvgWait = 0;
    s->rightMaxWait = 0;
    s->rightClearTime = 0;
}

void enqueue(QUEUE *q, int value){
    /*add a node to the back of the queue*/
    NODE *temp;
    if(!(temp = malloc(sizeof(NODE)))){
        printf("out of memory");
        exit(1);
    }
    /*need to trap memory here*/
    temp->time = value;
    temp->nextNode = NULL;
    
    /*if queue not empty set new node as rear node
      else set front and back node as new node
      then increase count of nodes in queue*/
    if(!isEmpty(q)){
        q->rear->nextNode = temp;
        q->rear = temp;
    }
    else {
        q->front = q->rear = temp;
    }
    q->count++;
}

int dequeue(QUEUE *q){
    /*redirect front of queue and remove front item if there are items in list, returning time*/
    int t = -1;
    if(q->front != NULL){
        NODE *temp;
        t = q->front->time;
        temp = q->front;
        q->front = q->front->nextNode;
        q->count--;
        free(temp);
    }
    return(t);
}

STATS runOneSimulation(float leftRate, float rightRate, int leftTime, int rightTime) {
    /*random number generator*/
    const gsl_rng_type *T;
    gsl_rng *r;
    gsl_rng_env_setup();
    T = gsl_rng_default;
    r = gsl_rng_alloc (T);
    gsl_rng_set(r,time(0));

    /*initialise queue stats and left and right queues*/
    STATS *qStats;
    if(!(qStats = malloc(sizeof(STATS)))){
        printf("out of memory");
        exit(1);
    }
    initialiseStats(qStats);
    QUEUE *leftQ, *rightQ;
    if(!(leftQ = malloc(sizeof(QUEUE)))){
        printf("out of memory");
        exit(1);
    }
    initialise(leftQ);
    if(!(rightQ = malloc(sizeof(QUEUE)))){
        printf("out of memory");
        exit(1);
    }
    /*need to trap memory here*/
    initialise(rightQ);

    bool lights = false; /*false = left green, true = right green, if one green other is red*/
    int i;
    int lastLight = 0;
    for(i = 0; i < 501; i++) {
        if(i==lastLight+leftTime && lights==false){
            lights = true;
            lastLight = i;
        }
        else if(i==lastLight+rightTime && lights==true){
            lights = false;
            lastLight = i;
        }
        else{
            if(gsl_rng_uniform (r) < leftRate){
                /*add to left queue*/
                enqueue(leftQ,i);
                qStats->leftVehicleNum ++;
                /*printf("added to left queue\n");*/
            }
            if(gsl_rng_uniform (r) < rightRate){
                /*add to right queue*/
                enqueue(rightQ,i);
                qStats->rightVehicleNum ++;
                /*printf("added to right queue\n");*/
            }
            if(gsl_rng_uniform (r) < 0.5){
                /*if left lights green, with 50/50 chance move 1 car through lights
                  not specified in spec to be configurable so this is how i set it*/
                if(!lights){
                    int time = dequeue(leftQ);
                    if(time > -1 && qStats->leftMaxWait < i-time){
                        qStats->leftMaxWait = i-time;
                    }
                    qStats->leftAvgWait += i-time;
                    /*printf("removed from left queue\n");*/
                }
                /*if right lights green, with 50/50 chance move 1 car throuh lights*/
                if(lights){
                    int time = dequeue(rightQ);
                    if(time > -1 && qStats->rightMaxWait < i-time){
                        qStats->rightMaxWait = i-time;
                    }
                    qStats->rightAvgWait += i-time;
                    /*printf("removed from right queue\n");*/
                }
            }
        }
    }

    int leftClearTimeL = 0;
    int rightClearTimeL = 0;

    while(leftQ->front!=NULL || rightQ->front!=NULL){
        /*whilst cars remain in either queue, clear the queues*/
        
        if(leftQ->front!=NULL){
            leftClearTimeL ++;
        }
        /*get clear time for each queue*/
        if(rightQ->front!=NULL){
            rightClearTimeL ++;
        }
        
        if(i==lastLight+leftTime && lights==false){
            lights = true;
            lastLight = i;
        }
        if(i==lastLight+rightTime && lights==true){
            lights = false;
            lastLight = i;
        }
        else{
            if(gsl_rng_uniform (r) < 0.5){
                /*if left lights green, with 50/50 chance move 1 car through lights
                  not specified in spec to be configurable so this is how i set it*/
                if(!lights){
                    int time = dequeue(leftQ);
                    if(time > -1 && qStats->leftMaxWait < i-time){
                        qStats->leftMaxWait = i-time;
                    }
                    qStats->leftAvgWait+=i-time;
                    /*printf("removed from left queue\n");*/
                }
                /*if right lights green, with 50/50 chance move 1 car throuh lights*/
                if(lights){
                    int time = dequeue(rightQ);
                    if(time > -1 && qStats->rightMaxWait < i-time){
                        qStats->rightMaxWait = i-time;
                    }
                    qStats->rightAvgWait+=i-time;
                    /*printf("removed from right queue\n");*/
                }
            }
        }
        
        i++;
    }
     
    /*update final stats for this simulations*/
    qStats->leftAvgWait /= qStats->leftVehicleNum;
    qStats->rightAvgWait /= qStats->rightVehicleNum;
    qStats->leftClearTime = leftClearTimeL;
    qStats->rightClearTime = rightClearTimeL;

    gsl_rng_free(r);
    free(leftQ);
    free(rightQ);
    return *qStats;
}
