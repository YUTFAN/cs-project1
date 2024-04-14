#include <stddef.h> 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>


#define READY 0
#define RUNNING 1
#define FINISHED 2
#define Max_Length 8

typedef struct node node_t;

// Define the process 
struct node {
    char process_name[Max_Length+1];
    int arrival_time;
    int remaining_time;
    int memory_size;
    int memory_start;
    int state;
    node_t *next;
    int frames[512];
    int page_num;//记录frame中存在多少个东西
};

// Define the list
typedef struct {
    node_t *head;
    node_t *tail;
} list_t;




// Create a new node
node_t *node_create(char *process_name, int arrival_time, int remaining_time, int memory_size){
    node_t *new_node = malloc(sizeof(node_t));
    // Initialize the node
    new_node -> arrival_time = arrival_time;
    new_node -> remaining_time = remaining_time;
    new_node -> memory_size = memory_size;
    new_node -> memory_start = -1;
    strcpy(new_node -> process_name, process_name); 
    new_node -> state = READY;
    new_node -> next = NULL;
    new_node -> page_num = 0; 
    return new_node;
} 

list_t *list_insert(list_t *list, node_t *new_node) {
    if(list -> head == NULL){
        list -> head = new_node;
        list -> tail = new_node;
    }else {
        list -> tail -> next = new_node;
        list -> tail = new_node;
    }
    return list;
}
//use this function to count the number of processes in the list
//注意注意注意，额外添加的函数
int count_nodes(list_t *list) {
    int count = 0;
    node_t *current = list->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}
// Get the head of the list
node_t *get_head(list_t *list){
    if(list == NULL || list -> head == NULL){
        return NULL;
    }
    node_t *head = list -> head;
    list -> head = head -> next;
    head -> next = NULL;
    return head;
}
// Check if the list is empty
int is_empty(list_t *list){
    if(list == NULL || list -> head == NULL){
        return 1;
    }
    return 0;
}
void read_input(list_t *input_queue, char *filename){
    FILE *fp = fopen(filename, "r");
    if(fp == NULL){
        printf("Failed to open file.\n");
        return;
    }
    char process_name[Max_Length+1];
    int arrival_time, remaining_time, memory_size;
    while(fscanf(fp, "%d %s %d %d\n", &arrival_time, process_name, &remaining_time, &memory_size) ==4){
        node_t *new_node = node_create(process_name, arrival_time, remaining_time, memory_size);
        input_queue = list_insert(input_queue, new_node);
    }
    fclose(fp);
}
void delete_node(list_t *list, node_t *node){
    if(list -> head == node){
        list -> head = list -> head -> next;
        return;
    }
    node_t *prev, *curr = list -> head;
    while (curr != NULL){
        prev = curr;
        curr = curr -> next;
        if(curr == node){
            break;
        }
    }
    prev -> next = curr -> next;
    if(curr == list -> tail){
        list -> tail = prev;
    }
}
list_t *list_create() {
    // Allocate memory for the list
    list_t *list = malloc(sizeof(list_t)); 
    assert(list != NULL);
// Initialize the list
    list->head = NULL;
    list->tail = NULL;
    return list;
}
void get_arr_list(list_t *input_queue, list_t *ready_queue, int time){
    node_t *temp = input_queue -> head;
    while(temp != NULL){
        node_t *next = temp -> next;
        if(temp -> arrival_time <= time){
            delete_node(input_queue, temp);
            temp -> next = NULL;
            ready_queue = list_insert(ready_queue, temp);
        }
        temp = next;
    }
}
//first fit算法
int first_fit(node_t *node, int *memory){
    int i, j;
    for(i=0; i<2048; i++){
        int exsit = 1;
        for(j=i; j < i + node -> memory_size; j++){
            if(memory[j] != 0){
                exsit = 0;
                break;
            }
        }
        if(exsit == 1){
            node -> memory_start = i;
            for(j=i; j < i + node -> memory_size; j++){
                memory[j] = 1;
            }
            return 1;
        }
    }
    return 0;
}
//释放内存 for task2
void release_memory(node_t *node, int *memory){
    int i;
    for(i=node -> memory_start; i < node -> memory_start + node -> memory_size; i++){
        memory[i] = 0;
    }
}
//计算内存使用率 task2
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

void evict_pages(int pages[], list_t *ready_queue, int time, int num_frame){
    while(count_pages(pages) < num_frame){
        int i;
        node_t *head = get_head(ready_queue);
        if(head -> frames == NULL){
            continue;
        }
        printf("%d,EVICTED,evicted-frames=", time);
        printArr(head->frames, head->page_num);

        for(i=0; i<head -> page_num; i++){
            int index = head -> frames[i];
            pages[index] = 0;
        }
        head->page_num = 0;  // Reset page count after eviction
        list_insert(ready_queue, head);
    }
}

void deallocate_pages(int pages[], node_t *node,int time){
    printf("%d,EVICTED,evicted-frames=", time);
    printArr(node->frames, node->page_num);
    int i;
    for(i=0; i<node -> page_num; i++){
        int num = node -> frames[i];
        pages[num] = 0;
    }
    node -> page_num = 0;
}

int page_fit(node_t *node, int pages[], list_t *ready_queue, int time){
    int num_frame = (int) ceil(node->memory_size / 4.0);
    int page_avail = count_pages(pages);
    if(page_avail < num_frame){
        evict_pages(pages, ready_queue, time, num_frame);
    }
    int i, count = 0;
    for(i=0; i<512; i++){
        if(pages[i] == 0){
            node -> frames[count] = i;
            count++;
            pages[i] = 1;
        }
        if(count == num_frame){
            break;
        }
    }
    //记录这个进程有多少个page
    node -> page_num = count;
    return 1;
}
 
