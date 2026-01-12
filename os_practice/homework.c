#include "sys/wait.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// int main(int argc, char *argv[]) {
//   int x = 9;

//   printf("The variable is %d \n", x);
//   int rc = fork();

//   if (rc == 0) {
//     int x = 12;
//     printf("The variable is %d \n", x);
//   } else {
//     int wc = wait(NULL);
//     int x = 10;
//     printf("The variable is %d \n", x);
//   }
// }

// int main(int argc, char *argv[]) {
//   close(STDOUT_FILENO);
//   open("./hw.output", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

//   int rc = fork();

//   if (rc == 0) {
//     printf("Hello This is the child process Writing!\n");
//     exit(0);
//   } else {
//     printf("Hello this is parent process writing!\n");
//     int wc = waitpid(rc, NULL, 0);
//     printf("Child process exited with status %d\n", wc);
//   }
// }
//

// int main(int argc, char *argv[]) {
//   int rc = fork();

//   if (rc < 0) {
//     fprintf(stderr, "fork failed\n");
//     exit(1);
//   } else if (rc == 0) {
//     close(STDOUT_FILENO);
//     printf("I am trying to print something after closing STDOUT_FILENO\n");
//     exit(0);
//   } else {
//     int wc = waitpid(rc, NULL, 0);
//     printf("This is the parent trying to print something");
//   }
// }
