#include <node_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/stat.h>
#include <linux/sched.h>
#include <signal.h>

#pragma pack(1)


typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;


uint db_path_length, threads_count;
char *db, *db_path;
int db_fd;
struct stat db_file_stat;
ulong bufsize;

struct thread *threads;
uchar threads_status = 0;
uchar reading_status = 0;
uchar writing_status = 0;
bool lossless = true;

timespec wait_creating_threads;
timespec wait_free_thread;
timespec wait_signal;
timespec threads_polling_interval;

struct thread {
    ulong tid;
    ulong ptr;
    ulong path_len;
    char *path;
    ulong vars_count;
    char *vars;
    ulong value_len;
    uchar name_len;
    char *name;
    uchar type;
    uchar method;
    ulong bufsize;
    char *buf;
    ulong res_tid;
    uchar status;
};


asm volatile ("fill:\n"
"   movq %rdx, %rcx\n"
"   shrq $3, %rcx\n"
"   cmpq $0, %rcx\n"
"   je fill_end\n"
"   rep movsq\n"

"fill_end:\n"
"   movq %rdx, %rcx\n"
"   andq $7, %rcx\n"
"   cmpq $0, %rcx\n"
"   je fill_return\n"
"   rep movsb\n"
    
"fill_return:\n"
"   ret\n"


"strcmp:\n"
"   movb (%rsi), %cl\n"
"   cmpb (%rdi), %cl\n"
"   jne strcmp_false\n"
"   cmpb $0, %cl\n"
"   je strcmp_true\n"
"   inc %rdi\n"
"   inc %rsi\n"
"   jmp strcmp\n"

"strcmp_true:\n"
"   movw $1, %ax\n"
"   ret\n"

"strcmp_false:\n"
"   movw $0, %ax\n"
"   ret\n"

"thread_create:\n"
"   movq $9, %rax\n"
"   movq $0, %rdi\n"
"   movq $4096, %rsi\n"
"   movq $7, %rdx\n"
"   movq $290, %r10\n"
"   movq $0, %r8\n"
"   movq $0, %r9\n"

"   syscall\n"

"   leaq 4088(%rax), %rsi\n"
"   movq %r14, (%rsi)\n"

"   movq $56, %rax\n"
"   movq $2147585792, %rdi\n"
"   movq $0, %rdx\n"
"   movq $0, %r10\n"
"   movq $0, %r8\n"

"   syscall\n"

"   ret\n");

extern "C" void fill(char *str, char *substr, ulong length);
extern "C" ushort strcmp(char *str1, char *str2);
extern "C" void thread_main();

void printerror() {

    printf("error!\n");

    return;

}