int main(int argc, char **argv) {
    int pages [512] = {0};
    int memory[2048] = {0};
    char *filename;
    char *strategy;
    int quantum, time = 0, i;
    if(argc != 7){
        printf("Invalid number of arguments\n");
        return 0;
    }
    for (i=0; i<7; i++){
        if(strcmp(argv[i], "-f") == 0){ 
            filename = argv[i+1];
        }
        if(strcmp(argv[i], "-m") == 0){
            strategy = argv[i+1];
        }
        if(strcmp(argv[i], "-q") == 0){
            quantum = atoi(argv[i+1]);
        }
    }
    // Create the input queue, ready queue and finished queue
    list_t *input_queue = list_create();
    read_input(input_queue, filename);
    list_t *ready_queue = list_create();
    list_t *finished_queue = list_create();

     while(! is_empty(input_queue) || ! is_empty(ready_queue)){
        get_arr_list(input_queue, ready_queue, time);
        //找到一个process，要么已被分配内存，要么可以被分配内存
        node_t *head = NULL;
        while(1){
            head = get_head(ready_queue);
            //如果ready_queue为空，那么跳出循环
            if(head == NULL){
                break;
            }
            //如果当前进程已经被分配内存且方法为first fit，那么跳出循环
            if(head -> memory_start != -1 && strcmp(strategy,"first-fit") == 0){
                break;
            }
            //如果执行的策略是paged，但它已经被分配了内存，那么跳出循环
            if((strcmp(strategy,"paged") == 0 )&& head -> page_num != 0){
                break;
            }
            int success = -1;
            //如果执行的策略是infinite，那么跳出循环
            if(strcmp(strategy,"infinite") == 0){ 
                break;
            }else if (strcmp(strategy,"first-fit") == 0){
                success = first_fit(head, memory);
            }else if (strcmp(strategy,"paged") == 0){
                success = page_fit(head, pages, ready_queue, time); 

            }
            if(success==1){
                break;
            }
            list_insert(ready_queue, head);
        }
        

        if(head == NULL){
            time += quantum;
            continue;
        }

        
        //如果执行的策略是infinite，那么输出task1 的格式
        if(strcmp(strategy,"infinite") == 0){
            printf("%d,RUNNING,process-name=%s,remaining-time=%d\n", 
                   time, head->process_name, head->remaining_time);
        }
        //如果执行的策略是first-fit，那么输出task2 的格式
        if(strcmp(strategy,"first-fit") == 0){
            int mem_usage = memory_usage(memory);
            printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,allocated-at=%d\n", 
                   time, head->process_name, head->remaining_time, mem_usage, head->memory_start);
        }
        //如果执行的策略是paged，那么输出task3 的格式
        if(strcmp(strategy,"paged") == 0){
            int page_usage_rate = page_usage(pages);
            printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=", 
                time, head->process_name, head->remaining_time, page_usage_rate);
            printArr(head ->frames, head->page_num);
        }

        //当前进程开始运行
        head -> state = RUNNING;
        //如果当前进程的剩余时间小于等于quantum，那么当前进程将完成
        if(head -> remaining_time <= quantum){
            time += quantum ;
            head->remaining_time = 0; // 然后将剩余时间设置为0
            head -> state = FINISHED;
            list_insert(finished_queue, head);
            get_arr_list(input_queue, ready_queue, time); // 更新 ready_queue
            int proc_remaining = count_nodes(ready_queue);// 计算剩余进程数
            //如果执行的策略是first-fit，那么释放内存
            if(strcmp(strategy,"first-fit") == 0){
                release_memory(head, memory);
            }
            if(strcmp(strategy,"paged") == 0){
                deallocate_pages(pages, head,time);
            }
            printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", time, head->process_name, proc_remaining);
        } else{
            time += quantum;
            head -> remaining_time -= quantum;// 更新当前进程的剩余时间
            get_arr_list(input_queue, ready_queue, time);// 更新 ready_queue

            // 如果 ready_queue 中有进程，那么将当前进程插入到 ready_queue 的尾部
            if (!is_empty(ready_queue)) {
                head -> state = READY;
                list_insert(ready_queue, head); 
            }else {
                //如果 ready_queue 中没有进程，且当前进程的剩余时间大于quantum，再次运行当前进程quantum时间
                //直到当前进程的剩余时间小于等于quantum 或者 ready_queue 中有进程,退·
                while (is_empty(ready_queue) && head->remaining_time > quantum) {
                    time += quantum;    
                    head->remaining_time -= quantum;
                    //更新ready_queue
                    get_arr_list(input_queue, ready_queue, time);
                }
                if (head->remaining_time <= quantum && is_empty(ready_queue)) {
                    time += quantum;
                    head->remaining_time = 0;
                    head->state = FINISHED;
                    list_insert(finished_queue, head);
                    get_arr_list(input_queue, ready_queue, time);
                    int new_proc_remaining = count_nodes(ready_queue);
                    if(strcmp(strategy,"first-fit") == 0){
                        release_memory(head, memory);
                    }
                    if(strcmp(strategy,"paged") == 0){
                        deallocate_pages(pages, head,time);
                    }
                    printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", time, head->process_name, new_proc_remaining);
                }else if (!is_empty(ready_queue)) {
                    head->state = READY;
                    list_insert(ready_queue, head);
                }
            }
        }
    }
    return 0;
}
 