/// main.cpp
//
#include "BaseID.h"
#include "Manager_if.h"
#include "Factory_if.h"
#include "command_if.h"


/// -------------------------------------------------
/// 强制连接.a库中的一些原本自动执行的全局构建符号
/// -------------------------------------------------
CPPBUILDUNIT_FORCELINK(vRegOsTaskStubFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegOsSemStubFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCObjectManagerToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCDispatchToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCNotifyToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCControlToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCTimerToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCScheduleToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCStatusToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCResponseToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCModelToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCDataToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCDataMemToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCDataFileToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCDataMySQLToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCTlvToSQLToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCConnectToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCProxyToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCSessionToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCUserToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCSecureToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCAccessToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCCommandToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCHttpServerToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCHttpRequestToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCHttpProcessToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCHttpJsonToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCHttpResponseToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCMonitorToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCAppBaseToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCPythonXToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCLuaXToFactoryFunc, 0);

/// -------------------------------------------------
/// 本段代码来自《The Cert C Secure Coding Standard》
/// -------------------------------------------------
#define STRING(n)       STRING_AGAIN(n)
#define STRING_AGAIN(n) #n
#define CHARS_TO_READ   255

/// -------------------------------------------------
/// 全局入口对象
/// -------------------------------------------------
IManager *g_piManager = NULL;
ICommand *g_piCommand = NULL;

/*******************************************************
  函 数 名: DcopServer_Output
  描    述: 输出
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void DcopServer_Output(const char *info, void *)
{
    if (!info) return;

    printf("%s", info);
    fflush(stdout);
}

/*******************************************************
  函 数 名: DcopServer_Start
  描    述: 启动
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void DcopServer_Start()
{
    /// 打开日志记录
    DebugLogStatus(3);

    /// 打开内存跟踪
    DebugMemStatus(1);

    printf("-----------------------------------------------\n");
    printf(" Dcop Server Start ... \n");
    printf("-----------------------------------------------\n");
    
    objBase *piBase = objBase::GetInstance();
    if (!piBase)
    {
        printf("  Dcop Server Start Failed! (objBase Kernel Null) \n");
        return;
    }

    g_piManager = (IManager *)piBase->Deploy("../../res/server.xml");
    if (!g_piManager)
    {
        printf("  Dcop Server Start Failed! (Because Deploy Failed) \n");
        return;
    }

    DCOP_QUERY_OBJECT_REFER(ICommand, DCOP_OBJECT_COMMAND, g_piManager, 0, g_piCommand);
    if (!g_piCommand)
    {
        printf("  Dcop Server Start Failed! (Query Command Failed) \n");
        return;
    }

    g_piCommand->Out((LOG_PRINT)DcopServer_Output);
    printf("  Dcop Server Start OK! \n");
}

/*******************************************************
  函 数 名: DcopServer_Welcome
  描    述: 欢迎
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void DcopServer_Welcome(const char *username)
{
    if (!g_piCommand) return;

    g_piCommand->Welcome(username);
}

/*******************************************************
  函 数 名: DcopServer_Command
  描    述: 命令
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void DcopServer_Command(const char *command)
{
    if (!g_piCommand) return;

    g_piCommand->Line(command);
}

/*******************************************************
  函 数 名: DcopServer_Stop
  描    述: 停止
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void DcopServer_Stop()
{
    DCOP_RELEASE_INSTANCE_REFER(0, g_piCommand);

    if (g_piManager)
    {
        delete g_piManager;
        g_piManager = NULL;
    }

    printf("-----------------------------------------------\n");
    printf(" Dcop Server Stop! \n");
    printf("-----------------------------------------------\n");
}

/*******************************************************
  函 数 名: main
  描    述: 进程入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
int main(int argc, char *argv[])
{
    /// 正式入口 : 启动聊天服务器
    DcopServer_Start();

    printf(">>>>>>>>> Enter The Program, ('exit' to exit)! \n");

    char command[CHARS_TO_READ + 1];
    memset(command, 0, sizeof(command));
    bool bExitMode = false;

    DcopServer_Welcome("root");

    for (;;)
    {
        /// 接收控制台输入命令字符串
        /// scanf("%"STRING(CHARS_TO_READ)"s", command);
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            continue;
        }

        /// 退出模式下，判断是否确认，否则就恢复到正常模式
        if (bExitMode)
        {
            if (!strcmp(command, "Y\n"))
            {
                break;
            }

            bExitMode = false;
            DcopServer_Command("\n");
            continue;
        }

        /// 判断接收的命令是否是退出命令
        if (!strcmp(command, "exit\n"))
        {
            bExitMode = true;
            printf("Are you sure to exit? (Y/N)");
            continue;
        }

        /// 处理正常的命令
        DcopServer_Command(command);
    }

    printf(">>>>>>>>> Exit The Program, Bye! \n");
    DcopServer_Stop();

    return 0;
}
