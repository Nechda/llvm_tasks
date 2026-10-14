/*
 * Рантайм-поддержка трассировки использования IR инструкций.
 * Вызывается из кода, сгенерированного TracePass.cpp.
 *
 * Пишет бинарную трассу пар (uint32 user_id, uint32 operand_id).
 *
 * Переменные окружения:
 *   LLVM_TRACE_FILE   -- файл трассы       (по умолчанию "trace.bin")
 *   LLVM_TRACE_MAX    -- максимум пар; 0 = не ограничивать (по умолчанию 0)
 *   LLVM_TRACE_EXIT   -- "1": по достижении MAX записи завершить программу
 *                        через _exit(0) (трасса досрочная, но полная до
 *                        момента останова); по умолчанию "0"
 *   LLVM_TRACE_LOG    -- "1": дублировать трассу в текстовом виде в stderr
 *
 * Формат файла:
 *   magic:   "TRC1"
 *   uint64   num_pairs (обновляется при завершении)
 *   num_pairs * (uint32 user_id, uint32 operand_id)
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_WORDS (1u << 20) /* 4 MiB буфер в uint32 */

static FILE *g_file = NULL;
static uint32_t *g_buf = NULL;
static uint64_t g_pairs = 0;
static uint64_t g_max = 0;
static int g_exit_on_max = 0;
static int g_log_stderr = 0;
static int g_enabled = 1;
static int g_inited = 0;

__attribute__((constructor)) static void trace_init(void) {
    const char *env;

    if ((env = getenv("LLVM_TRACE_FILE")) != NULL)
        g_file = fopen(env, "wb");
    else
        g_file = fopen("trace.bin", "wb");
    if (!g_file) {
        fprintf(stderr, "[trace_rt] cannot open trace file, tracing off\n");
        g_enabled = 0;
        return;
    }

    g_buf = (uint32_t *)malloc(BUF_WORDS * sizeof(uint32_t));
    if (!g_buf) {
        fprintf(stderr, "[trace_rt] out of memory, tracing off\n");
        g_enabled = 0;
        return;
    }

    if ((env = getenv("LLVM_TRACE_MAX")) != NULL)
        g_max = strtoull(env, NULL, 0);
    if ((env = getenv("LLVM_TRACE_EXIT")) != NULL)
        g_exit_on_max = *env == '1';
    if ((env = getenv("LLVM_TRACE_LOG")) != NULL)
        g_log_stderr = *env == '1';

    /* Заголовок: magic + счётчик пар (обновим его в trace_fini). */
    fputs("TRC1", g_file);
    if (fwrite(&g_pairs, sizeof(uint64_t), 1, g_file) != 1) {
        fprintf(stderr, "[trace_rt] header write failed\n");
        g_enabled = 0;
        return;
    }

    g_inited = 1;
    fprintf(stderr,
            "[trace_rt] tracing to '%s' (max_pairs=%llu exit_on_max=%d)\n",
            getenv("LLVM_TRACE_FILE") ? getenv("LLVM_TRACE_FILE") : "trace.bin",
            (unsigned long long)g_max, g_exit_on_max);
}

__attribute__((destructor)) static void trace_fini(void) {
    if (!g_inited)
        return;
    size_t words = (size_t)((g_pairs * 2) % BUF_WORDS);
    if (words && fwrite(g_buf, sizeof(uint32_t), words, g_file) != words)
        fprintf(stderr, "[trace_rt] final write failed\n");
    if (fseek(g_file, 4, SEEK_SET) == 0) {
        if (fwrite(&g_pairs, sizeof(uint64_t), 1, g_file) != 1)
            fprintf(stderr, "[trace_rt] header update failed\n");
    }
    fclose(g_file);
    g_file = NULL;
    g_inited = 0;
    fprintf(stderr, "[trace_rt] done: %llu pairs written\n",
            (unsigned long long)g_pairs);
}

static void trace_drop_tail_and_close(int do_exit) {
    size_t words = (size_t)((g_pairs * 2) % BUF_WORDS);
    if (words && fwrite(g_buf, sizeof(uint32_t), words, g_file) != words)
        fprintf(stderr, "[trace_rt] tail write failed\n");
    if (fseek(g_file, 4, SEEK_SET) == 0)
        fwrite(&g_pairs, sizeof(uint64_t), 1, g_file);
    fclose(g_file);
    g_file = NULL;
    g_inited = 0;
    fprintf(stderr, "[trace_rt] done: %llu pairs written\n",
            (unsigned long long)g_pairs);
    if (do_exit)
        _exit(0);
}

/*
 * Основная функция, вызываемая из инструментированного кода.
 * Вызывается крайне часто -- держим её максимально дешёвой.
 */
void __trace_pair(uint32_t user, uint32_t operand) {
    if (!g_inited)
        return;
    if (g_max && g_pairs >= g_max) {
        if (g_exit_on_max) {
            /* Досрочное завершение по квоте: сбросить хвост и заголовок. */
            trace_drop_tail_and_close(1);
        }
        return;
    }
    uint32_t *p = g_buf + ((g_pairs * 2) % BUF_WORDS);
    p[0] = user;
    p[1] = operand;
    g_pairs++;
    if ((g_pairs * 2) % BUF_WORDS == 0 &&
        fwrite(g_buf, sizeof(uint32_t), BUF_WORDS, g_file) != BUF_WORDS) {
        fprintf(stderr, "[trace_rt] write failed, tracing off\n");
        g_enabled = 0;
        g_inited = 0;
        fclose(g_file);
        g_file = NULL;
        return;
    }
    if (g_log_stderr)
        fprintf(stderr, "%u <- %u\n", user, operand);
}
