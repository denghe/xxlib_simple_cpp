#pragma once
#include <vector>
#include <thread>
#include <iostream>
#include<sstream>
#include <fstream>
//#include "xx_append.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <thread>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

enum LOG_TYPE
{
    LOG_TYPE_DEBUG,
    LOG_TYPE_WARNING,
    LOG_TYPE_ERROR,
    LOG_TYPE_INFO,
};

#define LOG_INFO(...) {\
	std::string s1;\
	xx::Append(s1,__VA_ARGS__);\
	SLOG_INFO(s1.c_str());\
}

#define LOG_ERR(...) {\
	std::string s1;\
	xx::Append(s1,__VA_ARGS__);\
	SLOG_ERROR(s1.c_str());\
}

#define LOG_WARN(...) {\
	std::string s1;\
	xx::Append(s1,__VA_ARGS__);\
	SLOG_WARN(s1.c_str());\
}

#define LOG_DEBUG(...) {\
	std::string s1;\
	xx::Append(s1,__VA_ARGS__);\
	SLOG_DEBUG(s1.c_str());\
}

#define LOG_TRACE(...) {\
	std::string s1;\
	xx::Append(s1,__VA_ARGS__);\
	SLOG_TRACE(s1.c_str());\
}

int SLOG_INIT(const char* config);
void SLOG_UNINIT();

//#ifdef __SLOG_FULL__
//#define PRE_STR "[%s:%s(%d)] ... "
//#define VAL_STR ,__FILE__,__FUNCTION__,__LINE__
//#else
#define PRE_STR
#define VAL_STR
//#endif

#define SLOG_TRACE(format, ...) slog_trace(__FILE__, __LINE__, __FUNCTION__,PRE_STR format VAL_STR ,##__VA_ARGS__)
#define SLOG_DEBUG(format, ...) slog_debug(__FILE__, __LINE__, __FUNCTION__,PRE_STR format VAL_STR ,##__VA_ARGS__)
#define SLOG_INFO(format, ...) slog_info(__FILE__, __LINE__, __FUNCTION__,PRE_STR format VAL_STR ,##__VA_ARGS__)
#define SLOG_WARN(format, ...) slog_warn(__FILE__, __LINE__, __FUNCTION__,PRE_STR format VAL_STR ,##__VA_ARGS__)
#define SLOG_ERROR(format, ...) slog_error(__FILE__, __LINE__, __FUNCTION__,PRE_STR format VAL_STR ,##__VA_ARGS__)

//void slog_trace(const char* file, int line, const char* func, const char *fmt, ...);
//void slog_debug(const char* file, int line, const char* func, const char *fmt, ...);
//void slog_info(const char* file, int line, const char* func, const char *fmt, ...);
//void slog_warn(const char* file, int line, const char* func, const char *fmt, ...);
//void slog_error(const char* file, int line, const char* func, const char *fmt, ...);


//--------------------------------------------------实现部分---------------------------------------//
//--------------------------------------------------实现部分---------------------------------------//
//--------------------------------------------------实现部分---------------------------------------//

//应用层每次写入log最大长度
#define BUFSIZE 20480

typedef enum __log_level
{
    SLOG_LEVEL_TRACE = 0,
    SLOG_LEVEL_DEBUG,
    SLOG_LEVEL_INFO,
    SLOG_LEVEL_WARN,
    SLOG_LEVEL_ERROR,
    SLOG_LEVEL_NOLOG
}LOG_LEVEL;

typedef struct _data_node_
{
    struct _data_node_* next;
    struct _data_node_* prev;
    int str_len;
}DataNode;

//configure sample
/*
 * ### log级别
 * slog_level=DEBUG
 * ### log文件名
 * slog_log_name=../log/server.log
 * ### log 文件最大大小(单位M)
 * slog_log_maxsize=10M
 * ### log文件最多个数
 * slog_log_maxcount=10
 * ### log缓冲区大小(单位KB,默认512KB)
 * slog_flush_size=1024
 * ### log缓冲刷新间隔(单位s,默认1s)
 * slog_flush_interval=2
 * ### log动态更新配置参数的时间间隔(单位s,默认60s)
 * config_update_interval=30
*/

