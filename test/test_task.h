/// -------------------------------------------------
/// test_task.h : Ö÷Òª²âÊÔwrapper task²Ù×÷
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_SUITE_TASK_H_
#define _TEST_SUITE_TASK_H_

#include "test.h"

#include "task.h"
#include "sem.h"
#include "msg.h"


class CTestSuiteTask : public ITestSuite
{
public:
    CTestSuiteTask();
    ~CTestSuiteTask();

    int TestEntry(int argc, char* argv[]);

    static void task_entry_recv(objTask::IPara *para);

    static void task_entry_send(objTask::IPara *para);

public:
    int m_iSenderCount;
    int m_iRecverCount;
};


#endif // #ifndef _TEST_SUITE_TASK_H_
