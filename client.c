#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "MultModulo.h"

struct Server {
  char ip[255];
  int port;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'}; // TODO: explain why 255

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        // TODO: your code here
		if (k <= 1) {
			printf("k is a positive number ( > 1)\n");
			return 1;
		}
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        // TODO: your code here
		if (mod <= 0) {
			printf("mod is a positive number\n");
			return 1;
		}
        break;
      case 2:
        // TODO: your code here
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  // TODO: for one server here, rewrite with servers from file
  FILE *fp;
  if((fp = fopen(servers, "r")) == NULL) {
      printf("Не удалось открыть файл\n");
      return 1;
  }
  unsigned int servers_num = 0;
  char str[100];
  while(fgets(str, 100, fp)){
     servers_num++;
  }
  //printf("servers_num = %d\n", servers_num);
  fseek(fp, 0L, SEEK_SET);
  
  struct Server *to = malloc(sizeof(struct Server) * servers_num);
  servers_num = 0;
  while(fgets (str, 100, fp) != NULL){
      int i = 0;
      while(str[i] != ':') i++;
      memcpy(to[servers_num].ip, str, sizeof(char)*i);
      char* s = &str[i+1];
      to[servers_num].port = atoi(s);
      printf("%d) %s:%d\n", servers_num+1, to[servers_num].ip, to[servers_num].port);
      servers_num++;
  }
  printf("\n");
  fclose(fp);
  
  // TODO: delete this and parallel work between servers
  //to[0].port = 20001;
  //memcpy(to[0].ip, "127.0.0.1", sizeof("127.0.0.1"));

  // TODO: work continiously, rewrite to make parallel
  uint64_t ans = 1;
  uint64_t step = k / servers_num;
  uint64_t start = 1;
  
  for (int i = 0; i < servers_num; i++) {
    struct hostent *hostname = gethostbyname(to[i].ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(to[i].port);
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      exit(1);
    }

    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
      fprintf(stderr, "Connection failed\n");
      exit(1);
    }

    // TODO: for one server
    // parallel between servers
    uint64_t begin = start;
    uint64_t end = start + step;
    start += step;
    
    if (end > k)
        end = k;
        
    if (begin == end)
        begin = end = 0;
    
    if (begin > k)
        begin = end = 0;

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }

    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Recieve failed\n");
      exit(1);
    }

    // TODO: from one server
    // unite results
    unsigned long long answer = 0;
    memcpy(&answer, response, sizeof(uint64_t));
    printf("intermediate result(%5llu, %5llu): %llu\n", begin, end, answer);
    if (answer > 0)
        ans = MultModulo(ans, answer, mod);

    close(sck);
  }
  printf("answer: %llu\n", ans);
  free(to);

  return 0;
}