void thread_main() {

    thread *th;
    uchar already_exists = 0;
    uint tmp;
    timespec time1, time2;

    struct {
        uchar type;
        ulong name_len;
        char *value;
        ulong elements_count;
        char *first_element;
    } element;

    asm volatile ("movq %%r13, %0\n"
    : "=r" (th)
    :: "r13", "memory");

    for(;;) {

        if(threads_status == 1) break;

        nanosleep(&wait_creating_threads, 0);

    }
    
    th->buf = (char*)mmap(0, bufsize, 7, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    for(;;) {

        if(th->status != 2) asm volatile ("wait_request:\n"
        "   movq $34, %%rax\n"
        "   syscall\n"
        
        "   cmpb $2, (%0)\n"
        "   jne wait_request\n"
        
        :: "r" (&th->status)
        : "rax", "memory");
        

        if(th->ptr != 3) {
            
            th->ptr += *(uchar*)(th->ptr+db+9) + 10;

        }
        
        
        if(th->path_len > 0) {
            
            asm volatile ("pushq 24(%3)\n" 

            "movq (%0, %4), %%r15\n"
            "cmpq $0, %%r15\n"
            "je not_found\n"
        
            "movq 16(%3), %%r14\n"
            "addq %%r14, 24(%3)\n"
        
            "movq 8(%0, %4), %%r14\n"
            "addq %4, %%r14\n"
        
            "movb (%1), %%r12b\n"
            "movzbq %%r12b, %%r10\n"
            "movzbq %%r12b, %%r13\n"
        
            "addq %1, %%r13\n"
            "addq $1, %1\n"
        
            "find_ptr:\n"
        
            "   cmpb %%r12b, 9(%%r14)\n"
            "   jne next_element\n"
        
            "   leaq 10(%%r14), %%r12\n"
        
            "cmp_names:\n"
        
            "   movb (%1), %%r11b\n"
            "   cmpb %%r11b, (%%r12)\n"
            "   jne next_element\n"
        
            "   cmpq %1, %%r13\n"
            "   je found\n"

            "   incq %1\n"
            "   incq %%r12\n"
        
            "   jmp cmp_names\n"

            "next_element:\n"

            "   decq %%r15\n"
            "   cmpq $0, %%r15\n"
            "   je not_found\n"
        
            "   movq 1(%%r14), %%r14\n"
            "   addq %4, %%r14\n"

            "   movq %%r13, %1\n"
            "   subq %%r10, %1\n"
            "   movb (%1), %%r12b\n"
            "   incq %1\n"

            "   jmp find_ptr\n"

            "next_path:\n"

            "   decq %1\n"

            "   movq (%%r14), %%r15\n"
            "   cmpq $0, %%r15\n"
            "   je not_found\n"
        
            "   movq 8(%%r14), %%r14\n"

            "   movb 1(%1), %%r12b\n"

            "   movzbq %%r12b, %%r13\n"

            "   movq 2(%%r13), %%r12\n"

            "   subq %%r12, 16(%3)\n"
            "   decq 16(%3)\n"

            "   movq %%r13, %%r12\n"

            "   addq %1, %%r13\n"

            "   jmp find_ptr\n"

            "found:\n"
        
            "   incq %1\n"
            "   cmpq %1, 24(%3)\n"
            "   jne next_path\n"
        
            "   movb (%%r14), %%al\n"
            "   movb %%al, (%2)\n"        

            "   movzbq 9(%%r14), %%r13\n"
            "   movb %%r13b, 1(%2)\n"

            "   subq %4, %%r14\n"
            "   movq %%r14, 8(%3)\n"
        
            "   jmp end\n"

            "not_found:\n"

            "   movq $0, 8(%3)\n"

            "end:\n"
            "   popq 24(%3)\n"
            
            :: "r" (th->ptr), "r" (th->path), "r" (&element), "r" (th), "r" (db)
            : "rax", "r15", "r14", "r13", "r12", "r11", "r10", "memory"); 

            if(th->method != 1 && th->ptr != 0) th->ptr += *(uchar*)(th->ptr+db+9) + 10;
        
        } else {
            
            element.type = *(uchar*)(th->ptr+db);

            if(th->method != 1) {

                element.name_len = *(uchar*)(th->ptr+db+9);
            
                if(element.type == 6) element.elements_count = *(ulong*)(th->ptr+db);

            }

        }


        if(th->ptr == 0) {



        } else if(th->method == 1) {

            th->type = element.type;
        
        } else if(th->method == 2) {

            th->type = element.type;

            if(element.type == 1) {

                *(double*)th->buf = *(double*)(th->ptr+db);

            } else if(element.type == 2) {

                *(long*)th->buf = *(long*)(th->ptr+db);

            } else if(element.type == 5) {

                th->value_len = *(ulong*)(th->ptr+db);

                if(th->value_len > 0) asm("movq (%0, %1), %%r15\n"
                "movq 8(%0, %1), %%r14\n"
                "movq 16(%0, %1), %%r13\n"
                "addq %1, %%r13\n"
                
                "jmp copy_blocks\n"

                "copy_last_block:\n"
                
                "   leaq 16(%%r13), %%rsi\n"
                "   movq %%r15, %%rdx\n"

                "   call fill\n"
                
                "   jmp copy_blocks_end\n"

                "copy_blocks:\n"
                
                "   cmpq (%%r13), %%r15\n"
                "   jle copy_last_block\n"
                
                "copy_block:\n" 

                "   leaq 16(%%r13), %%rsi\n"
                "   movq (%%r13), %%rcx\n"

                "   call fill\n"

                "   movq 8(%%r13), %%r13\n"
                "   addq %1, %%r13\n"

                "   subq %%rcx, %%r15\n"

                "   cmpq (%%r14), %%r15\n"
                "   jle copy_last_block\n"

                "   jmp copy_block\n"

                "copy_blocks_end:\n"

                :: "r" (th->ptr), "r" (db), "D" (th->buf)
                : "rsi", "rcx", "r15", "r14", "r13", "r12");

            }

        } else if(th->method == 3) {

            

        } else if(th->method == 4) {

            *(ulong*)th->buf = element.elements_count;

        } else if(th->method == 5) {
            
            if(element.elements_count != 0) asm volatile ("movq 8(%0, %1), %%r15\n"
            "addq %1, %%r15\n"

            "copy_name:\n"

            "   movq %%r15, %%rsi\n"
            
            "   movq 1(%%rsi), %%r15\n"
            "   addq %1, %%r15\n"

            "   movzbq 9(%%rsi), %%rdx\n"
            "   movb %%dl, (%3)\n"
            
            "   incq %3\n"
            "   addq $10, %%rsi\n"
            
            "   call fill\n"
            
            "   decq %2\n"
            "   cmpq $0, %2\n"
            "   jne copy_name\n"
            
            :: "r" (th->ptr), "r" (db), "r" (element.elements_count), "D" (th->buf)
            : "rsi", "rdx", "r15", "memory");

            th->value_len = element.elements_count;

        } else if(th->method == 6) {

            

        } else if(th->method == 7) {

            

        } else if(th->method == 8) {

            th->type = element.type;

            if(element.type == 1) napi_get_value_double(*(napi_env*)th->buf, *(napi_value*)(th->buf+8), (double*)(th->ptr+db)); else
            if(element.type == 2) napi_get_value_bigint_int64(*(napi_env*)th->buf, *(napi_value*)(th->buf+8), (long*)(th->ptr+db), &lossless); else
            if(element.type == 3 || element.type == 4) {

                napi_get_value_uint32(*(napi_env*)th->buf, *(napi_value*)(th->buf+8), &tmp);

                *(uchar*)(th->ptr+db-element.name_len-10) = (uchar)tmp;

            } else if(element.type == 5) {

                asm(""
                
                "jmp rewrite_blocks"

                "rewrite_first_block:\n"

                "   "

                "rewrite_create_blocks:\n"

                "   "

                "rewrite_blocks:\n"

                "   movq 16(%0, %1), %%r15\n"
                "   cmpq (%%r15, %1), %2\n"
                "   jle rewrite_first_block\n"

                "   cmpq (%0, %1), %2\n"
                "   jg rewrite_create_blocks\n"

                "   "

                "rewrite_blocks_end:\n"



                :: "r" (th->ptr), "r" (db), "r" (th->value_len), "S" (th->buf)
                : "rdi", "rdx", "rcx", "r15", "memory");

            }

        } else if(th->method == 9) {

            

        } else if(th->method == 10) {

            

        } else if(th->method == 11) {

            already_exists = 0;

            asm volatile ("movq (%1, %4), %%r15\n"
            "cmpq $0, %%r15\n"
            "je elem_doesnt_exist\n"
            
            "movq 8(%1, %4), %%r14\n"
            "addq %4, %%r14\n"
            
            "find_existed_elem:\n"
            
            "   cmpb %2, 9(%%r14)\n"
            
            "   jne next_existed_element\n"
            
            "   movzbq %2, %%r13\n"
            
            "   leaq 10(%%r14), %%r12\n"
            
            "cmp_existed_names:\n"

            "   decq %%r13\n"
            
            "   movb (%3, %%r13), %%r11b\n"
            "   cmpb %%r11b, (%%r12, %%r13)\n"
            "   jne next_existed_element\n"
            
            "   cmpq $0, %%r13\n"
            "   je elem_exists\n"

            "   jmp cmp_existed_names\n"

            "next_existed_element:\n"
            
            "   decq %%r15\n"
            "   cmpq $0, %%r15\n"
            "   je elem_doesnt_exist\n"
            
            "   movq 1(%%r14), %%r14\n"
            "   addq %4, %%r14\n"
            
            "   jmp find_existed_elem\n"

            "elem_exists:\n"

            "   movb $1, %0\n"

            "elem_doesnt_exist:\n"
            
            : "=r" (already_exists)
            : "r" (th->ptr), "r" (th->name_len), "r" (th->name), "r" (db)
            : "r15", "r14", "r13", "r12", "r11", "memory");
            
            if(already_exists == 1) th->ptr = 0; else {

                //clock_gettime(CLOCK_REALTIME, &time1);
                
                asm volatile ("movzbq %1, %%rax\n"
                
                "jmp check_elem_type\n"

                "write_first_ptr:\n"

                "   movq $1, (%%r15)\n"
                "   movq %%rax, 8(%%r15)\n"
                "   subq %2, 8(%%r15)\n"
                
                "   movzbq %1, %%r14\n"
                
                "   leaq 10(%%rax, %%r14), %%rax\n"
                
                "   ret\n"

                "write_last_ptr:\n"
            
                "   movq (%%r15), %%r14\n"

                "   incq (%%r15)\n"

                "   movq 8(%%r15), %%r15\n"
                
                "find_last_elem:\n"
                
                "   decq %%r14\n"
                "   cmpq $0, %%r14\n"
                "   je found_last_elem\n"
                
                "   movq 1(%%r15, %2), %%r15\n"
                
                "   jmp find_last_elem\n"

                "found_last_elem:\n"
                
                "   movq %%rax, 1(%%r15, %2)\n"
                
                "   subq %2, 1(%%r15, %2)\n"
                
                "   movzbq %1, %%r14\n"
                
                "   leaq 10(%%rax, %%r14), %%rax\n"
                
                "   ret\n"

                "alloc_elem:\n"
            
                "   lock xaddq %%rax, 19(%2)\n"
            
                "   addq %2, %%rax\n"
                
                "   movb %0, (%%rax)\n"
                "   movq $0, 1(%%rax)\n"
                "   movb %1, 9(%%rax)\n"
                
                "   leaq 10(%%rax), %%rdi\n"
                "   movq %3, %%rsi\n"
                "   movzbq %1, %%rdx\n"

                "   call fill\n"
                
                "   movq 8(%6), %%r15\n"
                "   addq %2, %%r15\n"
                
                "   cmpq $0, (%%r15)\n"
                "   je write_first_ptr\n"

                "   jmp write_last_ptr\n"

                "check_elem_type:\n"

                "   cmpb $1, %0\n"
                "   je create_elem1\n"

                "   cmpb $2, %0\n"
                "   je create_elem2\n"

                "   cmpb $3, %0\n"
                "   je create_elem34\n"

                "   cmpb $4, %0\n"
                "   je create_elem34\n"

                "   cmpb $5, %0\n"
                "   je create_elem5\n"

                "   cmpb $6, %0\n"
                "   je create_elem6\n"

                "create_elem1:\n"
            
                "   addq $18, %%rax\n"

                "   call alloc_elem\n"
                
                "   movq (%4), %%r15\n"
                "   movq %%r15, (%%rax)\n"
                
                "   jmp create_elem_end\n"

                "create_elem2:\n"
            
                "   addq $18, %%rax\n"

                "   call alloc_elem\n"

                "   movq (%4), %%r15\n"
                "   movq %%r15, (%%rax)\n"

                "   jmp create_elem_end\n"

                "create_elem34:\n"
            
                "   addq $10, %%rax\n"

                "   call alloc_elem\n"

                "   jmp create_elem_end\n"

                "create_elem5:\n"
                
                "   addq $34, %%rax\n"

                "   call alloc_elem\n"
                
                "   movq %5, (%%rax)\n"
                "   movq $1, 8(%%rax)\n"
                
                "   cmpq $16, %5\n"
                "   jl create_elem5_16\n"
                
                "   movq $10, %%r14\n"
                "   addq %5, %%r14\n"
                
                "   lock xaddq %%r14, 19(%2)\n"

                "   addq %2, %%r14\n"

                "   movq %%r14, 16(%%rax)\n"
                
                "   movq %5, (%%r14)\n"
                "   movq $0, 8(%%r14)\n"

                "   leaq 16(%%r14), %%rdi\n"
                "   movq %4, %%rsi\n"
                "   movq %5, %%rdx\n"

                "   call fill\n"

                "   jmp create_elem_end\n"

                "create_elem5_16:\n"
            
                "   movq $32, %%r14\n"
            
                "   lock xaddq %%r14, 19(%2)\n"
                
                "   movq %%r14, 16(%%rax)\n"
                
                "   addq %2, %%r14\n"

                "   movq $16, (%%r14)\n"
                "   movq $0, 8(%%r14)\n"
                
                "   leaq 16(%%r14), %%rdi\n"
                "   movq %4, %%rsi\n"
                "   movq %5, %%rdx\n"
                
                "   call fill\n"

                "   jmp create_elem_end\n"

                "create_elem6:\n"
                
                "   addq $26, %%rax\n"
                
                "   call alloc_elem\n"
                
                "   movq $0, (%%rax)\n"
                "   movq $0, 8(%%rax)\n"
                
                "create_elem_end:\n"
                
                "   movq %%rax, 8(%6)\n"
                "   subq %2, 8(%6)\n"
                "   subq $10, 8(%6)\n"
                "   movzbq %1, %%r15\n"
                "   subq %%r15, 8(%6)\n"

                :: "r" (th->type), "r" (th->name_len), "r" (db), "r" (th->name), "r" (th->buf), "r" (th->value_len), "r" (th)
                : "rax", "rcx", "rdi", "rsi", "rdx", "r15", "r14", "memory");

                //clock_gettime(CLOCK_REALTIME, &time2);
                
                //printf("done in: %lu ns\n", ((time2.tv_sec - time1.tv_sec) * 1000000000 + time2.tv_nsec - time1.tv_nsec));

            }

        } else if(th->method == 12) {

            

        }

        //clock_gettime(CLOCK_REALTIME, &time2);

        //printf("done in: %lu ns\n", ((time2.tv_sec - time1.tv_sec) * 1000000000 + time2.tv_nsec - time1.tv_nsec));

        th->status = 3;

        nanosleep(&wait_free_thread, 0);

        asm volatile ("movq $200, %%rax\n"
        "movq %0, %%rdi\n"
        "movq $10, %%rsi\n"
    
        "syscall\n"
        :: "r" (th->res_tid)
        : "rax", "rdi", "rsi");

    }


}



napi_value init_db(napi_env env, napi_callback_info info) {

    //printf("size: %lu\n", sizeof(napi_env));

    //exit(0);

    ulong argc = 4;
    napi_value args[4], result;
    
    napi_get_cb_info(env, info, &argc, args, 0, 0);
    napi_get_value_uint32(env, args[0], &db_path_length);
    
    db_path = (char*)malloc(db_path_length+1);
    
    napi_get_value_string_utf8(env, args[1], db_path, db_path_length+1, 0);
    
    db_fd = open(db_path, O_RDWR);
    
    if(db_fd == -1) {

        printf("DB file with path \"%s\" doesn't exist!", db_path);

        exit(0);

    }
    
    char *db_status = (char*)malloc(4);

    read(db_fd, db_status, 3);

    db_status[3] = 0;
    
    if(strcmp(db_status, "NKB\0")) {

        fstat(db_fd, &db_file_stat);
        
        db = (char*)mmap(0, db_file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, db_fd, 0);
        
        printf("DB started with path \"%s\"\n", db_path);

    } else if(strcmp(db_status, "NEW\0")) {

        ftruncate(db_fd, 4096);

        fstat(db_fd, &db_file_stat);
        
        db = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, db_fd, 0);

        fill(db, "NKB", 3);
        
        asm volatile ("movq $0, 3(%0)\n"
        "movq $0, 11(%0)\n"
        "movq $27, 19(%0)\n"
        :: "r" (db));

        printf("DB with path \"%s\" was created!\n", db_path);

    } else {

        printf("Can't parse DB file with path \"%s\"!", db_path);

        exit(0);

    }

    free(db_status);

    napi_get_value_uint32(env, args[2], &threads_count);

    threads = new thread[threads_count];

    napi_get_value_bigint_uint64(env, args[3], &bufsize, &lossless);

    asm volatile ("pushq %0\n"
    "pushq %1\n"
    "pushq %2\n"

    "movq %2, %%r13\n"
    "movq %1, %%r14\n"
    "movq %0, %%r15\n"

    "movq $92, %%rax\n"
    "mulq %%r15\n"

    "leaq -92(%%rax, %%r13), %%r13\n"
    
    "threads_create:\n"
    "   decq %%r15\n"

    "   call thread_create\n"
    
    "   movq %%rax, (%%r13)\n"
    "   subq $92, %%r13\n"

    "   cmpq $0, %%r15\n"
    "   jne threads_create\n"

    "popq %2\n"
    "popq %1\n"
    "popq %0\n"
    
    :: "r" ((ulong)threads_count), "r" (thread_main), "r" (threads)
    : "r13", "r14", "r15", "rbx", "rcx", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory");

    threads_status = 1;

    napi_get_boolean(env, true, &result);
    
    return result;

}

napi_value request(napi_env env, napi_callback_info info) {

    timespec time1, time2;
    ulong argc = 9;
    napi_value args[9], result, key, value;
    uint tmp;
    uchar method;
    thread *th;
    ulong length = 0;

    napi_get_cb_info(env, info, &argc, args, 0, 0);
    napi_get_value_uint32(env, args[0], &tmp);

    method = (uchar)tmp;

    asm volatile ("movq %1, %%rcx\n"
    
    "movq %%rcx, %%rax\n"
    "movq $92, %%r15\n"
    "mulq %%r15\n"
    
    "leaq 91(%%rax, %2), %%r15\n"

    "jmp threads_check\n"

    "wait_interval:\n"
    "   movq $35, %%rax\n"
    "   movq %4, %%rdi\n"
    "   movq $0, %%rsi\n"
    
    "   syscall\n"

    "   movq %1, %%rcx\n"

    "   movq %%rcx, %%rax\n"
    "   movq $92, %%r15\n"
    "   mulq %%r15\n"

    "   leaq 91(%%rax, %2), %%r15\n"

    "threads_check:\n"
    "   cmpq $0, %%rcx\n"
    "   je wait_interval\n"

    "   decq %%rcx\n"
    "   subq $92, %%r15\n"

    "   movb $1, %%al\n"
    "   movb $0, (%%r15)\n"
    
    "   lock xchgb %%al, (%%r15)\n"

    "   cmpb $1, %%al\n"
    "   je threads_check\n"

    "   leaq -91(%%r15), %0\n"

    : "=r" (th)
    : "r" ((ulong)threads_count), "r" (threads), "r" (method), "r" (&wait_free_thread)
    : "rax", "al", "rdx", "rcx", "rdi", "rsi", "r15", "memory");
    
    th->res_tid = gettid();
    th->name_len = 0;
    th->value_len = 0;
    
    napi_get_value_bigint_uint64(env, args[1], &th->ptr, &lossless);

    napi_get_value_uint32(env, args[2], &tmp);
    th->path_len = (ulong)tmp;

    if(th->path_len > 0) {

        th->path = (char*)malloc(th->path_len+1);

        napi_get_value_string_utf8(env, args[3], th->path, th->path_len+1, 0);

    }

    if(method == 1) {

        th->method = 1;
        
    } else if(method == 2) {

        th->method = 2;

    } else if(method == 3) {

        th->method = 3;

    } else if(method == 4) {

        th->method = 4;

    } else if(method == 5) {

        th->method = 5;

    } else if(method == 6) {

        th->method = 6;

    } else if(method == 7) {

        th->method = 7;

    } else if(method == 8) {

        th->method = 8;

        *(napi_env*)th->buf = env;
        *(napi_value*)(th->buf+8) = args[4];

    } else if(method == 9) {

        th->method = 9;

        napi_get_value_uint32(env, args[4], (uint*)&th->value_len);

        napi_get_value_string_utf8(env, args[5], th->buf, th->value_len, 0);

    } else if(method == 10) {

        th->method = 10;

    } else if(method == 11) {

        th->method = 11;

        //length += 32;
        
        napi_get_value_uint32(env, args[4], &tmp);

        th->name_len = (uchar)tmp;

        //length += tmp;

        th->name = (char*)malloc(th->name_len+1);
        
        napi_get_value_string_utf8(env, args[5], th->name, th->name_len+1, 0);
        
        napi_get_value_uint32(env, args[6], &tmp);

        th->type = (uchar)tmp;
        
        if(tmp == 1) {
            
            napi_get_value_double(env, args[8], (double*)th->buf);
            
        } else if(tmp == 2) napi_get_value_bigint_int64(env, args[8], (long*)th->buf, &lossless);
        
        if(tmp == 5) {

            th->value_len = 0;

            napi_get_value_uint32(env, args[7], (uint*)&th->value_len);

            //length += th->value_len;

            //if(th->value_len+1 > th->bufsize) mremap(th->buf, th->bufsize, ((th->value_len+1)*2) / 4096, MAP_ANONYMOUS, -1, 0);

            napi_get_value_string_utf8(env, args[8], th->buf, th->value_len+1, 0);

        }
 
    } else if(method == 12) {

        th->method = 12;

    }


    /*if((double)db_file_stat.st_size * 0.7 < (*(ulong*)(db+19)) + length) {

        db = (char*)mremap(db, db_file_stat.st_size, db_file_stat.st_size + (ulong)((double)db_file_stat.st_size * 0.3 + length) / 4096, MAP_SHARED, db_fd, 0);

    }*/


    th->status = 2;

    //clock_gettime(CLOCK_REALTIME, &time1);

    asm volatile ("movq $200, %%rax\n"
    "movq %0, %%rdi\n"
    "movq $10, %%rsi\n"
        
    "syscall\n"
        
    :: "r" (th->tid)
    : "rax", "rdi", "rsi");

    nanosleep(&wait_free_thread, 0);
    
    if(th->status != 3) asm volatile ("wait_response:\n"
    "   movq $34, %%rax\n"
    "   syscall\n"
        
    "   cmpb $3, (%0)\n"
    "   jne wait_response\n"

    :: "r" (&th->status)
    : "rax", "memory");
    
    //clock_gettime(CLOCK_REALTIME, &time2);

    //printf("done in: %lu ns\n", ((time2.tv_sec - time1.tv_sec) * 1000000000 + time2.tv_nsec - time1.tv_nsec));
    

    if(method == 1) {

        if(th->ptr == 0) {
            
            napi_get_null(env, &result);

        } else {
            
            napi_create_object(env, &result);

            napi_create_string_utf8(env, "ptr", 3, &key);
            napi_create_bigint_uint64(env, (ulong)th->ptr, &value);
            napi_set_property(env, result, key, value);

            napi_create_string_utf8(env, "type", 4, &key);
            napi_create_uint32(env, (uint)th->type, &value);
            napi_set_property(env, result, key, value);


        }
            
    } else if(method == 2) {

        if(th->ptr == 0) {

            napi_get_null(env, &result);

        } else {
            
            if(th->type == 1) napi_create_double(env, *(double*)th->buf, &result); else
            if(th->type == 2) napi_create_bigint_int64(env, *(long*)th->buf, &result); else
            if(th->type == 3) napi_get_boolean(env, true, &result); else
            if(th->type == 4) napi_get_boolean(env, false, &result); else
            if(th->type == 5) napi_create_string_utf8(env, th->buf, th->value_len, &result);

        }

    } else if(method == 3) {

            

    } else if(method == 4) {

        if(th->ptr == 0) {
            
            napi_get_null(env, &result);

        } else {
            
            napi_create_uint32(env, *(uint*)(th->buf), &result);

        }

    } else if(method == 5) {

        if(th->ptr == 0) {
            
            napi_get_null(env, &result);

        } else {
            
            napi_create_array_with_length(env, th->value_len, &result);

            if(th->value_len != 0) {

                napi_value elem_name;
                char *ptr = th->buf;

                for(ulong i = 0; i < th->value_len; i++) {

                    napi_create_string_utf8(env, (ptr+1), *ptr, &elem_name);
                    napi_set_element(env, result, i, elem_name);

                    ptr += *ptr + 1;

                }

            }

        }

    } else if(method == 6) {

            

    } else if(method == 7) {

            

    } else if(method == 8) {

        if(th->ptr == 0) {

            napi_get_null(env, &result);

        } else if(th->type == 1) napi_create_double(env, *(double*)th->buf, &result); else if(th->type == 2) napi_create_bigint_int64(env, *(ulong*)th->buf, &result); else if(th->type == 3) napi_get_boolean(env, true, &result); else if(th->type == 4) napi_get_boolean(env, false, &result); else napi_create_string_utf8(env, th->buf, th->value_len, &result);

    } else if(method == 9) {

        if(th->ptr == 0) {

            napi_get_null(env, &result);

        } else if(th->type == 1) napi_create_double(env, *(double*)th->buf, &result); else napi_create_bigint_int64(env, *(ulong*)th->buf, &result);

    } else if(method == 10) {

        if(th->ptr == 0) {

            napi_get_null(env, &result);

        } else napi_create_string_utf8(env, th->buf, th->value_len, &result);

    } else if(method == 11) {

        if(th->ptr == 0) {
            
            napi_get_null(env, &result);

        } else {
            
            napi_create_object(env, &result);
            
            napi_create_string_utf8(env, "ptr", 3, &key);
            napi_create_bigint_uint64(env, (ulong)th->ptr, &value);
            napi_set_property(env, result, key, value);

            napi_create_string_utf8(env, "type", 4, &key);
            napi_create_uint32(env, (uint)th->type, &value);
            napi_set_property(env, result, key, value);

        }

    } else if(method == 12) {

        

    }
    

    if(th->path_len != 0) free(th->path);
    if(th->name_len != 0) free(th->name);


    return result;

}


/*

error codes:

    0: success
    1: already exists
    2: doesn't exists
    3: types don't match

*/


napi_value Init(napi_env env, napi_value exports) {

    wait_creating_threads.tv_sec = 0;
    wait_creating_threads.tv_nsec = 10000;

    wait_free_thread.tv_sec = 0;
    wait_free_thread.tv_nsec = 2000;

    wait_signal.tv_sec = 1;
    wait_signal.tv_nsec = 0;

    threads_polling_interval.tv_sec = 0;
    threads_polling_interval.tv_nsec = 100000;


    napi_value fn;

    napi_create_function(env, NULL, 0, init_db, NULL, &fn);
    napi_set_named_property(env, exports, "init_db", fn);

    napi_create_function(env, NULL, 0, request, NULL, &fn);
    napi_set_named_property(env, exports, "request", fn);
    
    return exports;

}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)



