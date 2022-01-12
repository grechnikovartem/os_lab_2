#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
	char fn1[256];
	char fn2[256];
	if (scanf("%s", fn1) <= 0) {
		perror("scanf error");
		return -1;
	}
	if (scanf("%s", fn2) <= 0) {
		perror("scanf error");
		return -1;
	}
	int file1 = open(fn1, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
	if (file1 < 0) {
		perror("open error");
		return -1;
	}
	int file2 = open(fn2, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
	if (file2 < 0) {
		perror("open error");
		return -1;
	}

	int fd1[2], fd2[2];
	if (pipe(fd1) != 0) {
		perror("pipe error");
		return -1;
	}
	if (pipe(fd2) != 0) {
		perror("pipe error");
		return -1;
	}

	int id1 = fork();

	if (id1 == -1)	{
		perror("fork error");
		return -1;
		
	} else if (id1 == 0) {
		close(file2);
		close(fd2[0]);
		close(fd2[1]);
		close(fd1[1]);
		if (dup2(fd1[0], fileno(stdin)) == -1) {
			perror("dup2 error");
			return -1;
		}
		if (dup2(file1, fileno(stdout)) == -1) {
			perror("dup2 error");
			return -1;
		}
		close(file1);

		char * const * null = NULL;
		if (execv("child", null) == -1) {
			perror("execv error");
			return -1;
		}

	} else {
		int id2 = fork();

		if (id2 == -1)	{
			perror("fork error");
			return -1;

		} else if (id2 == 0) {
			close(file1);
			close(fd1[0]);
			close(fd1[1]);
			close(fd2[1]);
			if (dup2(fd2[0], fileno(stdin)) == -1) {
				perror("dup2 error");
				return -1;
			}
			if (dup2(file2, fileno(stdout)) == -1) {
				perror("dup2 error");
				return -1;
			}
			close(file2);

			char * const * null = NULL;
			if (execv("child", null) == -1) {
				perror("execv error");
				return -1;
			}

		} else {
			close(fd1[0]);
			close(fd2[0]);
			close(file1);
			close(file2);

			char c;
			char str[10];
			str[0] = '\0';
			int n = 0;
			while (scanf("%c", &c) > 0) {
				if (c != '\n') {
					if (n < 10) {
						str[n] = c;
					} else if (str[0] != '\0') {
						for (int i = 0; i < 10; ++i) {
							write(fd2[1], &str[i], sizeof(char));
							str[i] = '\0';
						}
						write(fd2[1], &c, sizeof(char));
					} else {
						write(fd2[1], &c, sizeof(char));
					}
					++n;
				} else {
					if (str[0] != '\0') {
						for (int i = 0; i < n; ++i) {
							write(fd1[1], &str[i], sizeof(char));
							str[i] = '\0';
						}
						write(fd1[1], &c, sizeof(char));
					} else {
						write(fd2[1], &c, sizeof(char));
					}
					n = 0;
				}
			}

			close(fd1[1]);
			close(fd2[1]);
		}
	}
	return 0;
}
