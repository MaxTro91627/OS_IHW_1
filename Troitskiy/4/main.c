#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const int size = 5001;

int isVowel(char c) {
  if (c == 'a' || c == 'A' || c == 'e' || c == 'E' || c == 'i' || c == 'I' ||
      c == 'o' || c == 'O' || c == 'u' || c == 'U' || c == 'y' || c == 'Y') {
    return 1;
  } else {
    return 0;
  }
}

char toHex(int num) {
  if (num < 10) {
    return '0' + num;
  } else {
    return 'A' + num - 10;
  }
}

int main(int argc, char **argv) {
  char reading_file, writing_file;
  int mem[2], str[2];
  pid_t reading, counting, output;
  size_t reading_bytes = size;
  int capacity = size;
  char sharing_arr[size + 1];
  char *res_string = (char *)malloc(capacity * sizeof(char));
  int *waiting_time = 1;

  pipe(mem);
  pipe(str);
  int val;

  reading = fork();

  if (reading == 0) {
    close(mem[0]);
    if ((reading_file = open(argv[1], O_RDONLY)) < 0) {
      printf("Cant open reading file");
      exit(-1);
    }
    do {
      reading_bytes = read(reading_file, sharing_arr, size);
      if (reading_bytes == -1) {
        printf("Cant read this file");
        exit(-1);
      }
      sharing_arr[reading_bytes] = '\0';
    } while (reading_bytes == size);

    write(mem[1], sharing_arr, size);

    if (close(reading_file) < 0) {
      printf("Cant close reading file");
    }
    exit(0);
  } else if (reading > 0) {
    waitpid(reading, NULL, 0);
    counting = fork();
    if (counting == 0) {
      close(mem[1]);
      int nbytes = read(mem[0], sharing_arr, sizeof(sharing_arr));
      capacity = strlen(sharing_arr);
      res_string = (char *)realloc(res_string, capacity * sizeof(char));

      int ind = 0;
      for (int i = 0; i < strlen(sharing_arr); ++i) {
        if (capacity - ind < 5) {
          capacity += 4;
          res_string = (char *)realloc(res_string, capacity * sizeof(char));
        }
        char c = sharing_arr[i];
        if (isVowel(c)) {
          int number = (int)c;
          res_string[ind] = '0';
          res_string[ind + 1] = 'x';
          res_string[ind + 2] = toHex(number / 16);
          res_string[ind + 3] = toHex(number % 16);
          ind += 4;
        } else {
          res_string[ind] = sharing_arr[i];
          ind++;
        }
      }

      close(str[0]);
      char buf[5000];

      for (int i = 0; i < strlen(res_string); ++i) {
        int n = sprintf(buf, "%c", res_string[i]);
        val = write(str[1], buf, n);
      }
      free(res_string);
      exit(0);
    } else if (counting > 0) {
      waitpid(counting, NULL, 0);
      output = fork();

      if (output == 0) {

        close(str[1]);
        int nbytes = read(str[0], sharing_arr, sizeof(sharing_arr));
        if ((writing_file = open(argv[2], O_WRONLY | O_CREAT, 0666)) < 0) {
          printf("Cant open writing file");
          exit(-1);
        }

        val = write(writing_file, sharing_arr, strlen(sharing_arr));

        if (close(writing_file) < 0) {
          printf("Cant close writing file");
        }
        exit(0);
      } else if (output > 0) {
        waitpid(output, NULL, 0);
        exit(0);
      } else {
        printf("Cant create child of output process");
        exit(-1);
      }
    } else {
      printf("Cant create child of counting process");
      exit(-1);
    }
  } else {
    printf("Cant creat child of reading process");
    exit(-1);
  }
}