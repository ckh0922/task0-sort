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

        count++;
    }

    if (reverse) {
        qsort(lines, count, sizeof(char *), compare_lines_reverse);
    } else {
        qsort(lines, count, sizeof(char *), compare_lines);
    }

    // 看看我们保存下来的内容
    for (size_t i = 0; i < count; i++) {
        printf("%s", lines[i]);
    }

    // 释放每一行
    for (size_t i = 0; i < count; i++) {
        free(lines[i]);
    }

    // 释放保存指针的数组
    free(lines);
    free(line);

    return 0;
}