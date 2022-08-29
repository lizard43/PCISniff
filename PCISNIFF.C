#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dos.h>

typedef unsigned int word;

#define VENDORID  0x00
#define DEVICEID  0x02

/*
// PCI matching struct
*/
struct CONFIG
{
   word wVendorID;
   word wDeviceID;
};


/*
// Checks if we have a PCI bus
*/
int CheckBIOS()
{
   union REGS reg;
   
   /*
   // int 0x1A func 
   */
   
   reg.x.ax = 0xB101;
   int86(0x1A, &reg, &reg);
   
   /*
   // 
   */
   if (reg.h.ah == 0x00)
	{
		printf("PCI Bus = %d\n", reg.h.cl);
      return reg.h.cl;
   }
   else
   {
      printf("No PCI BUS detected\n");
      return -1;
   }
}

/*
// Reads a WORD from the PCI config block.
*/
word GetWordConfig(int iBus, int iDevice, int iFunc, int iIndex)
{
   union REGS regs, outregs;

   /*
   // int 0x1A func
   */
   regs.x.ax = 0xB109;
   regs.h.bh = iBus;

   /*
   // 
   */
   regs.h.bl = (iDevice << 3) | (iFunc & 0x07);

   regs.x.di = iIndex;
   
   int86(0x1A, &regs, &outregs);
   
   /*
   //
   */
   if (outregs.x.cflag !=0x00)
   {
      return 0;
   }
   else
   {
      return outregs.x.cx;
   }
}

/*
//
*/
int PCIMatch(struct CONFIG *pConfig)
{
	struct CONFIG matchConfig[3][32];
   FILE *fp = NULL;
   int iType = 0;
	char szLine[80] = "";
	int iBus = 0;
   int iDevice = 0;
   word wVendorID = 0;
   word wDeviceID = 0;

   if (NULL == pConfig)
   {
      return 0;
   }

   memset(matchConfig, 0, sizeof(matchConfig));

   if (NULL == (fp = fopen("PCICfg.txt", "rt")))
   {
      printf("Error opening PCICfg.txt\n");
      return 0;
   }

   /*
   // Loop until EOF or PC match found
   */
   while (fgets(szLine, sizeof(szLine), fp))
   {
      if (szLine[0] != '\'')
      {
      /*
      // Is this start of a new PC Type?
         */
         if (szLine[0] == 'T')
         {
         /*
         // do we have a match?
            */
            if (!memcmp(matchConfig, pConfig, sizeof(matchConfig)))
            {
               fclose(fp);
               return iType;
            }

            /*
            // Clear match
            */
            memset(matchConfig, 0, sizeof(matchConfig));

            /*
            // save PC Type
            */
            iType = atoi(&szLine[1]);
			}

			/*
			// bus
			*/
			if (szLine[0] == 'B')
			{
				iBus = atoi(&szLine[1]);
			}

         /*
         // device
         */
         if (szLine[0] == 'D')
         {
            iDevice = atoi(&szLine[1]);
         }


         /*
         // VendorID
         */
         if (szLine[0] == 'V')
         {
            wVendorID = (unsigned)atol(&szLine[1]);
				matchConfig[iBus][iDevice].wVendorID = wVendorID;
         }

         /*
         // deviceID
         */
         if (szLine[0] == 'I')
         {
            wDeviceID = (unsigned)atol(&szLine[1]);
				matchConfig[iBus][iDevice].wDeviceID = wDeviceID;
         }
      }
   }

   /*
	// do we have a match?
   */
   if (!memcmp(matchConfig, pConfig, sizeof(matchConfig)))
   {
      fclose(fp);
      return iType;
   }


   fclose(fp);

	return 0;
}

/*
//
*/
int main(int argc, char *argv[])
{
   int iBus = 0;
   int iDevice = 0;
   int iMaxBus = 0;
   int iFunc = 0;
   word wVendorID = 0;
   word wDeviceID = 0;
	struct CONFIG config[3][32];

	memset(config, 0, sizeof(config));

   /*
   //
   */
   iMaxBus = CheckBIOS();

   /*
   //
   */
   if (-1 == iMaxBus)
   {
      return 0;
   }

   /*
   //
	*/
	for (iBus = 0; iBus < iMaxBus; iBus++)
	{
	for (iDevice = 9; iDevice < 32; iDevice++)
   {
   /*
	// Get VENDORID - Note Func = 0
      */
      wVendorID = GetWordConfig(iBus, iDevice, iFunc, VENDORID);

      /*
      //
      */
      if (0xFFFF != wVendorID)
      {
      /*
      // Get DEVICEID - Note using Bus = 0, Func = 0
         */
         wDeviceID = GetWordConfig(iBus, iDevice, iFunc, DEVICEID);

         /*
         //
         */
			config[iBus][iDevice].wVendorID = wVendorID;
			config[iBus][iDevice].wDeviceID = wDeviceID;

			printf("Bus = %02d  Device = %02d  VendorID = %05u = %04Xh DeviceID = %05u = %04Xh\n",
				iBus, iDevice, wVendorID, wVendorID, wDeviceID, wDeviceID);
		}
		}
   }

   /*
   //
   */


	return PCIMatch(&config[0][0]);
}