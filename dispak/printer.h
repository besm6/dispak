/*
 * Пример реализации АЦПУ в виде отдельного потока (pthread).
 */

/* Тип printer_t используется вызывающим уровнем.
 * Собственно структура определяется в файле printer.c и не видна снаружи. */
typedef struct _printer_t printer_t;

void printer_init (printer_t *unit);
void printer_write_reg (printer_t *unit, unsigned addr, uint64_t val);
uint64_t printer_read_reg (printer_t *unit, unsigned addr);
