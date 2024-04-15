#include <stddef.h> 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

typedef struct Process process;
#define READY 0
#define RUNNING 1
#define FINISHED 2

struct Process {
    char name[10];
    int arrival_time;
    int remaining_time;
    int mem_size;
    int mem_start;
    int state;
    process *next;
    int frames[512];
    int page_num;
};

typedef struct {
    process *head;
    process *tail;
} Queue;

process *create_process(char *name, int arrival_time, int remaining_time, int mem_size) {
    process *new_process = malloc(sizeof(process));
    new_process->arrival_time = arrival_time;
    new_process->remaining_time = remaining_time;
    new_process->mem_size = mem_size;
    new_process->mem_start = -1;
    new_process->state = READY;
    new_process->next = NULL;
    strcpy(new_process->name, name);
    new_process->page_num = 0;
    return new_process;

}

Queue *enqueue(Queue *queue, process *new_process) {
    if (queue->head == NULL) {
        queue->head = new_process;
        queue->tail = new_process;
    } else {
        queue->tail->next = new_process;
        queue->tail = new_process;
    }
    return queue;
}

int count_processes(Queue *queue) {
    int count = 0;
    process *current = queue->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

process *dequeue(Queue *queue) {
    if(queue->head == NULL) {
        return NULL;
    }
    process *removed = queue->head;
    queue->head = queue->head->next;
    if(queue->head == NULL) {
        queue->tail = NULL;
    }
    removed->next = NULL;
    return removed;
}

int is_empty(Queue *queue) {
    return queue->head == NULL;
}

void read_file(Queue *proc_queue, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }
    char name[10];
    int arrival_time;
    int remaining_time;
    int mem_size;
    while (fscanf(file, "%d %s %d %d\n",&arrival_time, name,  &remaining_time, &mem_size) == 4) {
        process *new_process = create_process(name, arrival_time, remaining_time, mem_size);
        proc_queue = enqueue(proc_queue, new_process);
    }
    fclose(file);
}

void remove_process(Queue *q, process *p) {
    if (q == NULL || p == NULL || q->head == NULL) {
        return; 
    }
    process *previous = NULL;
    process *current = q->head;
    while (current != NULL && current != p) {
        previous = current;
        current = current->next;
    }
    if (current == NULL) {
        return; 
    }
    if (previous == NULL) {
        q->head = current->next; 
        if (q->tail == p) {
            q->tail = NULL; 
        }
    } else {
        previous->next = current->next; 
        if (current == q->tail) {
            q->tail = previous; 
        }
    }
    current->next = NULL; 
}

Queue *create_queue() {
    Queue *queue = malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

void obtain_input(Queue *proc_queue, Queue *ready_queue, int time) {
    process *current = proc_queue->head;
    while (current != NULL && current->arrival_time <= time) {
        process *next = current->next;
        remove_process(proc_queue, current);
        current -> next = NULL;
        ready_queue = enqueue(ready_queue, current);
        current = next;
    }
    
}

int first_fit(process *p, int *memory) {
    int i = 0; // 初始化变量i
    int free_count = 0; // 连续空闲内存计数器
    while (i < 2048) {
        if (memory[i] == 0) { // 检查当前内存块是否空闲
            free_count++; // 增加连续空闲内存块计数
            if (free_count == p->mem_size) { // 检查是否有足够的连续空闲内存
                int start_index = i - free_count + 1; // 计算内存起始位置
                p->mem_start = start_index; // 设置进程的内存起始位置
                for (int j = start_index; j < start_index + p->mem_size; j++) {
                    memory[j] = 1; // 标记内存为已分配
                }
                return 1; // 成功分配内存，返回1
            }
        } else {
            free_count = 0; // 重置连续空闲内存块计数器
        }
        i++; // 移至下一个内存块
    }
    return 0; // 未找到足够的空闲内存，返回0
}

void release_memory(process *p, int *memory) {
    for (int i = p->mem_start; i < p->mem_start + p->mem_size; i++) {
        memory[i] = 0; // 释放内存
    }
}

int memory_usage(int *memory) {
    int one_count = 0;
    int i;
    for (i = 0; i < 2048; i++) {
        if (memory[i] == 1) {
            one_count++;
        }
    }
    // Add 1024 to the total before dividing to round the result
    return (one_count * 100 + 2047) / 2048;
}

int page_usage(int pages[]) {
    int one_count = 0;
    for (int i = 0; i < 512; i++) {
        if (pages[i] == 1) {
            one_count++;
        }
    }
    return (one_count * 100 + 511) / 512;  // 加上 511 实现向上取整
}
// Function to count the number of free pages
int count_pages(int pages[]){
    int count = 0;
    int i;
    for(i=0; i<512; i++){
        if(pages[i] == 0){
            count++;
        }
    }
    return count;
}

void printArr(int arr[], int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(",");
        }
    }
    printf("]\n");
}

int page_fit(process *p, int pages[], Queue *ready_queue,int time) {
    int page_needed = (p->mem_size + 3) / 4;
    int free_pages = count_pages(pages);
    if (free_pages < page_needed) {
        evict_page(pages, ready_queue, time,page_needed);
    }
    int free_count = 0;
    int i=0;
    while(i < 512){
        if(pages[i] == 0){
            p -> frames[free_count] = i;
            free_count++;
            pages[i] = 1;
        }
        if(free_count == page_needed){
            break;
        }
    }
    p -> page_num = page_needed;
    return 1;
}

