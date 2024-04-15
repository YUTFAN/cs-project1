//导入需要的头文件
#include <stddef.h> 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define MAX_LENTH_OF_PROCESS_NAME 9

//定义进程状态
typedef enum {
    READY = 0,
    RUNNING = 1,
    FINISHED = 2,
    WAITING = 3
} State;

//定义进程控制块结构体
typedef struct PCB{
    char proc_name[MAX_LENTH_OF_PROCESS_NAME];          //进程名
    int arrival_time;                                   //到达时间
    int service_time;                                   //服务时间
    int memory_requirement;                             //内存需求  
    State state;                                        //状态
    struct PCB *next;                                   //指向下一个进程的指针

}PCB;

//创建进程控制块
PCB *createPCB(char *proc_name, int arrival_time, int service_time, int memory_requirement, State state) {
    PCB *pcb = malloc(sizeof(PCB));
    strcpy(pcb->proc_name, proc_name);
    pcb->arrival_time = arrival_time;
    pcb->service_time = service_time;
    pcb->memory_requirement = memory_requirement;
    pcb->state = 3;
    pcb->next = NULL;
    return pcb;
}

//定义进程队列结构体
typedef struct {
    PCB *head;                                          //队列头指针
    PCB *tail;                                          //队列尾指针
    int size;                                           //队列大小
} Queue;

//创建进程队列
Queue *createQueue() {
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    return queue;
}

//判断队列是否为空
int isEmpty(Queue *queue) {
    return queue->size == 0;
}

//入队
void enqueue(Queue *queue, PCB *pcb) {
    if (isEmpty(queue)) {
        queue->head = pcb;
        queue->tail = pcb;
    } else {
        queue->tail->next = pcb;
        queue->tail = pcb;
    }
    queue->size++;
}

//出队
PCB *dequeue(Queue *queue) {
    if (isEmpty(queue)) {
        return NULL;
    }
    PCB *pcb = queue->head;
    queue->head = queue->head->next;
    queue->size--;
    return pcb;
}

//计算队列中的进程数量
int get_code_num (Queue *queue) {
    return queue->size;
}





//实现round robin调度算法
void round_robin(Queue *proc_queue, Queue *ready_queue, int quantum) {
    int time = 0;
    PCB *running_proc = NULL;
    while (!isEmpty(proc_queue) || !isEmpty(ready_queue) || running_proc != NULL) {
        add_to_ready_queue(proc_queue, ready_queue, time);
        PCB * running_proc = dequeue(ready_queue);
        if(running_proc == NULL){
            time++;
            continue;
        }
        running_proc -> state = RUNNING;
        printf("%d,RUNNING,process-name=%s,remaining-time=%d\n", time, running_proc->proc_name, running_proc->service_time);
        if(running_proc && running_proc->service_time <= quantum){
            time += quantum;
            running_proc->service_time = 0;
            running_proc->state = FINISHED;
            add_to_ready_queue(proc_queue, ready_queue, time);
            printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", time, running_proc->proc_name,get_code_num(ready_queue) );
            running_proc = NULL;
        }else{
            time += quantum;
            running_proc->service_time -= quantum;
            add_to_ready_queue(proc_queue, ready_queue, time);
            if(!isEmpty(ready_queue)){
                running_proc -> state = READY;
                enqueue(ready_queue, running_proc);
            }
            while(isEmpty(ready_queue)&&running_proc -> service_time > quantum){
                time += quantum;
                running_proc -> service_time -= quantum;
                add_to_ready_queue(proc_queue, ready_queue, time);
            }
            if(running_proc -> service_time <= quantum && isEmpty(ready_queue)){
                time += quantum;
                running_proc -> service_time = 0;
                running_proc -> state = FINISHED;
                add_to_ready_queue(proc_queue, ready_queue, time);
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", time, running_proc->proc_name,get_code_num(ready_queue) );
                running_proc = NULL;
            }
        }
    }
}


//将到达时间小于等于当前时间的进程加入就绪队列
void add_to_ready_queue(Queue *proc_queue, Queue *ready_queue, int time) {
    PCB *current = proc_queue->head, *previous = NULL;
    while (current != NULL && current->arrival_time <= time) {
        PCB *next = current->next;
        if (previous == NULL) {
            proc_queue->head = next;
        } else {
            previous->next = next;
        }
        if(proc_queue->head == NULL) {
            proc_queue->tail = NULL;
        }

        current->next = NULL;
        current->state = READY;
        enqueue(ready_queue, current);
        previous = current;  // 更新 previous
        current = next;
    }
}

//从文件中读取进程信息
Queue *get_processes(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("File not found\n");
        return NULL;
    }
    Queue *proc_queue = createQueue();
    char proc_name[MAX_LENTH_OF_PROCESS_NAME];
    int arrival_time;
    int service_time;
    int memory_requirement;
    while (fscanf(file, "%s %d %d %d", proc_name, &arrival_time, &service_time, &memory_requirement) == 4) {
        PCB *pcb = createPCB(proc_name, arrival_time, service_time, memory_requirement, WAITING);
        enqueue(proc_queue, pcb);
    }
    fclose(file);
    return proc_queue;
}





int main(int argc, char **argv){
    int quantum;
    char *filename;
    char *scheduling_algorithm;
    if(argc != 7){
        printf("invalid input\n");
        return 0;
    }
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-q") == 0){
            quantum = atoi(argv[i+1]);
        }
        if(strcmp(argv[i], "-f") == 0){
            filename = argv[i+1];
        }
        if(strcmp(argv[i], "-s") == 0){
            scheduling_algorithm = argv[i+1];
        }
    }
    Queue *proc_queue = get_processes(filename);
    Queue *ready_queue = createQueue();
    round_robin(proc_queue, ready_queue, quantum);
    return 0;
}






