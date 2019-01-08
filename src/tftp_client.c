/**
 * @file
 * @author Riccardo Mancini
 * @brief Implementation of the TFTP client that can only make read requests.
 */

/** Defines log level to this file. */
#define LOG_LEVEL LOG_INFO


#include "include/logging.h"
#include "include/tftp_msgs.h"
#include "include/tftp.h"
#include "include/fblock.h"
#include "include/inet_utils.h"
#include "include/debug_utils.h"
#include "include/netascii.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/** max stdin line length */
#define READ_BUFFER_SIZE 80

/** Maximum number of arguments for commands */
#define MAX_ARGS 3

/** String for txt */
#define MODE_TXT "txt"

/** String for bin*/
#define MODE_BIN "bin"


/** 
 * Global transfer_mode variable for storing user chosen transfer mode string.
 * 
 * @see MODE_TXT
 * @see MODE_BIN
 */
char* transfer_mode;


/**
 * Splits a string at each delim.
 * 
 * Trailing LF will be removed. Consecutive delimiters will be considered as one.
 * 
 * @param line [in]     the string to split
 * @param delim [in]    the delimiter
 * @param max_argc [in] maximum number of parts to split the line into
 * @param argc [out]    counts of the parts the line is split into
 * @param argv [out]    array of parts the line is split into
 */
void split_string(char* line, char* delim, int max_argc, int *argc, char **argv){
  char *ptr;
  int len;/**
 * Prints command usage information.
 */
  char *pos;

  // remove trailing LF
  if ((pos=strchr(line, '\n')) != NULL)
    *pos = '\0';

  // init argc
  *argc = 0;

  // tokenize string 
  ptr = strtok(line, delim);

	while(ptr != NULL && *argc <= max_argc){
    len = strlen(ptr);

    if (len == 0)
      continue;

    LOG(LOG_DEBUG, "arg[%d] = '%s'", *argc, ptr);

    argv[*argc] = malloc(strlen(ptr)+1);
    strcpy(argv[*argc], ptr);
		
		ptr = strtok(NULL, delim);
    (*argc)++;
	}
}

/**
 * Prints command usage information.
 */
void print_help(){
  printf("Usage: ./tftp_client SERVER_IP SERVER_PORT\n");
  printf("Example: ./tftp_client 127.0.0.1 69");
}

/**
 * Handles !help command, printing information about available commands.
 */
void cmd_help(){
  printf("Sono disponibili i seguenti comandi:\n");
  printf("!help --> mostra l'elenco dei comandi disponibili\n");
  printf("!mode {txt|bin} --> imposta il modo di trasferimento dei file (testo o binario)\n");
  printf("!get filename nome_locale --> richiede al server il nome del file <filename> e lo salva localmente con il nome <nome_locale>\n");
  printf("!quit --> termina il client\n");
}

/**
 * Handles !mode command, changing transfer_mode to either bin or text.
 * 
 * @see transfer_mode
 */
void cmd_mode(char* new_mode){
  if (strcmp(new_mode, MODE_TXT) == 0){
    transfer_mode = TFTP_STR_NETASCII;
    printf("Modo di trasferimento testo configurato\n");
  } else if (strcmp(new_mode, MODE_BIN) == 0){
    transfer_mode = TFTP_STR_OCTET;
    printf("Modo di trasferimento binario configurato\n");
  } else{
    printf("Modo di traferimento sconosciuto: %s. Modi disponibili: txt, bin\n", new_mode);
  }
}

/**
 * Handles !get command, reading file from server.
 */
