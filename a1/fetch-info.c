// Name: Grace Bertozzi V01012576

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* NOTES:
cpu model:     find in /proc/cpuinfo  model name      : Intel(R) Xeon(R) Gold 6254 CPU @ 3.10GHz
num cores:     find in /proc/cpuinfo  cpu cores       : 1
linux version: find in /proc/version  correct format, single line file
total memory:  find in /proc/meminfo  correct format
uptime:        find in /proc/uptime   631948.14 2491590.17 
        value 1 is total uptime in seconds since last reboot
        want to convert to this form: Uptime: 32 days, 0 hours, 20 minutes, 36 seconds

process name:               find in /proc/<pid>/status  correct format   
pid:                        pid given as input, print using formatted string
commandline:                find in /proc/<pid>/cmdline correct format(?), single line OR empty file
number of threads:          find in /proc/<pid>/status  correct format
number of contect switches: find in /proc/<pid>/status
        must add values from two lines


useful:

char *strstr (const char *s1, const char *s2); 
- takes two strings s1 and s2 as arguments and finds the first occurrence of the string s2 in the string s1
- returns pointer to first occurence of s2 in s1 else NULL

char *fgets(char *buffer, sizeof(buffer), FILE *stream);
- reads single line of text from a specified input stream and stores it in a char arrary,
- pointer iterates to next line until buffer capacity reached or EOF

char *strtok(char *str, const char *delims);
*/

// input: filename and pattern, prints formatted line where pattern is found in file
void print_info(char *file_name, char *pattern) {
    FILE *file = fopen(file_name, "r"); // open file
    if (!file) {
        printf("error opening file\n");
        return;
    }
    char buffer[1000]; // store fgets input
    int printed = 0; // bool flag to ensure only first instance is printed

    while (fgets(buffer, sizeof(buffer), file) && !printed) {
        if (strstr(buffer, pattern)) { // ie match found 
            char *label = strtok(buffer, ":");  // Get the label (before `:`)
            char *value = strtok(NULL, "");

                if (label && value) { // Remove trailing spaces from label
                    int len = strlen(label);
                    while (len > 0 && isspace(label[len - 1])) {
                        label[len - 1] = '\0'; 
                        len--;
                    }
                }

                printf("%s: %s", label, value);
                printed = 1; 
        } 
    }
    if (printed == 0) {
        printf("We didnt find a match.\n");
    }
    fclose(file); // close file
}

// prints "Uptime: -- days, -- hours, -- minutes, -- seconds"
void print_uptime() {
    FILE *file = fopen("/proc/uptime", "r");
    if (!file) {
        printf("Error opening uptime\n");
        return;
    }
    double upt;
    fscanf(file, "%lf", &upt); // get first value from file
    
    // convert from seconds
    int days = upt / 86400;         // 86400 seconds in a day
    int hours = (upt - days * 86400) / 3600;  // 3600 seconds in an hour
    int minutes = (upt - days * 86400 - hours * 3600) / 60;  // 60 seconds in a minute
    int seconds = (int)upt % 60;  

    printf("Uptime: %d days, %d hours, %d minutes, %d seconds\n", days, hours, minutes, seconds);

    fclose(file);
}

// print number and name if process is found, else error message
int print_pname(char *file_name, int p_num) {

    // have to check if /proc/<pid> directory exists
    FILE *file = fopen(file_name, "r");
    if (!file) {
        printf("Process number %d not found.\n", p_num);
        return -1;
    }
    char buffer[1000];

    // match on Name
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strstr(buffer, "Name")) { // ie match found 
            printf("Process number: %d\n", p_num); // print number
            printf("%s", buffer); // print name
        } 
    }
    fclose(file);
    return 0;
}

// prints contents of /proc/<pid>/cmdline
void print_console(int p_num) {
    char cmdline_path[100];
    char buffer[1000]; // store fgets input

    sprintf(cmdline_path, "/proc/%d/cmdline", p_num);

    FILE *file = fopen(cmdline_path, "r"); // open file
    if (!file) {
        printf("Error opening /proc/<pid>/cmdline\n");
        return;
    }
    memset(buffer, '\0', sizeof(buffer));
    fgets(buffer, sizeof(buffer), file);
    printf("Filename (if any): %s\n", buffer); // keeps printing out old buffer from nonvoluntary switches when there is no content if /cmdline
    
    fclose(file); // close file
}

void print_switches(char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        printf("Error opening /proc/p_num/sched\n");
        return;
    }
    int found = 0; // bool flag
    int x;
    char buffer[1000];

    while (fgets(buffer, sizeof(buffer), file) && !found) {
        if (strstr(buffer, "nr_switches")) { // line will read- voluntary_ctxt_switches:        0
            char* token = strtok(buffer, " "); // point to first token
            token = strtok(buffer, " "); // point to second token- "int"
            x = atoi(token); // convert to int
            found = 1;
        } 
    }
    printf("Total context switches: %d\n", x);
    
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc == 1) { // if argc = 1 then no process # 
        print_info("/proc/cpuinfo", "model name");
        print_info("/proc/cpuinfo", "cpu cores");
        print_info("/proc/version", "Linux version");
        print_info("/proc/meminfo", "MemTotal"); 
        print_uptime();
    }
    else if (argc == 2) { // if argc = 2 then process #
        char status_path[100];
        char sched_path[100];
        int p_num = atoi(argv[1]);
        sprintf(status_path, "/proc/%d/status", p_num); // format status path depending on input
        sprintf(sched_path, "/proc/%d/sched", p_num); // format sched path depending on input

        if (print_pname(status_path, p_num) == 0) { // if valid process number 
            print_console(p_num); // console command that started process if any
            print_info(status_path, "Threads"); // num threads
            print_switches(status_path); // total # context switches  
        } 
    }
    else {
        printf("Wrong number of parameters.\n");
    }

    return 0;
}
