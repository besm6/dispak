/*
 * Пример реализации АЦПУ в виде отдельного потока (pthread).
 */
#include <stdlib.h>
#include <pthread.h>
#include "defs.h"
#include "printer.h"

struct _printer_t {
	/* Отдельный поток, реализующий поведение устройства. */
	pthread_t thread;

	/* Семафор для блокировки доступа к данным из разных потоков. */
	pthread_mutex_t mutex;

	/* Переменная состояния для передачи сигналов потоку устройства. */
	pthread_cond_t mailbox;

	/* Регистры устройства, для примера. */
	uint64_t reg [2];
};

/*
 * Обслуживание устройства.
 * Выполняется в отдельном потоке.
 */
void *printer (void *arg)
{
	printer_t *unit = arg;

	pthread_mutex_lock (&unit->mutex);

	/* TODO: выполняем инициализацию устройства,
	 * задаём начальные значения регистров и т.п. */

	for (;;) {
		/* Ждём сигнала. */
		pthread_cond_wait (&unit->mailbox, &unit->mutex);

		/* TODO: анализируем изменения в регистрах и выполняем
		 * соответствующие действия. */
	}
}

void printer_init (printer_t *unit)
{
	pthread_mutex_init (&unit->mutex, 0);
	pthread_cond_init (&unit->mailbox, 0);

	/* Старт потока устройства. */
	if (0 != pthread_create (&unit->thread, 0, printer, unit)) {
		perror ("pthread_start");
		exit (1);
	}

	/* Даём возможность новому потоку запуститься и
	 * повиснуть на ожидании сигнала. */
	sched_yield ();
}

/*
 * Пишем регистры.
 * После каждой записи поток устройства производит какие-то действия.
 */
void printer_write_reg (printer_t *unit, unsigned addr, uint64_t val)
{
	pthread_mutex_lock (&unit->mutex);

	/* Меняем регистры. */
	unit->reg [addr] = val;

	/* Сообщаем устройству об изменениях. */
	pthread_cond_signal (&unit->mailbox);
	pthread_mutex_unlock (&unit->mutex);
}

/*
 * Читаем регистры.
 * Устройство этого "не замечает".
 */
uint64_t printer_read_reg (printer_t *unit, unsigned addr)
{
	uint64_t val;

	pthread_mutex_lock (&unit->mutex);
	val = unit->reg [addr];
	pthread_mutex_unlock (&unit->mutex);
	return val;
}