int cmd_get(char* remote_filename, char* local_filename, char* sv_ip, int sv_port){
  struct sockaddr_in my_addr, sv_addr;
  int sd;
  int ret, tid, result;
  struct fblock m_fblock;
  char *tmp_filename;

  LOG(LOG_INFO, "Initializing...\n");

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (strcmp(transfer_mode, TFTP_STR_OCTET) == 0)
    m_fblock = fblock_open(local_filename, TFTP_DATA_BLOCK, FBLOCK_WRITE|FBLOCK_MODE_BINARY);
  else if (strcmp(transfer_mode, TFTP_STR_NETASCII) == 0){
    tmp_filename = malloc(strlen(local_filename)+5);
    strcpy(tmp_filename, local_filename);
    strcat(tmp_filename, ".tmp");
    m_fblock = fblock_open(tmp_filename, TFTP_DATA_BLOCK, FBLOCK_WRITE|FBLOCK_MODE_TEXT);
  }else
    return 2;

  LOG(LOG_INFO, "Opening socket...");

  sv_addr = make_sv_sockaddr_in(sv_ip, sv_port);
  my_addr = make_my_sockaddr_in(0);
  tid = bind_random_port(sd, &my_addr);
  if (tid == 0){
    LOG(LOG_ERR, "Error while binding to random port");
    perror("Could not bind to random port:");
    fblock_close(&m_fblock);
    return 1;
  } else
    LOG(LOG_INFO, "Bound to port %d", tid);

  printf("Richiesta file %s (%s) al server in corso.\n", remote_filename, transfer_mode);

  ret = tftp_send_rrq(remote_filename, transfer_mode, sd, &sv_addr);
  if (ret != 0){
    fblock_close(&m_fblock);
    return 8+ret;
  }

  printf("Trasferimento file in corso.\n");

  ret = tftp_receive_file(&m_fblock, sd, &sv_addr);

  
  if (ret == 1){    // File not found
    printf("File non trovato.\n");
    result = 0;
  } else if (ret != 0){
    LOG(LOG_ERR, "Error while receiving file!");
    result = 16+ret;
  } else{
    int n_blocks = m_fblock.written/m_fblock.block_size + 1;
    printf("Trasferimento completato (%d/%d blocchi)\n", n_blocks, n_blocks);
    printf("Salvataggio %s completato.\n", local_filename);

    result = 0;
  }

  fblock_close(&m_fblock);
  if (strcmp(transfer_mode, TFTP_STR_NETASCII) == 0){
    netascii2unix(tmp_filename, local_filename);
    remove(tmp_filename);
    free(tmp_filename);
  }

  return result;

}

/**
 * Handles !quit command.
 */
void cmd_quit(){
  printf("Client terminato con successo\n");
  exit(0);
}

/** Main */
int main(int argc, char** argv){
  char* sv_ip;
  short int sv_port;
  int ret;
  char read_buffer[READ_BUFFER_SIZE];
  int cmd_argc;
  char *cmd_argv[MAX_ARGS];

  //init random seed
  srand(time(NULL));

  // default mode = bin
  transfer_mode = TFTP_STR_OCTET;

  if (argc != 3){
    print_help();
    return 1;
  }

  // TODO: check args
  sv_ip = argv[1];
  sv_port = atoi(argv[2]);

  while(1){
    printf("> ");
    fflush(stdout); // flush stdout buffer
    fgets(read_buffer, READ_BUFFER_SIZE, stdin);
    split_string(read_buffer, " ", MAX_ARGS, &cmd_argc, cmd_argv);

    if (cmd_argc == 0){
      printf("Comando non riconosciuto : ''\n");
      cmd_help();
    } else{
      if (strcmp(cmd_argv[0], "!mode") == 0){
        if (cmd_argc == 2)
          cmd_mode(cmd_argv[1]);
        else
          printf("Il comando richiede un solo argomento: bin o txt\n");
      } else if (strcmp(cmd_argv[0], "!get") == 0){
        if (cmd_argc == 3){
          ret = cmd_get(cmd_argv[1], cmd_argv[2], sv_ip, sv_port);
          LOG(LOG_DEBUG, "cmd_get returned value: %d", ret);
        } else{
           printf("Il comando richiede due argomenti: <filename> e <nome_locale>\n");
        }
      } else if (strcmp(cmd_argv[0], "!quit") == 0){
        if (cmd_argc == 1){
          cmd_quit();
        } else{
           printf("Il comando non richiede argomenti\n");
        }
      } else if (strcmp(cmd_argv[0], "!help") == 0){
        if (cmd_argc == 1){
          cmd_help();
        } else{
           printf("Il comando non richiede argomenti\n");
        }
      } else {
        printf("Comando non riconosciuto : '%s'\n", cmd_argv[0]);
        cmd_help();
      } 
    }

    // Free malloc'ed strings
    for (int i = 0; i < cmd_argc; i++)
      free(cmd_argv[i]); 
  }

  return 0;
}