typedef struct _slog_setting_
{
    char config_name[128];  //配置文件名
    char log_name[128];     //输出文件名
    LOG_LEVEL log_level;    //log的级别(默认DEBUG)
    FILE* log_file;         //当前打开到log文件(默认stdout)
    int log_size;           //当前log文件大小
    pthread_t thread_id;    //刷新线程
    int flush_size;         //当缓存大小超过该值时进行刷新log的缓存到slog_file(单位KB,默认512KB)
    int flush_interval;     //刷新log的缓存到slog_file的时间间隔(单位s,默认1s)
    int config_update_interval; //更新配置文件的时间间隔(单位s,默认60s)
    int need_output_ctl;	//是否需要输出到控制台

    //当slog_file为stdout时以下3个字段无效
    int log_count;          //当前最大文件索引号
    int log_max_size;       //每个log数据文件到最大大小(单位M,默认10M)
    int log_max_count;      //log文件到个数(默认10个)

    int buf_size;           //当前缓冲区大小
    DataNode buf_list;      //缓冲链表

    pthread_mutex_t mutex;
    pthread_mutex_t cond_mutex;
    pthread_cond_t cond;
    int exit_flush_thread;
}SlogSetting;
inline SlogSetting g_slog_setting;

//创建刷新线程
static void create_flush_thread();
//刷新线程
static void* flush_thread(void* user_data);
//刷新log缓冲区到输出文件
static void flush_buffer_to_file();

//打印配置参数
static void print_config();
//打印时间格式
static void format_time(char* buf);
//格式化数据
static void format_to_string(char* str, const char* file, int line, const char* func, int str_size, const char* log_level, const char* fmt, va_list* args);
//将临时数据输出到log的缓冲区
static void output_string_to_buffer(const char* str);
//通知线程刷新缓冲区
static void notify_flush();	//通知刷新线程
//当log文件满时,移动log文件
static void rename_log_file();

