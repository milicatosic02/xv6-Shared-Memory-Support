#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user.h"

int test1(void)
{
	printf("\nstarting test 1\n");
	if(fork())
	{
		int fd = shm_open("/test1"); // so that it stays alive
		wait();
		int size = shm_trunc(fd, 400);
		int *p;
		shm_map(fd, (void **) &p, O_RDWR);
		if(p[0] == 42 && p[1] == 42)
		{
			printf("Test 1 OK (if no other errors appeared)\n");
		}
		else
		{
			printf("Test 1 Not OK\n");
		}
		shm_close(fd);
		return 0;
	}

	if(fork())
	{
		wait();
		int fd = shm_open("/test1");
		int size = shm_trunc(fd, 400);
		int *p;
		shm_map(fd, (void **) &p, O_RDWR);
		p[0] = 42;
		shm_close(fd);
	}
	else
	{
		int fd = shm_open("/test1");
		int size = shm_trunc(fd, 400);
		int *p;
		shm_map(fd, (void **) &p, O_RDWR);
		p[1] = 42;
		shm_close(fd);
	}
	return 1;
}

int test2(void)
{
	if (fork()) {
		wait();
		return 0;
	}
	printf("\nstarting test 2\n");
	int fd = shm_open("/test2");
	int size = shm_trunc(fd, 400);
	int *p;
	shm_map(fd, (void **) &p, O_RDWR);
	p[0] = p[1] = 0;
	if(fork())
	{
		wait();
		if(p[0] == 42 && p[1] == 42)
		{
			printf("Test 2 OK (if no other errors appeared)\n");
		}
		shm_close(fd);
		return 1;
	}

	if(fork())
	{
		p[0] = 42;
		shm_close(fd);
		wait();
	}
	else
	{
		p[1] = 42;
		shm_close(fd);
	}
	return 1;
}

int test3(void)
{
	if (fork()) {
		wait();
		return 0;
	}

	printf("\nstarting test 3\n");
	int fd = shm_open("/test3");
	int pid;
	int size = shm_trunc(fd, 400);
	int *p;
	shm_map(fd, (void **) &p, O_RDONLY);
	close(open("/shm_test3", O_CREATE | O_WRONLY));
	if((pid = fork()))
	{
		wait();
		printf("Test 3 OK if trap 14 was triggered before this by process with pid: %d\n", pid);
		printf("... seems to%s be OK\n",
		       open("/shm_test3", O_RDONLY) >= 0 ? "" : " not");
		shm_close(fd);
		unlink("/shm_test3");
		return 1;
	}

	printf("Triggering trap 14!\n");
	p[1] = 42;
	shm_close(fd); //this doesent get called, cleanup elsewhere on crash
	unlink("/shm_test3");
	return 1;
}

int test4(void)
{
	if (fork()) {
		wait();
		return 0;
	}

	printf("\nstarting test 4\n");
	int fd = shm_open("/test4");
	int pid;
	int size = shm_trunc(fd, 400);
	int *p;
	shm_map(fd, (void **) &p, O_RDWR);
	close(open("/shm_test4", O_CREATE | O_WRONLY));
	if((pid = fork()))
	{
		wait();
		printf("Test 4 OK if trap 14 was triggered before this by process with pid: %d\n", pid);
		printf("... seems to%s be OK\n",
		       open("/shm_test4", O_RDONLY) >= 0 ? "" : " not");
		shm_close(fd);
		unlink("/shm_test4");
		return 1;
	}

	printf("Triggering trap 14!\n");
	shm_close(fd);
	p[1] = 42;
	unlink("/shm_test4");
	return 1;
}

int test5(void)
{
	printf("\nstarting test 5\n");
	printf("stress testing open to find limits\n");
	if (fork()) {
		wait();
		return 0;
	}

	int imax = -1;
	int i;
	for (i = 0; i < 128; i++) {
		char name[] = "hello ";
		name[sizeof(name) - 1] = i;
		int id;
		if ((id = shm_open(name)) < 0)
			break;

		if (id > imax)
			imax = id;
	}
	printf("computed limit: %d\n", i);
	printf("trying to open more by closing..\n");
	for (int i = 0; i <= imax; i++)
		shm_close(i);

	int limit = i;
	int ok = 1;
	for (int i = 0; i < 2 * limit; i++) {
		char name[] = "hello ";
		name[sizeof(name) - 1] = i;
		int id = shm_open(name);
		if (id < 0) {
			printf("despite closing shms, could not open more than the previous limit\n");
			ok = 0;
			break;
		}
		shm_close(id);
	}
	printf("Test 5 %sOK\n", ok ? "" : "not ");
	return 1;
}

int test6(void)
{
	printf("\nstarting test 6\n");
	printf("stress testing trunc and map\n");
	if (fork()) {
		wait();
		printf("test 6 done (if you do not see OK/not OK, then not OK)\n");
		return 0;
	}

	int ok = 1;
	const int step = 4096;
	// if you'd prefer a different size shared memory region, adjust the
	// constant below.
	const unsigned max_sz = 0x40000000 - step - 1;
	void* foo;
	for (unsigned tot_sz = 0; tot_sz < max_sz; tot_sz += step) {
		int id;
		if ((id = shm_open("hello")) < 0) {
			ok = 0;
			break;
		}
		if (shm_trunc(id, step) < 0) {
			printf("failed to trunc despite closing..\n");
			shm_close(id);
			ok = 0;
			break;
		}
		if (shm_map(id, &foo, O_RDWR) < 0) {
			shm_close(id);
			printf("failed to map %x despite closing..\n",
			       foo);
			ok = 0;
			break;
		}
		((char*)foo)[0] = 42;
		shm_close(id);
	}

	printf("Test 6 %sOK\n", ok ? "" : "not ");
	return 1;
}


int
main(int argc, char *argv[])
{
	printf("note that errors could happen as a result of prior errors\n"
	       "as a result, you should inspect errors in sequence\n");
	if(test1()) goto ex;
	if(test2()) goto ex;
	if(test3()) goto ex;
	if(test4()) goto ex;
	if(test5()) goto ex;
	if(test6()) goto ex;

ex:
	exit();
}