/*

asm volatile ("movzbq %3, %%rax\n"
                
                "jmp check_elem_type\n"

                "write_first_ptr:\n"

                "   movq $1, (%%r15)\n"
                "   movq %%rax, 8(%%r15)\n"
                "   subq %1, 8(%%r15)\n"
                
                "   movzbq %3, %%r14\n"
                
                "   leaq 10(%%rax, %%r14), %%rax\n"
                
                "   ret\n"

                "write_last_ptr:\n"
            
                "   movq (%%r15), %%r14\n"

                "   incq (%%r15)\n"

                "   movq 8(%%r15), %%r15\n"
                
                "find_last_elem:\n"
                
                "   decq %%r14\n"
                "   cmpq $0, %%r14\n"
                "   je found_last_elem\n"
                
                "   movq 1(%%r15, %1), %%r15\n"
                
                "   jmp find_last_elem\n"

                "found_last_elem:\n"
                
                "   movq %%rax, 1(%%r15, %1)\n"
                
                "   subq %1, 1(%%r15, %1)\n"
                
                "   movzbq %3, %%r14\n"
                
                "   leaq 10(%%rax, %%r14), %%rax\n"
                
                "   ret\n"

                "alloc_elem:\n"
            
                "   lock xaddq %%rax, 19(%1)\n"
            
                "   addq %1, %%rax\n"
                
                "   movb %0, (%%rax)\n"
                "   movq $0, 1(%%rax)\n"
                "   movb %3, 9(%%rax)\n"
                
                "   leaq 10(%%rax), %%rdi\n"
                "   movq 57(%2), %%rsi\n"
                "   movzbq %3, %%rdx\n"

                "   call fill\n"
                
                "   movq 8(%2), %%r15\n"
                "   addq %1, %%r15\n"
                
                "   cmpq $0, (%%r15)\n"
                "   je write_first_ptr\n"

                "   jmp write_last_ptr\n"

                "check_elem_type:\n"

                "   cmpb $1, %0\n"
                "   je create_elem1\n"

                "   cmpb $2, %0\n"
                "   je create_elem2\n"

                "   cmpb $3, %0\n"
                "   je create_elem34\n"

                "   cmpb $4, %0\n"
                "   je create_elem34\n"

                "   cmpb $5, %0\n"
                "   je create_elem5\n"

                "   cmpb $6, %0\n"
                "   je create_elem6\n"

                "create_elem1:\n"
            
                "   addq $18, %%rax\n"

                "   call alloc_elem\n"
                
                "   movq 75(%2), %%r15\n"
                "   movq %%r15, (%%rax)\n"
                
                "   jmp create_elem_end\n"

                "create_elem2:\n"
            
                "   addq $18, %%rax\n"

                "   call alloc_elem\n"

                "   movq 75(%2), %%r15\n"
                "   movq %%r15, (%%rax)\n"

                "   jmp create_elem_end\n"

                "create_elem34:\n"
            
                "   addq $10, %%rax\n"

                "   call alloc_elem\n"

                "   jmp create_elem_end\n"

                "create_elem5:\n"
                
                "   addq $34, %%rax\n"

                "   call alloc_elem\n"
                
                "   movq %4, (%%rax)\n"
                "   movq $1, 8(%%rax)\n"
                
                "   cmpq $16, %4\n"
                "   jl create_elem5_16\n"
                
                "   movq $10, %%r14\n"
                "   addq %4, %%r14\n"
                
                "   lock xaddq %%r14, 19(%1)\n"

                "   movq %%r14, 16(%%rax)\n"

                "   addq %1, %%r14\n"
                
                "   movq %4, (%%r14)\n"
                "   movq $0, 8(%%r14)\n"

                "   leaq 16(%%r14), %%rdi\n"
                "   movq 75(%2), %%rsi\n"
                "   movq %4, %%rdx\n"

                "   call fill\n"

                "   jmp create_elem_end\n"

                "create_elem5_16:\n"
            
                "   movq $26, %%r14\n"
            
                "   lock xaddq %%r14, 19(%1)\n"
                
                "   movq %%r14, 16(%%rax)\n"
                
                "   addq %1, %%r14\n"

                "   movq $16, (%%r14)\n"
                "   movq $0, 8(%%r14)\n"
                
                "   leaq 16(%%r14), %%rdi\n"
                "   movq 75(%2), %%rsi\n"
                "   movq %4, %%rdx\n"
                
                "   call fill\n"

                "   jmp create_elem_end\n"

                "create_elem6:\n"
                
                "   addq $26, %%rax\n"
                
                "   call alloc_elem\n"
                
                "   movq $0, (%%rax)\n"
                "   movq $0, 8(%%rax)\n"
                
                "create_elem_end:\n"
                
                "   movq %%rax, 8(%2)\n"
                "   subq %1, 8(%2)\n"
                "   subq $10, 8(%2)\n"
                "   movzbq %3, %%r15\n"
                "   subq %%r15, 8(%2)\n"

                :: "r" (th->type), "r" (db), "r" (th), "r" (th->name_len), "r" (th->value_len)
                : "rax", "rcx", "rdi", "rsi", "rdx", "r15", "r14", "memory");


*/