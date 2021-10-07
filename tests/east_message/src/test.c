#include <stdio.h>

#include "types.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "east_message.h"

/* -------------------------------------------------------------------------- */

typedef FW_BOOLEAN (* TestFunction_t)(void);

static EAST_p pEAST             = NULL;
static U8     eastContainer[16] = {0};
static U8     eastBuffer[100]   = {0};

/* --- Tests ---------------------------------------------------------------- */

static FW_BOOLEAN Test_SomeCase(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("*** Some Case Test ***\r\n");

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_InitSuccess(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;

    DBG("*** EAST Success Initialization Test ***\r\n");

    pEAST = EAST_Init(eastContainer, sizeof(eastContainer), NULL, 0);
    result = (FW_BOOLEAN)(NULL != pEAST);
    if (FW_FALSE == result) return result;

    status = EAST_SetBuffer(pEAST, eastBuffer, sizeof(eastBuffer));
    result = (FW_BOOLEAN)(FW_SUCCESS == status);

    return result;
}

/* -------------------------------------------------------------------------- */

static U8 gTestEastPacketSuccess[] =
{
    /* Start Token */
    0x24,
    /* Length */
    0x05, 0x00,
    /* Data */
    0x01, 0x02, 0x03, 0x04, 0x05,
    /* Stop Token */
    0x42,
    /* Control Sum */
    0x01, 0x00
};

static FW_BOOLEAN Test_PutSuccess(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 byte = 0, test = 0, size = 0;
    U8 * testPacket = NULL;

    DBG("*** EAST Success Put Byte Test ***\r\n");

    testPacket = gTestEastPacketSuccess;
    size = sizeof(gTestEastPacketSuccess);

    /* Init the EAST packet */
    pEAST = EAST_Init(eastContainer, sizeof(eastContainer), NULL, 0);
    result = (FW_BOOLEAN)(NULL != pEAST);
    if (FW_FALSE == result) return result;

    for (test = 0; test < 2; test++)
    {
        /* Setup/Reset the EAST packet */
        status = EAST_SetBuffer(pEAST, eastBuffer, sizeof(eastBuffer));
        result = (FW_BOOLEAN)(FW_SUCCESS == status);
        if (FW_FALSE == result) break;

        /* Fill the EAST packet */
        for (byte = 0; byte < size; byte++)
        {
            status = EAST_PutByte(pEAST, testPacket[byte]);
            if ( (size - 1) == byte )
            {
                result &= (FW_BOOLEAN)(FW_COMPLETE == status);
            }
            else
            {
                result &= (FW_BOOLEAN)(FW_INPROGRESS == status);
            }
            if (FW_FALSE == result) break;
        }
        if (FW_FALSE == result) break;

        result &= (FW_BOOLEAN)(eastBuffer[0] == testPacket[3]);
        result &= (FW_BOOLEAN)(eastBuffer[2] == testPacket[5]);
        result &= (FW_BOOLEAN)(eastBuffer[4] == testPacket[7]);
        if (FW_FALSE == result) break;
    }

    return result;
}

/* -------------------------------------------------------------------------- */

static U8 gTestEastPacketWrongLength[] =
{
    /* Start Token */
    0x24,
    /* Length */
    0x00, 0x05,
    /* Data */
    0x01, 0x02, 0x03, 0x04, 0x05,
    /* Stop Token */
    0x42,
    /* Control Sum */
    0x01, 0x00
};

static FW_BOOLEAN Test_PutWrongLength(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 byte = 0, size = 0;
    U8 * testPacket = NULL;

    DBG("*** EAST Wrong Length Put Byte Test ***\r\n");

    testPacket = gTestEastPacketWrongLength;
    size = sizeof(gTestEastPacketWrongLength);

    /* Init the EAST packet */
    pEAST = EAST_Init(eastContainer, sizeof(eastContainer), NULL, 0);
    result = (FW_BOOLEAN)(NULL != pEAST);
    if (FW_FALSE == result) return result;

    /* Setup/Reset the EAST packet */
    status = EAST_SetBuffer(pEAST, eastBuffer, sizeof(eastBuffer));
    result = (FW_BOOLEAN)(FW_SUCCESS == status);
    if (FW_FALSE == result) return result;

    /* Fill the EAST packet */
    for (byte = 0; byte < size; byte++)
    {
        status = EAST_PutByte(pEAST, testPacket[byte]);
        result &= (FW_BOOLEAN)(FW_INPROGRESS == status);
        if (FW_FALSE == result) break;
    }

    return result;
}

/* -------------------------------------------------------------------------- */

static U8 gTestEastPacketWrongCS[] =
{
    /* Start Token */
    0x24,
    /* Length */
    0x05, 0x00,
    /* Data */
    0x01, 0x02, 0x03, 0x04, 0x05,
    /* Stop Token */
    0x42,
    /* Control Sum */
    0x00, 0x01
};

static FW_BOOLEAN Test_PutWrongCS(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 byte = 0, size = 0;
    U8 * testPacket = NULL;

    DBG("*** EAST Wrong CS Put Byte Test ***\r\n");

    testPacket = gTestEastPacketWrongCS;
    size = sizeof(gTestEastPacketWrongCS);

    /* Init the EAST packet */
    pEAST = EAST_Init(eastContainer, sizeof(eastContainer), NULL, 0);
    result = (FW_BOOLEAN)(NULL != pEAST);
    if (FW_FALSE == result) return result;

    /* Setup/Reset the EAST packet */
    status = EAST_SetBuffer(pEAST, eastBuffer, sizeof(eastBuffer));
    result = (FW_BOOLEAN)(FW_SUCCESS == status);
    if (FW_FALSE == result) return result;

    /* Fill the EAST packet */
    for (byte = 0; byte < size; byte++)
    {
        status = EAST_PutByte(pEAST, testPacket[byte]);
        result &= (FW_BOOLEAN)(FW_INPROGRESS == status);
        if (FW_FALSE == result) break;
    }

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_GetSuccess(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 byte = 0, size = 0;
    U8 * testPacket = NULL;
    U8 value = 0;

    DBG("*** EAST Success Get Byte Test ***\r\n");

    testPacket = gTestEastPacketSuccess;
    size = sizeof(gTestEastPacketSuccess);

    /* Init the EAST packet */
    pEAST = EAST_Init(eastContainer, sizeof(eastContainer), NULL, 0);
    result = (FW_BOOLEAN)(NULL != pEAST);
    if (FW_FALSE == result) return result;

    /* Fill the packet buffer */
    for (byte = 0; byte < (size - 6); byte++)
    {
        eastBuffer[byte] = testPacket[byte + 3];
    }

    /* Setup/Reset the EAST packet */
    status = EAST_SetBuffer(pEAST, eastBuffer, (size - 6));
    result = (FW_BOOLEAN)(FW_SUCCESS == status);
    if (FW_FALSE == result) return result;

    size = EAST_GetMessageSize(pEAST);
    result = (FW_BOOLEAN)((sizeof(gTestEastPacketSuccess) - 6) == size);
    if (FW_FALSE == result) return result;

    /* Get the EAST packet */
    size = sizeof(gTestEastPacketSuccess);
    for (byte = 0; byte < size; byte++)
    {
        status = EAST_GetByte(pEAST, &value);
        if ( (size - 1) == byte )
        {
            result &= (FW_BOOLEAN)(FW_COMPLETE == status);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_INPROGRESS == status);
        }
        if (FW_FALSE == result) break;

        result &= (FW_BOOLEAN)(value == testPacket[byte]);
        if (FW_FALSE == result) break;
    }

    return result;
}

/* --- Test Start Up Function (mandatory, called before RTOS starts) -------- */

void vTestStartUpFunction(void)
{
  DBG_ClearScreen();
  DBG("*** Start Up Test ***\r\n");
}

/* --- Test Prepare Function (mandatory, called before RTOS starts) --------- */

void vTestPrepareFunction (void)
{
  DBG("*** Prepare Test ***\r\n");
}

/* --- Helper Task Main Function (mandatory) -------------------------------- */

void vTestHelpTaskFunction(void * pvParameters)
{
  //DBG("LED Task Started\r\n");

  while (FW_TRUE)
  {
      //DBG("LED Hi\r\n");
      vTaskDelay(50);
      //DBG("LED Lo\r\n");
      vTaskDelay(50);
  }
  //vTaskDelete(NULL);
}

/* --- Test Cases List (mandatory) ------------------------------------------ */

const TestFunction_t gTests[] =
{
    Test_SomeCase,
    Test_InitSuccess,
    Test_PutSuccess,
    Test_PutWrongLength,
    Test_PutWrongCS,
    Test_GetSuccess,
};

U32 uiTestsGetCount(void)
{
    return (sizeof(gTests) / sizeof(TestFunction_t));
}

/* --- Error callback function (mandatory) ---------------------------------- */

void on_error(S32 parameter)
{
  while (FW_TRUE) {};
}

/* -------------------------------------------------------------------------- */
