#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {
    char error_msg[1024];
    size_t offset = -1;
    int fd = -1;
    char end;

    if (argc != 2) {
        printf("Usage: append file_name\n");
        return 0;
    }

    fd = open(argv[1], O_RDWR|O_APPEND|O_CREAT);
    if (-1 == fd) {
        sprintf(error_msg, "Open file %s:", argv[1]);
        perror(error_msg);
        return 1;
    }

    offset = lseek(fd, 0, SEEK_END);
    printf("The length of file = %lu\n", offset);

    end = 0;
    while (offset < 510) {
        if (write(fd, &end, 1) != 1) {
            perror("Write file meets error");
            return 1;
        }
        ++offset;
    }

    end = 0x55;
    if (write(fd, &end, 1) != 1) {
        perror("Write file end 0x55 meets error!\n");
        return 1;
    }

    end = 0xAA;
    if (write(fd, &end, 1) != 1) {
        perror("Write file end 0xAA meets error!\n");
        return 1;
    }

    close(fd);
    return 0;
}
