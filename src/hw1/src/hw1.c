#include "stdio.h"
#include "stdlib.h"
#include <string.h>  
#include "stdbool.h"
#include <signal.h>
#include <unistd.h>
#include <sys/fcntl.h>

typedef struct Case{
    int total_case;
    int mild_case[9];
    int severe_case[9];

    // add function pointer to set case here
    void (*set_case)(struct Case *self, int area, int case_condition, int case_amount);
    // add function pointer returning both cases by reference here
    void (*get_case)(struct Case *self, int area, int *mild_case, int *severe_case);
    // add function pointer to check if an area has cases
    bool (*has_case)(struct Case *self, int area);
    // add function pointer to get total cases by reference
    void (*get_total_case)(struct Case *self, int *total_case);
    
    

}Case;


bool program_terminate = false;


// accept user input and set case
void set_case(Case *self, int area, int case_condition, int case_amount){
    if(case_condition == 'm'){
        self->mild_case[area] += case_amount;
    }
    else if(case_condition == 's'){
        self->severe_case[area] += case_amount;
    }
}

// actual implementation of get_case
void get_case(Case *self, int area, int *mild_case, int *severe_case){
    *mild_case = self->mild_case[area];
    *severe_case = self->severe_case[area];
}

// check if an area has cases
bool has_case(Case *self, int area){
    if(self->mild_case[area] > 0 || self->severe_case[area] > 0){
        return true;
    }
    else{
        return false;
    }
}

// get total cases
void get_total_case(Case *self, int *total_case){
    int i;
    for(i = 0; i < 9; i++){
        *total_case += self->mild_case[i];
        *total_case += self->severe_case[i];
    }
}

// a handler dealing with SIGINT
void sigint_handler(int signum){
    program_terminate = true;
}

char *strrev(char *str){
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}


enum function_type{
    Confirmed_case = '1',
    Reportings,
    Exit
};

