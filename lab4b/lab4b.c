//NAME: Dawei Huang
//EMAIL: daweihuang@g.ucla.edu
//ID: 304792166

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

// temperature variables
const int B = 4275; // value of thermistor
char temp_scale = 'F';
FILE *log_file = NULL;

//time variables
time_t local_timezone, begin, end;
char time_str[16];
struct tm* local_time_info;
int time_period = 1;

//program commands
char command[64];
bool generate_reports = true;

//process program's arguments
void process_args(int argc, char** argv) {
	
  int opt;
  const char* SHORT_OPTS = "p:s:l";
  static struct option LONG_OPTS[] = {
    {"period", required_argument, 0, 'p'},
    {"scale", required_argument, 0, 's'},
    {"log", required_argument, 0, 'l'},
    {NULL, 0, NULL, 0}
  };

  while ((opt = getopt_long(argc, argv, SHORT_OPTS, LONG_OPTS, NULL)) != -1) {		
    if (opt == 'p') {
      time_period = atoi(optarg);
    }
    else if (opt == 's') {
      if (optarg[0] == 'C' || optarg[0] == 'F')
	temp_scale = optarg[0];
      else {
	perror("You must do measure the temperature in Celsius or Fahrenheits");
	exit(1);
      }
    }
    else if (opt == 'l') {
      log_file = fopen(optarg, "w");
    }
    else {
      fprintf(stderr, "304-792-166: unrecognized argument: %c \n", opt);
      exit(1);
    }
  }
}

//get local time and converts them into strings
void get_local_time(time_t *l_tz, struct tm* l_t_i, char * t_str) {
  time(l_tz);
  l_t_i = localtime(l_tz);
  strftime(t_str, 10, "%H:%M:%S", l_t_i);
}

//print out the time_string and saves the log file
void print_local_time (double degrees) {
  fprintf(stdout, "%s %.1f\n", time_str, degrees);
  if (log_file != NULL) {
    fprintf(log_file, "%s %.1f\n", time_str, degrees);
    fflush(log_file);
  }
}

//prints out shut down and log off
void shut_down(FILE *log_file) {
	
  //fprintf(stdout, "%s SHUTDOWN\n", time_str);
  if (log_file != NULL) {
    fprintf(log_file, "%s SHUTDOWN\n", time_str);
    fflush(log_file);
  }
	
  exit(0);
}

//prints out the commands into the log files
void print_command(const char* command) {
	
  //fprintf(stdout, "%s\n", command);
  if (log_file != NULL) {
    fprintf(log_file, "%s\n", command);
    fflush(log_file);
  }
}

double convert_volt_to_temp(int volt_temp) {
  double temp = 1023.0 / ((double)volt_temp) - 1.0;
  temp *= 100000.0;
  double celsius = 1.0 / (log(temp/100000.0) / B + 1/298.15) - 273.15;

  if (temp_scale == 'C')
    return celsius;
  return celsius * 9/5 + 32;
}

//attempt to find the period value in the commands
bool parse_period() {
  //int i = 0;
  int len = strlen(command);
  int prefix_length = strlen("PERIOD=");
  if (prefix_length < len) {
    char *prefix = (char*) malloc(prefix_length + 1);
    strncpy(prefix, command, prefix_length);
    prefix[prefix_length] = '\0';

    if(strcmp(prefix, "PERIOD=") == 0) {
      
      time_period = atoi(command + len - 1);
      
      return true;
    }
    else {
      return false;
    }
    
  }

  return false;
}

void process_commands() {
	
  if (strcmp(command, "OFF") == 0) {
    print_command("OFF");
    shut_down(log_file);
  }
  else if (strcmp(command, "STOP") == 0) {
    generate_reports = false;
    print_command(command);
  }
  else if (strcmp(command, "START") == 0) {
    generate_reports = true;
    print_command(command);
  }
  else if (strcmp(command, "SCALE=F") == 0) {
    temp_scale='F';
    print_command(command);
  }
  else if (strcmp(command, "SCALE=C") == 0) {
    temp_scale='C';
    print_command(command);
  }
  else {
    fprintf(stderr, "%s\n", command);
    if (strlen(command) < 8) {
      print_command("Invalid commands");
    }
    else {
      if (parse_period() == false) {
	//fprintf(stderr, "period: %s\n", command); 
	print_command("Invalid commands"); 
      }
      else {
	print_command(command);
      }
    }
  }
}

int main(int argc, char **argv) {
  process_args(argc, argv);
	
  // initialized temperature sensor at Analog Input 0
  mraa_aio_context temp_sensor;
  temp_sensor = mraa_aio_init(1);
  double temp_in_degrees;

  // initialized button at Digital Input 3
  mraa_gpio_context button;
  button = mraa_gpio_init(73);
  mraa_gpio_dir(button, MRAA_GPIO_IN);

  int volt_temp = 0;

  struct pollfd pfds[1];
  pfds[0].fd = STDIN_FILENO; 
  pfds[0].events = POLLIN | POLLHUP | POLLERR;
	
  while (true) {
		
    if (generate_reports == true) {
      volt_temp = mraa_aio_read(temp_sensor);
      temp_in_degrees = convert_volt_to_temp(volt_temp);
			
      get_local_time(&local_timezone, local_time_info, time_str);
      print_local_time(temp_in_degrees);
      //fprintf(stderr, "begin\n");
      time(&begin); 
      time(&end);	
		
    }
		
    int elapsed_time = difftime(end, begin);
    while (elapsed_time < time_period) {
      //fprintf(stderr, "%d, %d, %d\n", begin, end, time_period);
			
      if (mraa_gpio_read(button)) {
	get_local_time(&local_timezone, local_time_info, time_str);
	print_local_time(temp_in_degrees);	
	shut_down(log_file);
	//fprintf(stderr, "end\n");
      }

      int ret = poll(pfds, 1, 0);
      if (ret == -1) {
	perror("polling error\n");
	exit(1);
      }

      if (pfds[0].revents & POLLIN) {
	scanf("%s", command);
	process_commands();
      }

      if (generate_reports == true) {
	time(&end);
	elapsed_time = difftime(end, begin);
      }

    }
  }
	
  return 0;
}
