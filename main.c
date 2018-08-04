#include "msp.h"
#include "driverlib.h"
#include "stdio.h"
#include "uart.h"
#include "string.h"
#include "math.h"
/**
 * main.c
 */
volatile char receivedBuffer[1000];
int backwardStep(int phasecount);
int forwardStep(int phasecount);
int midpoint;
int phasecount;
int degreespertick = 550;
int checker;
int checker3 = 0;
int q = 0;
int checker2;
int n;
int intOut = 0;
int digit;
int denum;
int r = 0;
int s = 0;
volatile int numOut;
int newArray[];
//Configure timer A to interrupt every one second. Note the possibility of adding a lowpass filter later on in the code.
const Timer_A_UpModeConfig config =
{
 TIMER_A_CLOCKSOURCE_SMCLK,
 TIMER_A_CLOCKSOURCE_DIVIDER_32,
 10000,
 TIMER_A_TAIE_INTERRUPT_ENABLE,
 TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,
 TIMER_A_DO_CLEAR
};

//Configure UART
const eUSCI_UART_Config UART_init = {
                                     EUSCI_A_UART_CLOCKSOURCE_SMCLK, 19, 8, 0,
                                     EUSCI_A_UART_NO_PARITY,
                                     EUSCI_A_UART_LSB_FIRST,
                                     EUSCI_A_UART_ONE_STOP_BIT,
                                     EUSCI_A_UART_MODE,
                                     EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION };



void main(void)
{

    CS_setDCOFrequency(3E+6);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);

    //Define GPIOs and UART TX/RX
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    //Initialize UART
    UART_initModule(EUSCI_A0_BASE, &UART_init);
    UART_enableModule(EUSCI_A0_BASE);

    //Enable UART receive interrupts and master
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
        Interrupt_enableInterrupt(INT_EUSCIA0);
    //Initialize Timer A
    Timer_A_configureUpMode(TIMER_A0_BASE,&config);
    //Interrupt_enableInterrupt(INT_TA0_N);
    Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);
    Interrupt_enableMaster();

    char string[] = "Please enter a number\n\r";//Create String to send
           while (1)//While loop
           {
               if (n == 22)//end condition
               {
                   break;
               }

               UART_transmitData(EUSCI_A0_BASE, string[n]);//Transmit the string
               n++;//increment the counter
           }
           n = 0;
    while (1)//This is the real while loop
        {
            if(checker==1){//If the buffer is ready, enter the transmit loop
                int i = 0;//Initialize the transmit counter
                int seconds = 0;
                for(i=0;i<q;i++){
                   digit = receivedBuffer[i]-48;
                   denum = digit*pow(10,q-i-1);
                   numOut = numOut+denum;
                }
                //Convert numOut From Seconds To "Ticks"
                numOut = numOut*(degreespertick/60);
                //Save the old value of NumOut in a new variable
                midpoint = numOut;
                numOut = numOut*2;
                checker2 = 0;
                receivedBuffer[q+1] = '\n';//Add the newline character to the string
                receivedBuffer[q+2] = '\r';//Add the carriage return character to the string

                q = q+2;//Make q reflect the actual length
                Interrupt_disableMaster();//Disable interrupts
                for(i=0;i<q;i++){//Begin the transmit for loop
                    UART_transmitData(EUSCI_A0_BASE,receivedBuffer[i]);//Transmit
                    receivedBuffer[i] = NULL;//delete the string as you transmit
                }

                checker = 0;//reset the checker variable

                Interrupt_enableMaster();//reenable the interrupts

                //Enable The Timer Interrupt To begin the Motor Running Process; Disable The UART interrupt
                Interrupt_enableInterrupt(INT_TA0_N);
                Interrupt_disableInterrupt(INT_EUSCIA0);
                while(1){
                    if(numOut<=0){
                        q=0;
                        break;
                    }
                }
                //End the Process
                Interrupt_enableInterrupt(INT_EUSCIA0);
                Interrupt_disableInterrupt(INT_TA0_N);
                if(checker3==0){
                        char string[] = "Please enter a number\n\r";//Create String to send

                            while (1)//While loop
                            {   checker3 = 1;
                                if (n == 28)//end condition
                                {
                                    n=0;
                                    break;
                                }

                                UART_transmitData(EUSCI_A0_BASE, string[n]);//Transmit the string
                                n++;//increment the counter
                            }
                    }
                numOut = 0;
        }


    }


}



//Timer A 1 second interrupts
void TA0_N_IRQHandler(void){
    //printf("Time!\n");
    if(numOut>midpoint){
        phasecount = backwardStep(phasecount);
        checker2 = 0;
    } else if(numOut<=midpoint){

        if(checker2 == 0){
            checker2 = 1;
            char string[] = "Start?\n\r";//Create String to send
            Interrupt_disableMaster();
                while (1)//While loop
                {   checker3 = 0;
                    if (n == 8)//end condition
                    {
                        n = 0;
                        Interrupt_enableMaster();
                        break;
                    }

                    UART_transmitData(EUSCI_A0_BASE, string[n]);//Transmit the string
                    n++;//increment the counter
                }
                while(1){
                    if(UART_receiveData(EUSCI_A0_BASE)==' '){
                        break;
                    }
                }
        }
        phasecount = forwardStep(phasecount);

    }
    numOut = numOut-1;
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);


}


void EUSCIA0_IRQHandler(void)
{
    Interrupt_disableMaster();
    uint32_t interruptstatus = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);//Get the flag thing
    MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, interruptstatus);//clear the flag

    if (interruptstatus & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)//verify the thing, then get the data
    {
        receivedBuffer[q] = UART_receiveData(EUSCI_A0_BASE);//Add the data to the array
        if (receivedBuffer[q]!='\r'){//If the Line is not enter...

            q++;//increment the counter
        }else{//else

            checker = 1;//enable the transmission by flipping the checker


    }

}
    Interrupt_enableMaster();
}


int backwardStep(int phasecount){
    if(phasecount==0){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN5);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN6|GPIO_PIN7);
    }
    else if(phasecount==1){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN5|GPIO_PIN6);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN7);
    }
    else if(phasecount==2){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN6|GPIO_PIN7);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN5);
    }
    else if(phasecount==3){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN7);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN6|GPIO_PIN5);
    }

    if(phasecount==3){
        phasecount = 0;
    }
    else{
        phasecount++;
    }
    return phasecount;
}

int forwardStep(int phasecount){
    if(phasecount==0){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN5);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN6|GPIO_PIN7);
    }
    else if(phasecount==1){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN5|GPIO_PIN6);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN7);
    }
    else if(phasecount==2){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN6|GPIO_PIN7);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN5);
    }
    else if(phasecount==3){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN7);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN6|GPIO_PIN5);
    }

    if(phasecount==0){
        phasecount = 3;
    }
    else{
        phasecount--;
    }
    return phasecount;
}