inline int SLOG_INIT(const char* config)
{
    static int is_first_invoke = 1;
    if (is_first_invoke)
    {
        is_first_invoke = 0;
        g_slog_setting.config_name[0] = '\0';
        g_slog_setting.log_name[0] = '\0';
        g_slog_setting.log_level = SLOG_LEVEL_DEBUG;
        g_slog_setting.log_file = stdout;
        g_slog_setting.log_size = 0;
        g_slog_setting.flush_size = 512 * 1024;	//默认512KB
        g_slog_setting.flush_interval = 1;		//默认刷新间隔1s
        g_slog_setting.config_update_interval = 60;		//默认60s
        g_slog_setting.buf_size = 0;		//slog缓冲区大小
        g_slog_setting.buf_list.next = NULL;		//slog缓冲区链表
        g_slog_setting.buf_list.prev = NULL;		//slog缓冲区链表
        //the 3 elements are invalid when slog_file is set to stdout
        g_slog_setting.log_max_size = 1000;		//10M
        g_slog_setting.log_max_count = 10;		//10 log files
        g_slog_setting.log_count = 0;		//当前生成的log文件数

        g_slog_setting.exit_flush_thread = 0;
        g_slog_setting.need_output_ctl = 1;
        pthread_mutex_init(&g_slog_setting.mutex, NULL);
        pthread_mutex_init(&g_slog_setting.cond_mutex, NULL);
        pthread_cond_init(&g_slog_setting.cond, NULL);
        create_flush_thread();
    }

    if (config == NULL || strlen(config) == 0)
    {
        fprintf(stderr, "WARN!!! slog config file is invalid !!!\n");
        print_config();
        return 0;
    }

    pthread_mutex_lock(&g_slog_setting.mutex);  //刷新线程会调用SLOG_INIT函数来更新配置参数

    char buf[1024];
    FILE* file = NULL;
    if ((file = fopen(config, "r")) == NULL)
    {
        pthread_mutex_unlock(&g_slog_setting.mutex);
        fprintf(stderr, "can't open slog config file:%s !!!\n", config);
        return 0;
    }
    //save the configure name
    if (g_slog_setting.config_name[0] == '\0')
        strcpy(g_slog_setting.config_name, config);
    //parse configure file
    while (fgets(buf, 1024, file) != NULL)  //read one line
    {
        int i = 0, len = (int)strlen(buf);
        while (i < len && (buf[i] == ' ' || buf[i] == '\t'))
            ++i;
        if (buf[i] == '\n' || buf[i] == '\r' || buf[i] == '#' || buf[i] == '/')	//属于空行或者注释
            continue;

        //get key and value
        char* key = strtok(&buf[i], "=");
        if (key == NULL)
            continue;
        char* value = strtok(NULL, "=");
        if (value == NULL)
            continue;
        //parse key and value
        if (strcmp(key, "slog_level") == 0)
        {
            char str[30];
            sscanf(value, "%s", str);
            if (strcmp(str, "TRACE") == 0)
                g_slog_setting.log_level = SLOG_LEVEL_TRACE;
            else if (strcmp(str, "DEBUG") == 0)
                g_slog_setting.log_level = SLOG_LEVEL_DEBUG;
            else if (strcmp(str, "INFO") == 0)
                g_slog_setting.log_level = SLOG_LEVEL_INFO;
            else if (strcmp(str, "WARN") == 0)
                g_slog_setting.log_level = SLOG_LEVEL_WARN;
            else if (strcmp(str, "ERROR") == 0)
                g_slog_setting.log_level = SLOG_LEVEL_ERROR;
        }
        else if (strcmp(key, "slog_log_name") == 0)
        {
            if (g_slog_setting.log_name[0] != '\0')
                continue;
            char log_filename[128];
            sscanf(value, "%s", log_filename);
            time_t now = time(NULL);
            struct tm* t = localtime(&now);
            char createTime[128];
            sprintf(createTime, "-%d-%d-%d+%d:%d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
            strcpy(g_slog_setting.log_name, log_filename);
            strcat(g_slog_setting.log_name, createTime);
            g_slog_setting.log_file = fopen(g_slog_setting.log_name, "w");
            if (NULL == g_slog_setting.log_file)
            {
                g_slog_setting.log_file = stdout;
                fprintf(stderr, "ERROR!!! open log file=%s failed.use stdout.", log_filename);
            }
        }
        else if (strcmp(key, "slog_log_maxsize") == 0)
        {
            sscanf(value, "%d", &g_slog_setting.log_max_size);
        }
        else if (strcmp(key, "slog_log_maxcount") == 0)
        {
            sscanf(value, "%d", &g_slog_setting.log_max_count);
        }
        else if (strcmp(key, "slog_flush_size") == 0)
        {
            sscanf(value, "%d", &g_slog_setting.flush_size);
            g_slog_setting.flush_size *= 1024;
        }
        else if (strcmp(key, "slog_flush_interval") == 0)
        {
            sscanf(value, "%d", &g_slog_setting.flush_interval);
        }
        else if (strcmp(key, "config_update_interval") == 0)
        {
            sscanf(value, "%d", &g_slog_setting.config_update_interval);
        }
        else if (strcmp(key, "need_output_ctl") == 0)
        {
            sscanf(value, "%d", &g_slog_setting.need_output_ctl);
        }
    }//while
    fclose(file);
    pthread_mutex_unlock(&g_slog_setting.mutex);

    //print_config();
    return 0;
}

inline void SLOG_UNINIT()
{
    char str[BUFSIZE];
    format_time(str);
    sprintf(str + strlen(str), "[SLOG UNINIT]...");
    output_string_to_buffer(str);

    g_slog_setting.exit_flush_thread = 1;
    pthread_join(g_slog_setting.thread_id, NULL);
    //printf("slog flush thread exit.\n");

    if (g_slog_setting.log_file != NULL && g_slog_setting.log_file != stdout)
        fclose(g_slog_setting.log_file);
    pthread_mutex_destroy(&g_slog_setting.mutex);
    pthread_mutex_destroy(&g_slog_setting.cond_mutex);
    pthread_cond_destroy(&g_slog_setting.cond);
}

inline void slog_trace(const char* file, int line, const char* func, const char* fmt, ...)
{
    if (SLOG_LEVEL_TRACE >= g_slog_setting.log_level)
    {
        char str[BUFSIZE];
        va_list args;

        //格式化
        va_start(args, fmt);
        format_to_string(str, file, line, func, BUFSIZE, "TRACE", fmt, &args);
        va_end(args);
        //输出
        output_string_to_buffer(str);
    }
}

inline void slog_debug(const char* file, int line, const char* func, const char* fmt, ...)
{
    if (SLOG_LEVEL_DEBUG >= g_slog_setting.log_level)
    {
        char str[BUFSIZE];
        va_list args;

        //格式化
        va_start(args, fmt);
        format_to_string(str, file, line, func, BUFSIZE, "DEBUG", fmt, &args);
        va_end(args);
        //输出
        output_string_to_buffer(str);
    }
}

inline void slog_info(const char* file, int line, const char* func, const char* fmt, ...)
{
    if (SLOG_LEVEL_INFO >= g_slog_setting.log_level)
    {
        char str[BUFSIZE];
        va_list args;

        //格式化
        va_start(args, fmt);
        format_to_string(str, file, line, func, BUFSIZE, "INFO ", fmt, &args);
        va_end(args);
        //输出
        output_string_to_buffer(str);
    }
}
inline void slog_warn(const char* file, int line, const char* func, const char* fmt, ...)
{
    if (SLOG_LEVEL_WARN >= g_slog_setting.log_level)
    {
        char str[BUFSIZE];
        va_list args;

        //格式化
        va_start(args, fmt);
        format_to_string(str, file, line, func, BUFSIZE, "WARN ", fmt, &args);
        va_end(args);
        //输出
        output_string_to_buffer(str);
    }
}

inline void slog_error(const char* file, int line, const char* func, const char* fmt, ...)
{
    if (SLOG_LEVEL_ERROR >= g_slog_setting.log_level)
    {
        char str[BUFSIZE];
        va_list args;

        //格式化
        va_start(args, fmt);
        format_to_string(str, file, line, func, BUFSIZE, "ERROR", fmt, &args);
        va_end(args);
        //输出
        output_string_to_buffer(str);
    }
}


/////////////////////////////  static function  /////////////////////////////////
//格式化时间
inline void format_time(char* buf)
{
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

//打印当前配置参数
inline void print_config()
{
    char str[512];
    format_time(str);
    sprintf(str + strlen(str), "[SLOG INIT]current config of SLOG"
                               ":log_level=%d"
                               ",log_file=%s"
                               ",max_log_size=%d(M)"
                               ",max_log_count=%d"
                               ",flush_size=%d"
                               ",flush_interval=%d"
                               ",config_update_interval=%d."
            , g_slog_setting.log_level
            , g_slog_setting.log_name[0] != '\0' ? g_slog_setting.log_name : "stdout"
            , g_slog_setting.log_max_size
            , g_slog_setting.log_max_count
            , g_slog_setting.flush_size
            , g_slog_setting.flush_interval
            , g_slog_setting.config_update_interval);

    output_string_to_buffer(str);
}

//格式化输入内容到buf
inline void format_to_string(char* str, const char* file, int line, const char* func, int str_size, const char* log_level, const char* fmt, va_list* args)
{
    if (str == NULL || str_size < 100)  //打印时间格式需要预留空间
        return;

    format_time(str);
    if (log_level != NULL)
    {
        std::string buf(file);
        size_t nPos = buf.find_last_of("/\\");
        if (nPos != std::string::npos)
        {
            file = buf.c_str() + nPos + 1;
        }
        sprintf(str + strlen(str), "[%s][file:%s line:%d func:%s]", log_level, file, line, func);
    }
    auto len = strlen(str);
    vsnprintf(str + len, str_size - len, fmt, *args);
}

//输出buf的内容(以'\0'结束)
inline void output_string_to_buffer(const char* str)
{
    //判断线程是否已经挂掉
    int kill_result = pthread_kill(g_slog_setting.thread_id, 0);
    if (kill_result == ESRCH)
        create_flush_thread();

    int len = (int)strlen(str) + 2;  //include '\n' and '\0'
    char* buf = (char*)calloc(sizeof(DataNode) + len, 1);
    DataNode* data_node = (DataNode*)buf;
    data_node->str_len = len - 1; //exclude '\0'
    buf += sizeof(DataNode);
    strcpy(buf, str);
    buf[data_node->str_len - 1] = '\n';

    //将数据插入到链表头(逆序)
    pthread_mutex_lock(&g_slog_setting.mutex);
    data_node->prev = NULL;
    data_node->next = g_slog_setting.buf_list.next;
    g_slog_setting.buf_list.next = data_node;
    if (data_node->next != NULL)
        data_node->next->prev = data_node;
    if (g_slog_setting.buf_list.prev == NULL)
        g_slog_setting.buf_list.prev = data_node;

    g_slog_setting.buf_size += len;
    if (g_slog_setting.buf_size >= g_slog_setting.flush_size)
        notify_flush();	//通知线程刷新缓冲区
    pthread_mutex_unlock(&g_slog_setting.mutex);
    return;
}

inline void notify_flush()
{
    //printf("notify_flush\n");
    pthread_mutex_lock(&g_slog_setting.cond_mutex);
    pthread_cond_signal(&g_slog_setting.cond);
    pthread_mutex_unlock(&g_slog_setting.cond_mutex);
    return;
}

//slog的刷新线程
inline void* flush_thread(void* user_data)
{
    struct timeval start, last_update;
    struct timespec timeout;

    last_update.tv_sec = 0;
    while (g_slog_setting.exit_flush_thread != 1)
    {
        gettimeofday(&start, NULL);
        if (last_update.tv_sec == 0)
            last_update.tv_sec = start.tv_sec;

        timeout.tv_sec = start.tv_sec + g_slog_setting.flush_interval;
        timeout.tv_nsec = start.tv_usec * 1000;
        //等待超时或者刷新通知
        pthread_mutex_lock(&g_slog_setting.cond_mutex);
        pthread_cond_timedwait(&g_slog_setting.cond, &g_slog_setting.cond_mutex, &timeout);
        pthread_mutex_unlock(&g_slog_setting.cond_mutex);
        //将缓冲区数据刷新到log文件
        flush_buffer_to_file();

        gettimeofday(&start, NULL);
        if (start.tv_sec - last_update.tv_sec > g_slog_setting.config_update_interval)
        {
            //printf("init...\n");
            SLOG_INIT(g_slog_setting.config_name);  //update the configure at short intervals
            last_update.tv_sec = start.tv_sec;
        }
    }
    //将缓冲区的剩余数据刷新到log文件
    flush_buffer_to_file();
    return NULL;
}

inline void create_flush_thread()
{
    if (pthread_create(&g_slog_setting.thread_id, NULL, flush_thread, (void*)NULL) != 0)
    {
        fprintf(g_slog_setting.log_file, "ERROR!!! create flush thread failed. exit !!!");
        exit(1);
    }
}

inline void flush_buffer_to_file()
{
    //printf("flush to file\n");
    DataNode* temp = NULL, * data_list = NULL;
    pthread_mutex_lock(&g_slog_setting.mutex);
    data_list = g_slog_setting.buf_list.prev;
    g_slog_setting.buf_list.next = g_slog_setting.buf_list.prev = NULL;
    g_slog_setting.buf_size = 0;
    pthread_mutex_unlock(&g_slog_setting.mutex);

    int log_size = g_slog_setting.log_max_size * 1024 * 1024;
    auto det = data_list;
    for (temp = data_list; temp; temp = data_list)
    {
        data_list = temp->prev;
        if (g_slog_setting.need_output_ctl && g_slog_setting.log_file != stdout)
            fwrite((char*)temp + sizeof(DataNode), temp->str_len, 1, stdout);
        fwrite((char*)temp + sizeof(DataNode), temp->str_len, 1, g_slog_setting.log_file);
        g_slog_setting.log_size += temp->str_len; //include '\n'
        if (g_slog_setting.log_size >= log_size) //重新创建新的log文件
            rename_log_file();
    }
    for (; det != NULL;)
    {
        auto q = det;
        det = det->prev;
        free(q);
    }
    if (g_slog_setting.need_output_ctl && g_slog_setting.log_file != stdout) {
        fflush(stdout);
    }
    fflush(g_slog_setting.log_file);
}

inline void rename_log_file()
{
    //printf("rename log file\n");
    g_slog_setting.log_size = 0;
    if (g_slog_setting.log_name[0] == '\0' || g_slog_setting.log_file == stdout)
        return;
    fclose(g_slog_setting.log_file);

    int i = g_slog_setting.log_count;
    char old_log_name[225], new_log_name[225];
    while (i > 0)
    {
        snprintf(old_log_name, sizeof(old_log_name), "%s.%d", g_slog_setting.log_name, i);
        snprintf(new_log_name, sizeof(new_log_name), "%s.%d", g_slog_setting.log_name, i + 1);
        if (i == g_slog_setting.log_max_count)
            remove(old_log_name);
        else
            rename(old_log_name, new_log_name);
        --i;
    }

    snprintf(old_log_name, sizeof(old_log_name), "%s", g_slog_setting.log_name);
    snprintf(new_log_name, sizeof(new_log_name), "%s.%d", g_slog_setting.log_name, i + 1);
    rename(old_log_name, new_log_name);

    if (g_slog_setting.log_count < g_slog_setting.log_max_count)
        ++g_slog_setting.log_count;

    g_slog_setting.log_file = fopen(old_log_name, "w");
    if (g_slog_setting.log_file == NULL)
    {
        g_slog_setting.log_file = stdout;
        fprintf(stderr, "ERROR!!! open log file=%s failed.use stdout.", old_log_name);
    }
}

