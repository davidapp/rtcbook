#pragma once

/*
* Hello Server 的协议
* [1字节]命令字+[n-1字节]命令内容
*/

#define HELLO_RESULT_SUCCESS 0
#define HELLO_RESULT_FAIL 1

// Client -> Server

#define HELLO_CS_CMD_HEARTBEAT 0  // 心跳包，每10秒一次
//Req:[1字节命令字]
//Rsp:[1字节命令字 + 1字节结果]

#define HELLO_CS_CMD_GETINFO  1  // 拉取 Server 上的人员信息 
//Req:[1字节命令字]
//Rsp:[1字节命令字 + 1字节结果 + 2字节后面元素个数n + n*(4字节ID + 2字节名字长度 + 2字节名字)]

#define HELLO_CS_CMD_SENDTEXT 2  // 给 Server 上指定的人(Index)发送一条文本消息 
//Req:[1字节命令字 + 4字节对象ID + 4字节字符串长度 + 字符串]
//Rsp:[1字节命令字 + 1字节结果]

#define HELLO_CS_CMD_SETNAME   3 // 设置自己的名字 
//Req:[1字节命令字 + 2字节名字长度 + 名字]
//Rsp:[1字节命令字 + 1字节结果]

#define HELLO_CS_CMD_BROADCAST 4 // 给 Server 上的所有人广播一条文本消息 
//Req:[1字节命令字 + 4字节字符串长度 + 字符串]
//Rsp:[1字节命令字 + 1字节结果]


// Server->Client

#define HELLO_SC_CMD_ENTER    101  // 有人进来了 
//[1字节命令字 + 4字节ID + 2字节名字长度 + 名字]

#define HELLO_SC_CMD_LEAVE    102  // 有人退出了 
//[1字节命令字 + 4字节ID + 2字节名字长度 + 名字]

#define HELLO_SC_CMD_PMSG    103   // 有人给你发了一条消息 
//[1字节命令字 + 4字节From ID + 4字节消息长度 + 消息]

#define HELLO_SC_CMD_GMSG    104   // 有人群发了一条消息 
//[1字节命令字 + 4字节From ID + 4字节消息长度 + 消息]
