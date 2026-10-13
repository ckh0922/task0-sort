#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int compare_lines(const void *a, const void *b) {
    const char *left = *(const char **)a;
    const char *right = *(const char **)b;

    return strcmp(left, right);
}

int compare_lines_reverse(const void *a, const void *b) {
    const char *left = *(const char **)a;
    const char *right = *(const char **)b;

    return strcmp(right, left);
}

void sort_and_write(char **lines, size_t count, int reverse, FILE *output) {
    if (reverse) {
        qsort(lines, count, sizeof(char *), compare_lines_reverse);
    } else {
        qsort(lines, count, sizeof(char *), compare_lines);
    }

    for (size_t i = 0; i < count; i++) {
        fputs(lines[i], output);
    }
}

void merge_chunks(FILE **chunks, size_t chunk_count, int reverse) {
    if (chunk_count == 0) {
        return;
    }
    
    char **current_lines = calloc(chunk_count, sizeof(char *));
    size_t *capacities = calloc(chunk_count, sizeof(size_t));

    if (current_lines == NULL || capacities == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    // 回到每个临时文件开头，并读取第一行
    for (size_t i = 0; i < chunk_count; i++) {
        rewind(chunks[i]);
        getline(&current_lines[i], &capacities[i], chunks[i]);
    }

    while (1) {
        size_t best = chunk_count;

        // 找出当前应该输出的那一行
        for (size_t i = 0; i < chunk_count; i++) {
            if (current_lines[i] == NULL) {
                continue;
            }

            if (best == chunk_count) {
                best = i;
            } else {
                int cmp = strcmp(current_lines[i], current_lines[best]);

                if ((!reverse && cmp < 0) || (reverse && cmp > 0)) {
                    best = i;
                }
            }
        }

        // 所有文件都读完了
        if (best == chunk_count) {
            break;
        }

        fputs(current_lines[best], stdout);

        // 从刚刚选中的文件读取下一行
        if (getline(&current_lines[best],
                    &capacities[best],
                    chunks[best]) == -1) {
            free(current_lines[best]);
            current_lines[best] = NULL;
            capacities[best] = 0;
        }
    }

    for (size_t i = 0; i < chunk_count; i++) {
        free(current_lines[i]);
        fclose(chunks[i]);
    }

    free(current_lines);
    free(capacities);
}

int main(int argc, const char **argv) {
    int reverse = 0;

    if (argc > 1 && strcmp(argv[1], "-r") == 0) {
        reverse = 1;
    }

    char *line = NULL;
    size_t capacity = 0;

    // 用来保存所有行
    char **lines = NULL;
    size_t count = 0;

    size_t memory_used = 0;
    const size_t MEMORY_LIMIT = 32 * 1024 * 1024;

    FILE **chunks = NULL;
    size_t chunk_count = 0;

    while (getline(&line, &capacity, stdin) != -1) {
        // 给数组增加一个位置
        char **new_lines = realloc(lines, (count + 1) * sizeof(char *));

        if (new_lines == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }

        lines = new_lines;

        // 复制当前这一行
        lines[count] = strdup(line);

        if (lines[count] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }

        memory_used += strlen(line) + 1 + sizeof(char *);
        count++;

        if (memory_used >= MEMORY_LIMIT) {
            FILE *temp = tmpfile();

            if (temp == NULL) {
                fprintf(stderr, "Could not create temporary file\n");
                return 1;
            }

            sort_and_write(lines, count, reverse, temp);

            FILE **new_chunks =
                realloc(chunks, (chunk_count + 1) * sizeof(FILE *));

            if (new_chunks == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                return 1;
            }

            chunks = new_chunks;
            chunks[chunk_count] = temp;
            chunk_count++;

            for (size_t i = 0; i < count; i++) {
                free(lines[i]);
            }

            free(lines);
            lines = NULL;
            count = 0;
            memory_used = 0;
       }
    }

    if (count > 0) {
        FILE *temp = tmpfile();

        if (temp == NULL) {
            fprintf(stderr, "Could not create temporary file\n");
            return 1;
        }

        sort_and_write(lines, count, reverse, temp);

        FILE **new_chunks =
            realloc(chunks, (chunk_count + 1) * sizeof(FILE *));

        if (new_chunks == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }

        chunks = new_chunks;
        chunks[chunk_count] = temp;
        chunk_count++;
    }

    // 释放每一行
    for (size_t i = 0; i < count; i++) {
        free(lines[i]);
    }

    // 释放保存指针的数组
    free(lines);
    free(line);

    merge_chunks(chunks, chunk_count, reverse);
    free(chunks);

    return 0;
}