int main(int argc, char **argv){

    signal(SIGINT, sigint_handler);
    
    char program_mode = 0;
    Case reporting_system;

    // initialize the function pointers
    reporting_system.set_case = set_case;
    reporting_system.get_case = get_case;
    reporting_system.has_case = has_case;
    reporting_system.get_total_case = get_total_case;

    // open a device file, /dev/hw1_led, and check if it is opened successfully
    // O_RDWR: open for reading and writing is included in sys/fcntl.h
    int led_fd = open("/dev/hw1_led", O_RDWR);
    if(led_fd < 0){
        printf("Error opening /dev/hw1_led\n");
        exit(-1);
    }

    // open a device file, /dev/hw1_7seg, and check if it is opened successfully
    // O_RDWR: open for reading and writing is included in sys/fcntl.h
    int seven_seg_fd = open("/dev/hw1_7seg", O_RDWR);
    if(seven_seg_fd < 0){
        printf("Error opening /dev/hw1_7seg\n");
        exit(-1);
    }


    // initialize the case array
    for(int i = 0; i < 9; i++){
        reporting_system.mild_case[i] = 0;
        reporting_system.severe_case[i] = 0;
    }

    while(!program_terminate){
        printf("1. Confirmed case\n");
        printf("2. Reporting system\n");
        printf("3. Exit\n");
        // read input
        scanf(" %c", &program_mode);
        // switch to different function by program mode
        switch(program_mode){
            case Confirmed_case:{
                char cmd;
                char buf[9];
                // show case of each area
                printf("Showing cases of each area by LEDs.\n");
                for(int i = 0; i < 9; i++){
                    int mild_case, severe_case;
                    reporting_system.get_case(&reporting_system, i, &mild_case, &severe_case);
                    // print the sum of two cases
                    int total_case = mild_case + severe_case;
                    printf("Area %d: %d\n", i, total_case);
                    // write the sum of two cases to the device file( + '0' to convert int to char)
                    buf[i] = reporting_system.has_case(&reporting_system, i) + 48;
                }
                // write the buf array to the device file(led_fd)
                write(led_fd, buf, 9);

                while(1){
                    printf("Choose a cmd: \n");
                    printf("q) for quit.\n");
                    printf("0~8) for focusing on a centain aria.\n");
                    printf("Else then keep idling.\n\n\n\n");
                    scanf(" %c", &cmd);
                    // bread if cmd is 'q'
                    if(cmd == 'q'){
                        break;
                    }
                    // if cmd is a number, report case of that area
                    else if(cmd >= '0' && cmd <= '9'){
                        // toggle(0 and 1 respectively) the number of this area to device file(led_driver) ever 0.5 second, totally 3 second
                        for(int i = 0; i < 6; i++){
                            buf[cmd - '0'] = (buf[cmd - '0'] == '0') + 48;
                            write(led_fd, buf, 9);
                            sleep(0.5);
                        }
                        printf("Showing cases of a certain area by LEDs.\n");
                        int mild_case, severe_case;
                        reporting_system.get_case(&reporting_system, cmd - 48, &mild_case, &severe_case);
                        int total_case = mild_case + severe_case;
                        printf("Area %d: %d\n", cmd - 48, total_case);
                        // write the total case to 7 seg device
                        // if total case is larger than 10, split it into number of its digits
                        if(total_case >= 10){
                            sprintf(buf,"%d",total_case);
                            strcpy(buf,strrev(buf));
                            for(int temp = 0; temp < strlen(buf); temp ++){
                                write(seven_seg_fd,&buf[temp],1);
                                sleep(0.5);
                            }
                        }
                        else{
                            buf[0] = total_case + 48;
                            // write the buf array to the device file(seven_seg_fd)
                            write(seven_seg_fd, buf, 1);
                        }
                        
                    }
                    // if cmd is neither a number nor 'q' then show case of each area
                    else{
                        for(int i = 0; i < 9; i++){
                            int mild_case, severe_case;
                            reporting_system.get_case(&reporting_system, i, &mild_case, &severe_case);
                            int total_case = mild_case + severe_case;
                            printf("Area %d: %d\n\n\n\n", cmd - 48, total_case);
                        }

                        // get total case of all areas and write to 7 seg device
                        int total_case = 0;
                        reporting_system.get_total_case(&reporting_system, &total_case);
                        if (total_case >= 10)
                        {

                            sprintf(buf,"%d",total_case);
                            for(int temp = 0; temp < strlen(buf); temp ++){
                                write(seven_seg_fd,&buf[temp],1);
                                sleep(0.5);
                            }
                        }
                        else
                        {
                            buf[0] = total_case + 48;
                            // write the buf array to the device file(seven_seg_fd)
                            write(seven_seg_fd, buf, 1);
                        }
                    }

                }
                break;

            }

            case Reportings:{

                char area;
                printf("Area(0~8): ");
                while(1){
                    printf("Choose a cmd: \n");
                    printf("e) for quit.\n");
                    printf("0~8) for focusing on a centain aria.\n");
                    printf("Else then keep idling.\n\n\n\n");
                    scanf(" %c", &area);

                    // if area is 'e' then break
                    if(area == 'e'){
                        break;
                    }
                    // if area is 'c' continue
                    else if(area == 'c'){
                        printf("Area(0~8): ");
                        continue;
                    }
                    // if area is a number, ask for case condition and set case
                    else if(area >= '0' && area <= '9'){
                        char case_condition;
                        int case_amount;
                        printf("Case condition(mild: m, severe: s): ");
                        scanf(" %c", &case_condition);
                        printf("Case amount: ");
                        scanf("%d", &case_amount);
                        reporting_system.set_case(&reporting_system, area - 48, case_condition, case_amount);
                        // after setting case, show total confirmed_case by concluding mild_case and severe_case of each area
                        int total_mild_case = 0;
                        int total_severe_case = 0;
                        int total_case = 0;
                        for(int i = 0; i < 9; i++){
                            int mild_case, severe_case;
                            reporting_system.get_case(&reporting_system, i, &mild_case, &severe_case);
                            total_mild_case += mild_case;
                            total_severe_case += severe_case;
                        }
                        total_case = total_mild_case + total_severe_case;
                        printf("Total confirmed case: %d\n", total_case);

                        // write the specific area to the device file(led_driver)
                        char buf[9];
                        printf("Showing the area reporting now by LEDs.\n\n\n");
                        for(int i = 0; i < 9; i++){
                            buf[i] = (i == (area - 48)) + 48;
                        }
                        write(led_fd, buf, 9);
                        // write the total case to 7 seg device
                        if (total_case >= 10)
                        {
                            sprintf(buf,"%d",total_case);
                            for(int temp = 0; temp < strlen(buf); temp ++){
                                write(seven_seg_fd,&buf[temp],1);
                                sleep(0.5);
                            }
                        }
                        else
                        {
                            buf[0] = total_case + 48;
                            // write the buf array to the device file(seven_seg_fd)
                            write(seven_seg_fd, buf, 1);
                        }
                    }

                }
                break;
            }

            case Exit:{
                program_terminate = true;
                // close all leds by sending 0 to led_driver
                printf("Closing all LEDs.\n");
                printf("Closing 7 seg.\n");
                char buf[9];
                for(int i = 0; i < 9; i++){
                    buf[i] = 0 + 48;
                }
                write(led_fd, buf, 9);
                // close 7 seg device by sending 'c' to seven_seg_driver
                buf[0] = 'c';
                write(seven_seg_fd, buf, 1);
                break;
            }
        }
    }


    // clese device descriptor
    close(led_fd);
    close(seven_seg_fd); 
    return 0;
}