void evict_page(int pages[], Queue *ready_queue, int time, int page_needed){
    while(count_pages(pages) < page_needed){
        int i = 0;
        process *oldest = dequeue(ready_queue);
        if(oldest -> frames == NULL){
            continue;
        }
        printf("%d,EVICTED,evicted-frames=", time);
        printArr(oldest -> frames, oldest -> page_num);
        for(i = 0; i < oldest -> page_num; i++){
            pages[oldest -> frames[i]] = 0;
        }
        oldest -> page_num = 0;
        oldest -> state = READY;
        ready_queue = enqueue(ready_queue, oldest);

    }
}

int parse_arguments(int argc, char **argv, char **filename, char **algorithm, int *quantum) {
    if (argc != 7) {
        printf("Invalid number of arguments\n");
        return 0; // 返回0表示参数不足
    }
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) { 
            *filename = argv[i + 1];
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            *algorithm = argv[i + 1];
        } else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) {
            *quantum = atoi(argv[i + 1]);
        }
    }
    return 1; // 返回1表示成功解析所有参数
}


void deallocate(int pages[], process *p, int time){
    printf("%d,EVICTED,evicted-frames=", time);
    printArr(p -> frames, p -> page_num);
    int i = 0;
    while(i < p -> page_num){
        pages[p -> frames[i]] = 0;
        i++;
    }
    p -> page_num = 0;
}

void print_process_status(const char *algorithm, process *current, int *memory, int *pages, int time) {
    if (strcmp(algorithm, "infinite") == 0) {
        printf("%d,RUNNING,process-name=%s,remaining-time=%d\n", 
               time, current->name, current->remaining_time);
    } else if (strcmp(algorithm, "first-fit") == 0) {
        int mem_usage = memory_usage(memory);
        printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,allocated-at=%d\n", 
               time, current->name, current->remaining_time, mem_usage, current->mem_start);
    } else if (strcmp(algorithm, "paged") == 0) {
        int page_usage_rate = page_usage(pages);
        printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=", 
               time, current->name, current->remaining_time, page_usage_rate);
        printArr(current->frames, current->page_num);
    }
}




int main(int argc, char **argv){
    int memory[2048] = {0};
    int pages[512] = {0};
    Queue *proc_queue = create_queue();
    Queue *ready_queue = create_queue();
    char *filename;
    char *algorithm;
    int quantum;
    int time = 0;
    if (!parse_arguments(argc, argv, &filename, &algorithm, &quantum)) {
        return 0;
    }
    read_file(proc_queue, filename);
    while(!is_empty(proc_queue) || ! is_empty(ready_queue)){
        obtain_input(proc_queue, ready_queue, time);
        process *current = NULL;
        for(;;){
            current = dequeue(ready_queue);
            while (
                current ==NULL||
                current -> mem_start != -1 && strcmp(algorithm, "first") == 0 ||
                strcmp(algorithm, "page") == 0 && current -> page_num != 0||
                strcmp(algorithm,"infinite") == 0
            ){
                break;
            }
            int has_memory = -1;
            if (strcmp(algorithm,"first-fit")==0){
                has_memory = first_fit(current, memory);
            }else if (strcmp(algorithm,"page")==0){
                has_memory = page_fit(current, pages, ready_queue, time);
            }
            if(has_memory == 1){
                break;
            }
            enqueue(ready_queue, current);
        }
        if(current == NULL){
            time++;
            continue;
        }
        print_process_status(algorithm, current, memory, pages, time);

        current -> state = RUNNING;
        if(current -> remaining_time <= quantum){
            time += quantum ;
            current -> remaining_time = 0;
            current -> state = FINISHED;
            obtain_input(proc_queue, ready_queue, time);
            int count = count_processes(ready_queue);
            if(strcmp(algorithm, "first-fit") == 0){
                release_memory(current, memory);
            }
            if(strcmp(algorithm, "page") == 0){
                deallocate(pages, current, time);
            }
            printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n",time, current -> name, count);
            current = NULL;
        }else {
            time += quantum;
            current -> remaining_time -= quantum;
            obtain_input(proc_queue, ready_queue, time);
            if(!is_empty(ready_queue)){
                current -> state = READY;
                enqueue(ready_queue, current);
            }else{
                while(is_empty(ready_queue) && current -> remaining_time > quantum){
                    time+= quantum;
                    current -> remaining_time -= quantum;
                    obtain_input(proc_queue, ready_queue, time);
                }
                if(current -> remaining_time <= quantum && is_empty(ready_queue)){
                    time += quantum;
                    current -> remaining_time = 0;
                    current -> state = FINISHED;
                    obtain_input(proc_queue, ready_queue, time);
                    int count = count_processes(ready_queue);
                    if(strcmp(algorithm, "first-fit") == 0){
                        release_memory(current, memory);
                    }
                    if(strcmp(algorithm, "page") == 0){
                        deallocate(pages, current, time);
                    }
                    printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n",time, current -> name, count);
                    current = NULL;
                }else{
                    current -> state = READY;
                    enqueue(ready_queue, current);
                }
            }
        } 
    }
    return 0;
}





    