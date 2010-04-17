.PHONY: all clean

OUTPUT_DIR := obj

APP_HEADERS := include/app_main.h include/app_util.h
APP_SOURCES := app_main.c app_util.c
APP_OBJS := ${OUTPUT_DIR}/app_main.o ${OUTPUT_DIR}/app_util.o
APP_DIR := app

THREADSLIB_HEADERS := include/queue.h include/thread.h include/scheduler.h
THREADSLIB_SOURCES := queue.c thread.c scheduler.c fifo_scheduler.c prio_scheduler.c
THREADSLIB_OBJS := ${OUTPUT_DIR}/queue.o ${OUTPUT_DIR}/thread.o ${OUTPUT_DIR}/scheduler.o ${OUTPUT_DIR}/fifo_scheduler.o ${OUTPUT_DIR}/prio_scheduler.o
THREADSLIB_DIR := threadslib

LDFLAGS := 
CFLAGS := -O0 -g3 -Wall -Wextra -c

INCLUDE := .



all: app threadslib ${OUTPUT_DIR}/main.o
	gcc ${LDFLAGS} ${APP_OBJS} ${THREADSLIB_OBJS} ${OUTPUT_DIR}/main.o -o assignment1.exe

${OUTPUT_DIR}/main.o: main.c ${APP_HEADERS} makedir
	gcc ${CFLAGS} -I${INCLUDE} main.c -o ${OUTPUT_DIR}/main.o

threadslib: ${THREADSLIB_OBJS}

${OUTPUT_DIR}/queue.o: ${THREADSLIB_DIR}/queue.c ${THREADSLIB_HEADERS} makedir
	gcc ${CFLAGS} -I${INCLUDE} ${THREADSLIB_DIR}/queue.c -o ${OUTPUT_DIR}/queue.o

${OUTPUT_DIR}/thread.o: ${THREADSLIB_DIR}/thread.c ${THREADSLIB_HEADERS} makedir
	gcc ${CFLAGS} -I${INCLUDE} ${THREADSLIB_DIR}/thread.c -o ${OUTPUT_DIR}/thread.o

${OUTPUT_DIR}/scheduler.o: ${THREADSLIB_DIR}/scheduler.c ${THREADSLIB_HEADERS} makedir
	gcc ${CFLAGS} -I${INCLUDE} ${THREADSLIB_DIR}/scheduler.c -o ${OUTPUT_DIR}/scheduler.o

${OUTPUT_DIR}/fifo_scheduler.o: ${THREADSLIB_DIR}/fifo_scheduler.c ${THREADSLIB_HEADERS} makedir
	gcc ${CFLAGS} -I${INCLUDE} ${THREADSLIB_DIR}/fifo_scheduler.c -o ${OUTPUT_DIR}/fifo_scheduler.o

${OUTPUT_DIR}/prio_scheduler.o: ${THREADSLIB_DIR}/prio_scheduler.c ${THREADSLIB_HEADERS} makedir
	gcc ${CFLAGS} -I${INCLUDE} ${THREADSLIB_DIR}/prio_scheduler.c -o ${OUTPUT_DIR}/prio_scheduler.o

app: ${APP_OBJS}

${OUTPUT_DIR}/app_main.o: ${APP_DIR}/app_main.c ${THREADSLIB_HEADERS} ${APP_HEADERS} makedir
	gcc ${CFLAGS} -I${INCLUDE} ${APP_DIR}/app_main.c -o ${OUTPUT_DIR}/app_main.o

${OUTPUT_DIR}/app_util.o: ${APP_DIR}/app_util.c ${THREADSLIB_HEADERS} ${APP_HEADERS} makedir
	gcc ${CFLAGS} -I${INCLUDE} ${APP_DIR}/app_util.c -o ${OUTPUT_DIR}/app_util.o

makedir: 
	[ -d ${OUTPUT_DIR} ] || mkdir -p ${OUTPUT_DIR}