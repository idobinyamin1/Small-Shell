#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num)
{

  cout << "smash: got ctrl-Z" << endl;
  if (getpid() == SmallShell::getInstance().forground_PID)
  {
    return; // nothing on forground
  }
  if (kill(SmallShell::getInstance().forground_PID, 19) == -1)
  {
    perror("smash error: kill failed");
    return;
  }
  cout << "smash: process " << SmallShell::getInstance().forground_PID << " was stopped" << std::endl;

  SmallShell::getInstance().jobList.removeFinishedJobs();
  SmallShell::getInstance().jobList.addJob(SmallShell::getInstance().forground_cmd, true, 0, SmallShell::getInstance().forground_job_id);

  SmallShell::getInstance().forground_cmd = nullptr;
  SmallShell::getInstance().forground_job_id = -1;

  SmallShell::getInstance().forground_PID = getpid();
}

void ctrlCHandler(int sig_num)
{

  cout << "smash: got ctrl-C" << endl;
  if (getpid() == SmallShell::getInstance().forground_PID)
  {
    return; // nothing on forground
  }
  if (kill(SmallShell::getInstance().forground_PID, 9) == -1)
  {

    perror("smash error: kill failed");

    return;
  }

  cout << "smash: process " << SmallShell::getInstance().forground_PID << " was killed" << std::endl;

  SmallShell::getInstance().forground_PID = getpid();
}

void alarmHandler(int sig_num)
{
  cout << "smash: got an alarm" << endl;
  SmallShell::getInstance().jobList.removeFinishedJobs();
  SmallShell::getInstance().timedList.removeTimedoutJobs();
}
