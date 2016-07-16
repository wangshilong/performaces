/*
 *
 * Benchmark Filesystem Metadata peformaces.
 * Written by Wang Shilong <wangshilong1991@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>

struct my_thread_arg {
	char *dir;
	int nums;
};

int create_files(const char *dir, int nums)
{
	char tmp[256];
	int rc;
	int i;
	int fd;

	for (i = 0; i < nums; i++) {
		sprintf(tmp, "%s/%d", dir, i);
		fd = open(tmp, O_CREAT);
		if (fd < 0) {
			fprintf(stderr, "failed to create file: %s,%s\n", tmp, strerror(errno));
			return rc;
		}
		close(fd);
	}
	return 0;
}


/* Reserved for future useage */
/*
int remove_files(const char *dir, int nums)
{
	char tmp[256];
	int rc;
	int i;

	for (i = 0; i < nums; i++) {
		sprintf(tmp, "%s/%d", dir, i);
		rc = unlink(tmp);
		if (rc < 0) {
			fprintf(stderr, "failed to remove file: %s", tmp);
			return rc;
		}
	}
	return 0;
}
*/

void *thread_worker(void *arg)
{
	struct my_thread_arg *arg1 = (struct my_thread_arg*)arg;
	create_files(arg1->dir, arg1->nums);

	return NULL;
}

int main(int argc, char **argv)
{
	int i = 0;
	char tmp[256];
	pthread_t id;
	pthread_t *id_array;
	int ret;
	struct my_thread_arg *arg;
	struct timeval time_start;
	struct timeval time_end;
	unsigned long timer;
	int number_of_threads = 1;
	int number_per_threads = 100000;
	int result;
	double per_create = 0;
	int is_btrfs = 0;

	opterr = 0;
	while ((result = getopt(argc, argv, "t:n:b")) != -1 )
	{
		switch(result)
		{
		case 't':
			number_of_threads = atoi(optarg);
			break;
		case 'n':
			number_per_threads = atoi(optarg);
			break;
		case 'b':
			is_btrfs = 1;
			break;
		default:
			break;
		}
	}
	id_array = malloc(sizeof(pthread_t) * number_of_threads);
	if (!id_array)
		return -1;

	memset(tmp, 0, 256);
	for (i = 0; i < number_of_threads; i++) {
		if (is_btrfs) {
			sprintf(tmp, "btrfs sub create subvolume%d", i);
			system(tmp);
		} else {
			sprintf(tmp, "subvolume%d", i);
			mkdir(tmp, 777);
		}
	}

	//start from here
	gettimeofday(&time_start, NULL);

	for (i = 0; i < number_of_threads; i++) {
		arg = malloc(sizeof(struct my_thread_arg));
		if (!arg)
			return -1;
		sprintf(tmp, "subvolume%d", i);
		arg->dir = strdup(tmp);
		arg->nums = number_per_threads;

		ret = pthread_create(&id_array[i], NULL, thread_worker, arg);
		if (ret)
			return ret;
	}
	for (i = 0; i < number_of_threads; i++)
		pthread_join(id_array[i], NULL);

	//end here
	gettimeofday(&time_end, NULL);
	timer = 1000000 * (time_end.tv_sec-time_start.tv_sec) + time_end.tv_usec - time_start.tv_usec;
	fprintf(stdout, "Total creates: %lu files, Total time: %lu usec\n",
			number_per_threads * number_of_threads, timer);

	per_create = timer / 1000000.0;
	per_create = number_per_threads * number_of_threads / per_create;
	fprintf(stdout, "file creates per seconds: %f\n", per_create);
	return 0;
}

