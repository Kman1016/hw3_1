#include "mbed.h"
#include <cstdint>
Thread thread_master;
Thread thread_slave;

// master def.
SPI spi(D11, D12, D13);
DigitalOut cs(D9);

// slave def.
SPISlave device(PD_4, PD_3, PD_1, PD_0);

int slave()
{
   device.format(16, 3); // 1+8+16
   device.frequency(1000000);
   uint16_t storage[256] = {};
   int wr_mode = 0;  // 1: read, 0: write
   int cmd, addr;
   //device.reply(0x00); // Prime SPI with first reply
    while (1)
    {
        if (device.receive())
        {
            cmd = device.read(); // Read byte from master
            printf("Slave's cmd read from master: %d\n", cmd);
            wr_mode = cmd >> 9;
            if (wr_mode == 1) {
                device.reply(wr_mode); // slave reply
                cmd = device.read();   // master read back & claer
                
                addr = cmd % 512;
                printf("Slave: cmd is mode = %d\n", wr_mode);
                printf("Slave: Addr is %d\n", addr);
                
                device.reply(storage[addr]); // slave reply
                cmd = device.read();         // master read back & claer
            }
            else if(wr_mode == 0) {
                device.reply(wr_mode);  // slave reply
                cmd = device.read();    // master read back & claer
                
                addr = cmd % 512; 
                printf("Slave: cmd is mode = %d\n", wr_mode);
                printf("Slave: Addr is %d\n", addr);
                
                storage[addr] = device.read();
                printf("slave: I get storage[%d] = %hd\n", addr, storage[addr]);
            }
            

        }
    }
}

void master()
{
   int number = 0;
   bool data_mode = 0;
   int wr_mode; // 1: read; 0: write
   int addr;
   int cmd;
   int response = 0;
   uint16_t data; 
   
   // Setup the spi for 8 bit data, high steady state clock,
   // second edge capture, with a 1MHz clock rate
   spi.format(16, 3);
   spi.frequency(1000000);

   for(int i=0; i<5; ++i){ //Run for 5 times
      // Chip must be deselected
        cs = 1;
        // Select the device by seting chip select low
        cs = 0;
        

        //printf("----------master----------\n");
        
        printf("R/W & Addr:\n");
        scanf("%d.%d", &wr_mode, &addr);
        printf("%d & %d\n", wr_mode, addr);
        if (wr_mode == 0) {
            printf("Input Data:\n");
            scanf("%hd", &data);
            printf("%hd\n", data);
        }
        cmd = (wr_mode << 9) + addr;
        spi.write(cmd);
        ThisThread::sleep_for(100ms); //Wait for debug print

        response = spi.write(cmd);    // read back
        printf("master: I get reply: %d\n", response);
        
        ThisThread::sleep_for(100ms);
        if (response == 1) {
            data = spi.write(cmd);
            printf("master: I read data = %d\n", data);
        } else {
            spi.write(data);
        }
        ThisThread::sleep_for(100ms);
        
        
        
    }
}

// main() runs in its own thread in the OS
int main()
{
    thread_slave.start(slave);
    thread_master.start(master);
}

