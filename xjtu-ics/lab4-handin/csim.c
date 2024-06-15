#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cachelab.h"

typedef struct
{
    int valid;     // 有效位
    int tag;       // 标记位
    int timestamp; // 时间戳
} Cache_line;

typedef struct
{
    int S;
    int E;
    int B;
    Cache_line **line;
} Cache;

int hit_count = 0, miss_count = 0, eviction_count = 0; // 记录命中、缺失和替换次数
char t[1000];
Cache *cache = NULL;

void cache_init(int s, int E, int b)
{

    cache = (Cache *)malloc(sizeof(Cache));
    cache->S = 1 << s;
    cache->E = E;
    cache->B = 1 << b;
    cache->line = (Cache_line **)malloc(sizeof(Cache_line *) * (1 << s));
    // 初始化Cache行
    int S = 1 << s;
    for (int i = 0; i < S; i++)
    {
        cache->line[i] = (Cache_line *)malloc(sizeof(Cache_line) * E);

        for (int j = 0; j < E; j++)
        {
            Cache_line *line = &cache->line[i][j];

            line->valid = 0; // 初始时，高速缓存是空的
            line->tag = -1;
            line->timestamp = 0;
        }
    }
}

void update(int i, int op_s, int op_tag)
{
    Cache_line *line = &cache->line[op_s][i];
    line->tag = op_tag;
    line->valid = 1;
    // 更新时间戳
    for (int j = 0; j < cache->E; j++)
    {
        if (cache->line[op_s][j].valid)
        {
            cache->line[op_s][j].timestamp++;
        }
    }
    line->timestamp = 0;
}

void update_info(int op_tag, int op_s)
{
    int max_index = 0;
    int index = -1;
    int empty_index = -1;
    int max_timestamp = 0;
    // 遍历Cache Set中的每一行
    for (int i = 0; i < cache->E; i++)
    {
        // 如果该行有效且标记与操作标记相同，则发生了Cache Hit
        if (cache->line[op_s][i].tag == op_tag && cache->line[op_s][i].valid)
        {
            index = i; // 记录索引
            break;     // 停止查找
        }
        // 记录第一个空闲的Cache行的索引
        if (cache->line[op_s][i].valid == 0 && empty_index == -1)
        {
            empty_index = i;
        }
        // 记录时间戳最大的Cache行的索引
        if (cache->line[op_s][i].timestamp > max_timestamp)
        {
            max_index = i;
            max_timestamp = cache->line[op_s][i].timestamp;
        }
    }
    // 如果没有找到相同标记的Cache行，则发生了Cache Miss
    if (index == -1)
    {
        miss_count++;
        // 如果存在空闲的Cache行，则使用该行进行更新
        if (empty_index != -1)
        {
            update(empty_index, op_s, op_tag);
        }
        else
        {
            eviction_count++;
            update(max_index, op_s, op_tag); // 选择时间戳最大的行进行替换
        }
    }
    else
    {
        hit_count++; // 否则发生了Cache Hit
        update(index, op_s, op_tag);
    }
}

void process_operation(char identifier, int op_tag, int op_s)
{
    update_info(op_tag, op_s);
    if (identifier == 'M')
    {
        update_info(op_tag, op_s);
    }
}

void get_trace(int s, int E, int b)
{
    FILE *pFile = fopen(t, "r");
    if (pFile == NULL)
    {
        exit(-1);
    }
    char identifier;
    unsigned int address;
    int size;
    // 读取形如 " M 20,5" 或 "L 17,3" 的行
    while (fscanf(pFile, " %c %x,%d", &identifier, &address, &size) > 0)
    {
        // 提取标记位和组序号
        int op_tag = (address >> (s + b));
        int op_s = (address >> b) & ((1 << s) - 1);
        // 根据操作类型进行处理
        switch (identifier)
        {
        case 'M':
        case 'L':
        case 'S':
            process_operation(identifier, op_tag, op_s);
            break;
        default:
            // 处理未知操作类型
            break;
        }
    }
    fclose(pFile);
}

int main(int argc, char *argv[])
{
    char opt;
    int s, E, b;
    opt = getopt(argc, argv, "hvs:E:b:t:");
    while (opt != -1)
    {
        switch (opt)
        {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            strcpy(t, optarg);
            break;
        default:
            printf("wrong argument\n");
            exit(-1);
        }
        opt = getopt(argc, argv, "hvs:E:b:t:");
    }
    // 初始化Cache
    cache_init(s, E, b);
    // 读取trace文件并处理操作
    get_trace(s, E, b);
    // 释放缓存和相关资源
    for (int i = 0; i < cache->S; i++)
    {
        free(cache->line[i]);
    }
    free(cache->line);
    free(cache);
    // 打印结果
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
