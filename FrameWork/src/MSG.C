/*
-----------------------------------------------------------------------------------------------
@       Name :      Msg.c
@
@       Brief:
@
@       Date :
-----------------------------------------------------------------------------------------------
*/

#define  FW_MSG_GLOBALS


void MsgPost2Logic(INT8U msg) 
{
    if (msg<0x10){
        Message &= 0xF0;
    }
    Message |= msg;
}

void MsgPost2LightCtrl(INT8U msg) 
{
    if (msg<0x10){
        Message1 &= 0xF0;
    }
    Message1 |= msg;
}

