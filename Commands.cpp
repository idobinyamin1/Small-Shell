#include <unistd.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <fstream>

using namespace std;
#define MAX_ARGS 20
#define MAX_CMD_LENGTH 80

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

const string WHITESPACE = " \n\r\t\f\v";

string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    memset(args[i], 0, s.length() + 1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h
bool SmallShell::isPipeCommand(const char *cmd_line)
{
  for (int i = 0; i < (int)strlen(cmd_line); i++)
  {
    if (cmd_line[i] == '|')
    {
      return true;
    }
  }
  return false;
}

bool SmallShell::isPipe2Command(const char *cmd_line)
{
  for (int i = 0; i < (int)strlen(cmd_line) - 1; i++)
  {
    if (cmd_line[i] == '|' && cmd_line[i + 1] == '&')
    {
      return true;
    }
  }
  return false;
}
bool SmallShell::isRedirectionCommand(const char *cmd_line) // > command
{
  for (int i = 0; i < (int)strlen(cmd_line); i++)
  {
    if (cmd_line[i] == '>')
    {
      return true;
    }
  }
  return false;
}
bool SmallShell::isRedirection2Command(const char *cmd_line) // >> command
{
  for (int i = 0; i < (int)strlen(cmd_line) - 1; i++)
  {
    if (cmd_line[i] == '>' && cmd_line[i + 1] == '>')
    {

      return true;
    }
  }
  return false;
}

SmallShell::SmallShell() : forground_PID(getpid()), forground_cmd(nullptr), lastDir(nullptr), prompt(DEFAULT_PROMPT), forground_job_id(-1), smash_PID(getpid())
{

  // TODO: add your implementation
}

SmallShell::~SmallShell()
{
  if (lastDir != nullptr)
  {
    delete lastDir;
  }
}

void SmallShell::changePrompt(const char *prompt_str)
{
  prompt = prompt_str;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line)
{

  string cmd_s = _trim(string(cmd_line));
  // if (_isBackgroundComamnd(cmd_line))
  // {
  //   cmd_s = cmd_s.substr(0, cmd_s.size() - 1);
  // }

  int next = cmd_s.find_first_of(" \n");
  string firstWord = (cmd_s.substr(0, next));
  if (isPipe2Command(cmd_line))
  {
    return new Pipe2Command(cmd_line); // 2 should be always checked after 1
  }
  if (isPipeCommand(cmd_line))
  {
    return new PipeCommand(cmd_line);
  }
  if (isRedirection2Command(cmd_line))
  {
    return new Redirection2Command(cmd_line); // 2 should be always checked after 1
  }
  if (isRedirectionCommand(cmd_line))
  {
    return new RedirectionCommand(cmd_line);
  }
  if (firstWord.compare("timeout") == 0)
  {
    return new TimeoutCommand(cmd_line);
  }
  if (firstWord.compare("chprompt") == 0)
  {
    return new ChpromptCommand(cmd_line);
  }
  if (firstWord.compare("showpid") == 0)
  {
    return new ShowPidCommand(cmd_line);
  }
  if (firstWord.compare("pwd") == 0)
  {
    return new GetCurrDirCommand(cmd_line);
  }
  if (firstWord.compare("cd") == 0)
  {
    return new ChangeDirCommand(cmd_line, &lastDir);
  }
  if (firstWord.compare("jobs") == 0)
  {
    return new JobsCommand(cmd_line, &SmallShell::getInstance().jobList);
  }
  if (firstWord.compare("kill") == 0)
  {
    return new KillCommand(cmd_line, &SmallShell::getInstance().jobList);
  }
  if (firstWord.compare("fg") == 0)
  {
    return new ForegroundCommand(cmd_line, &SmallShell::getInstance().jobList);
  }
  if (firstWord.compare("bg") == 0)
  {
    return new BackgroundCommand(cmd_line, &SmallShell::getInstance().jobList);
  }
  if (firstWord.compare("quit") == 0)
  {
    return new QuitCommand(cmd_line, &SmallShell::getInstance().jobList);
  }
  if (firstWord.compare("head") == 0)
  {
    return new HeadCommand(cmd_line);
  }

  return new ExternalCommand(cmd_line);

  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  // TODO: Add your implementation here
  // for example:
  Command *cmd = CreateCommand(cmd_line);
  cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

Command::Command(const char *cmd_line, bool _isTimed) : args(new char *[MAX_ARGS]), isTimed(_isTimed), init_bg(_isBackgroundComamnd(cmd_line))
{
  cmd = (char *)malloc(sizeof(char) * MAX_CMD_LENGTH);

  strcpy(cmd, cmd_line);

  cargs = _parseCommandLine(cmd_line, args);
}
Command::~Command()
{
  delete[] args;
  delete cmd;
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}
ChpromptCommand::ChpromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void ChpromptCommand::execute()
{
  if (cargs == 1)
  {
    SmallShell::getInstance().changePrompt(DEFAULT_PROMPT);
  }
  else
  {
    SmallShell::getInstance().changePrompt(args[1]);
  }
}

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void ShowPidCommand::execute()
{

  std::cout << "smash pid is " << SmallShell::getInstance().smash_PID << std::endl;
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void GetCurrDirCommand::execute()
{
  char *currDir = getcwd(NULL, 0);
  //if(currDir==NULL)cwd

  std::cout << currDir << std::endl;
  delete currDir;
}

ChangeDirCommand::ChangeDirCommand(const char *cmd_line, char **plastPwd) : BuiltInCommand(cmd_line), lastDir(plastPwd) {}
void ChangeDirCommand::execute()
{
  int result = -1;
  char *tmpDir = getcwd(NULL, 0);
  char *nextDir;

  if (cargs == 1)
  {
    return;
  }
  // if too many arguments
  if (cargs > 2)
  {
    std::cerr << "smash error: cd: too many arguments" << std::endl;
    return;
  }
  // if cd to last directory
  if (strcmp(args[1], "-") == 0)
  {
    if (*lastDir != nullptr)
    {
      nextDir = *lastDir;
    }
    else
    {
      std::cerr << "smash error: cd: OLDPWD not set" << std::endl;
      return;
    }
  }
  else
  {
    nextDir = args[1];
  }

  result = chdir(nextDir);
  //if dir changes successfuly
  if (result == 0)
  {
    delete *lastDir;
    *lastDir = tmpDir;
    // lastDir = new char[tmpDir.length()];
    // lastDir = strcpy(tmpDir);
  }
  else
  {
    perror("smash error: chdir failed");
  }
}

ExternalCommand ::ExternalCommand(const char *cmd_line) : Command(cmd_line) {}
void ExternalCommand::execute()
{

  pid_t pid;

  std::string str(cmd);
  char *temp_string = new char[str.length()];
  strcpy(temp_string, cmd);
  if (init_bg)
  {
    _removeBackgroundSign(temp_string);
  }

  pid = fork();

  if (pid == -1)
  {
    perror("smash error: fork failed");
    return;
  }

  if (pid == 0) //son
  {
    if (setpgrp() == -1)
    {
      perror("smash error: setpgrp failed");
      delete temp_string;

      exit(0);
    }
    execl("/bin/bash", "/bin/bash", "-c", temp_string, NULL);
    perror("smash error: execv failed");
    delete temp_string;

    return;
  }

  delete temp_string;

  //father
  if (!init_bg)
  {
    SmallShell::getInstance().forground_PID = pid; //updating sons pid becuse he is in forground now
    SmallShell::getInstance().forground_cmd = this;
    SmallShell::getInstance().forground_cmd->PID = pid;

    if (waitpid(pid, NULL, WUNTRACED) == -1)
    {

      perror("smash error: waitpid1 failed");

      return;
    }
    SmallShell::getInstance().forground_PID = getpid(); //updating forground pid back
  }
  else
  {
    //current commands pid, used for job list tracking
    this->PID = pid;
    if (!isTimed)
    {
      //timed commands already appear in list as their 'timeout x command' parent
      SmallShell::getInstance().jobList.removeFinishedJobs();
      SmallShell::getInstance().jobList.addJob(this);
    }
  }
}

JobsList::JobEntry::JobEntry(int JobID, pid_t pid, Command *cmd, bool isStopped, int duration) : JobID(JobID),
                                                                                                 PID(pid),
                                                                                                 command(cmd),
                                                                                                 BirthTime(time(NULL)),
                                                                                                 Duration(duration),
                                                                                                 isStopped(isStopped)
{
  if (isStopped)
  {
    state = Stopped;
    time(&StoppedTime);
  }
  else
  {
    state = Running;
    StoppedTime = 0;
  }
}

void JobsList::addJob(Command *cmd, bool isStopped /*= false*/, int duration /*= 0*/, int job_id_ /*= -1*/)
{

  JobsList *jobList = this; // &SmallShell::getInstance().jobList;
  int job_id = 1;

  if (job_id_ != -1)
  {
    job_id = job_id_;
  }
  else if (!jobList->linkedList.empty())
  {
    job_id = jobList->linkedList.back().JobID;
    job_id++;
  }

  pid_t pid = cmd->PID;
  //jobList->maxJobId++; //can delete

  // need to add it sorted
  auto one_after = jobList->linkedList.begin();
  while (one_after != jobList->linkedList.end() && one_after->JobID < job_id)
  {
    one_after++;
  }
  if (one_after != jobList->linkedList.end())
  {
    jobList->linkedList.insert(one_after, JobEntry(job_id, pid, cmd, isStopped, duration));
  }
  else
  {
    jobList->linkedList.push_back(JobEntry(job_id, pid, cmd, isStopped, duration));
  }
}

void JobsList::printJobsList()
{
  //print format:
  //[<job-id>] <command> : <process id> <seconds elapsed> (stopped)
  //if command was sent by user as background job, add the original
  // & symbol
  time_t runningTime;
  string stopped = "";
  string command = "";
  string line = "";
  int pid = 0;
  removeFinishedJobs();

  for (list<JobEntry>::iterator it = linkedList.begin(); it != linkedList.end(); it++)
  {
    stopped = it->isStopped ? "(stopped)" : "";
    command = (it->command->cmd);
    // command = it->command->init_bg ? command + "&" : command;
    runningTime = difftime(time(NULL), it->BirthTime); // check returned time is in seconds
    pid = (int)it->PID;
    line = "[" + to_string(it->JobID) + "] " + command + " : " + to_string(pid) + ' ' + to_string(runningTime) + " secs " + stopped;
    // line.append("[").append(to_string(it->JobID)).append("] ");
    // line.append(command).append(" : ").append(to_string(pid));
    cout << line << std::endl;
    // cout << '[' << it->JobID << "] " << command << " : " << it->PID << ' ' << runningTime + " secs " << stopped << std::endl;
  }
}
void JobsList::removeFinishedJobs()
{

  if (getpid() != SmallShell::getInstance().smash_PID)
  {
    return;
  }
  auto it = linkedList.begin();
  while (it != linkedList.end())
  {

    if (it->isStopped)
    {
      it++;
      continue;
    }
    int error = waitpid(it->PID, NULL, WNOHANG);
    if (error == -1)
    {
      perror("smash error: waitpid2 failed");
      return;
    }

    else if (error != 0)
    {
      it = linkedList.erase(it);
    }
    else
    {
      it++;
    }
  }

  // maxJobId = linkedList.back().JobID;
}
void JobsList::removeTimedoutJobs()
{
  auto it = linkedList.begin();
  int runningTime = 0;
  int nextAlarm = 0; //time remaining to next alarm
  bool runningbg = false;
  bool runningfg = false;
  int error = 0;
  pid_t pid = 0;
  Command *exChild = nullptr;

  while (it != linkedList.end())
  {

    runningTime = difftime(time(NULL), it->BirthTime);
    if (nextAlarm == 0 && (it->Duration > runningTime))
    {
      nextAlarm = (it->Duration - runningTime);
    }

    if (runningTime >= it->Duration)
    {
      exChild = dynamic_cast<TimeoutCommand *>(it->command)->exChild;
      pid = exChild->PID; //it->command->PID
      runningbg = SmallShell::getInstance().jobList.findJobByPid(pid);
      runningfg = (SmallShell::getInstance().forground_PID == pid);
      if (runningbg || runningfg)
      {

        SmallShell::getInstance().jobList.removeJobByPid(pid);
        // exChild = dynamic_cast<TimeoutCommand *>(it->command)->exChild;

        error = waitpid(pid, NULL, WNOHANG);
        if (error == 0)
        {
          if (kill(pid, 9) == -1)
          {
            perror("smash error: kill failed");
            return;
          }
          std::cout << "smash: " << it->command->cmd << " timed out!" << std::endl;
        }
      }
      it = linkedList.erase(it);
    }
    else
    {
      if (nextAlarm > (it->Duration - runningTime))
      {
        nextAlarm = (it->Duration - runningTime);
      }

      it++;
    }
  }
  //set up next alarm
  alarm(nextAlarm);
  return;
}
void JobsList::removeJobByPid(pid_t jobPid)
{
  auto it = linkedList.begin();
  while (it != linkedList.end())
  {

    if (it->PID == jobPid)
    {
      linkedList.erase(it);
      return;
    }
    it++;
  }
}
bool JobsList::findJobByPid(pid_t jobPid)
{
  auto it = linkedList.begin();
  while (it != linkedList.end())
  {
    if (it->PID == jobPid)
    {
      return true;
    }
    it++;
  }
  return false;
}
JobsCommand ::JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}
void JobsCommand::execute()
{
  jobs->printJobsList();
}

KillCommand::KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}
void KillCommand::execute()
{

  //checking argunmts are valid
  if (cargs != 3 || args[1][0] != '-')
  {
    std::cerr << "smash error: kill: invalid arguments" << std::endl;
    return;
  }

  for (int i = 0; i < (int)strlen(args[2]); i++)
  {
    if (args[2][i] > '9' || args[2][i] < '0')
    {
      if (i == 0 && args[2][i] == '-')
      {
        continue;
      }
      std::cerr << "smash error: kill: invalid arguments" << std::endl;
      return;
    }
  }

  for (int i = 0; i < (int)strlen(args[1]); i++)
  {
    if (args[1][i] > '9' || args[1][i] < '0')
    {
      if (i == 0 && args[1][i] == '-')
      {
        continue;
      }
      std::cerr << "smash error: kill: invalid arguments" << std::endl;
      return;
    }
  }
  // find the job

  for (auto it = jobs->linkedList.begin(); it != jobs->linkedList.end(); it++)
  {
    if (it->JobID == stoi(args[2]))
    {
      //job found
      if (kill(it->PID, -stoi(args[1])) == -1)
      {
        perror("smash error: kill failed");
        return;
      }
      std::cout << "signal number " << -stoi(args[1]) << " was sent to pid " << it->PID << std::endl;

      if (-stoi(args[1]) == 18) //18=SIGCONT
      {
        it->isStopped = false;
      }
      //if (-stoi(args[1]) == 19) // 19=SIGSTOP
      // {
      // it->isStopped = true;
      //}
      if (-stoi(args[1]) == 9) // 19=SIGKILL
      {

        it->isStopped = false;
        //   jobs->linkedList.erase(it);
      }

      return;
    }
  }
  //job didnt found
  std::cerr << "smash error: kill: job-id " << args[2] << " does not exist" << std::endl;
  return;
}
ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}
void ForegroundCommand::execute()
{
  if (cargs > 2)
  {
    std::cerr << "smash error: fg: invalid arguments" << std::endl;
    return;
  }
  // need to return the job with maximal job id (should be the last on list)
  if (cargs == 1)
  {
    if (jobs->linkedList.empty())
    {
      std::cerr << "smash error: fg: jobs list is empty" << std::endl;
      return;
    }
    JobsList::JobEntry to_forground = jobs->linkedList.back();
    std::cout << to_forground.command->cmd << " : " << to_forground.PID << std::endl;
    SmallShell::getInstance().forground_cmd = SmallShell::getInstance().CreateCommand(to_forground.command->cmd);
    SmallShell::getInstance().forground_cmd->PID = to_forground.PID;
    SmallShell::getInstance().forground_PID = to_forground.PID;
    SmallShell::getInstance().forground_job_id = to_forground.JobID;
    jobs->linkedList.pop_back();
    if (kill(to_forground.PID, 18) == -1)
    {
      perror("smash error: kill failed");

      return;
    }

    if (waitpid(to_forground.PID, NULL, WUNTRACED) == -1)
    {
      perror("smash error: waitpid3 failed");
    }

    return;
  }
  // need to get to foreground specific job (cargs==2)

  // checking arguments are valid first

  if (args[1][0] != '-' && (args[1][0] < '0' || args[1][0] > '9'))
  {
    std::cerr << "smash error: fg: invalid arguments" << std::endl;
    return;
  }
  for (int i = 1; i < (int)strlen(args[1]); i++)
  {
    if (args[1][i] < '0' || args[1][i] > '9')
    {
      std::cerr << "smash error: fg: invalid arguments" << std::endl;
      return;
    }
  }
  for (auto it = jobs->linkedList.begin(); it != jobs->linkedList.end(); it++)
  {
    if (it->JobID == stoi(args[1]))
    {
      JobsList::JobEntry to_forground = *it;

      SmallShell::getInstance().forground_cmd = SmallShell::getInstance().CreateCommand(to_forground.command->cmd);
      SmallShell::getInstance().forground_cmd->PID = to_forground.PID;
      SmallShell::getInstance().forground_PID = to_forground.PID;
      SmallShell::getInstance().forground_job_id = to_forground.JobID;

      jobs->linkedList.erase(it);
      if (kill(to_forground.PID, 18) == -1)
      {
        perror("smash error: kill failed");
        return;
      }

      std::cout << to_forground.command->cmd << " : " << to_forground.PID << std::endl;
      if (waitpid(to_forground.PID, NULL, WUNTRACED) == -1)
      {
        perror("smash error: waitpid4 failed");
      }
      return;
    }
  }

  // job want found
  std::cerr << "smash error: fg: job-id " << args[1] << " does not exist" << std::endl;
  return;
}

BackgroundCommand::BackgroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}
void BackgroundCommand::execute()
{

  if (cargs > 2)
  {
    std::cerr << "smash error: bg: invalid arguments" << std::endl;
    return;
  }

  // specefic job needed
  if (cargs == 2)
  {

    // checking arguments are valid first
    if (args[1][0] != '-' && (args[1][0] > '9' || args[1][0] < '0'))
    {
      std::cerr << "smash error: bg: invalid arguments" << std::endl;
      return;
    }
    for (int i = 1; i < (int)strlen(args[1]); i++)
    {
      if (args[1][i] > '9' || args[1][i] < '0')
      {
        std::cerr << "smash error: bg: invalid arguments" << std::endl;
        return;
      }
    }

    for (auto it = jobs->linkedList.begin(); it != jobs->linkedList.end(); it++)
    {
      if (it->JobID == stoi(args[1]))
      {
        if (it->isStopped == false)
        {
          std::cerr << "smash error: bg: job-id " << it->JobID << " is already running in the background" << std::endl;
          return;
        }
        it->isStopped = false;
        std::cout << it->command->cmd << " : " << it->PID << std::endl;
        if (kill(it->PID, 18) == -1)
        {
          perror("smash error: kill failed");
          return;
        }
        return;
      }
    }
    // not founded
    std::cerr << "smash error: bg: job-id " << stoi(args[1]) << " does not exist" << std::endl;
  }

  // the maximal stopped job is needed
  if (cargs == 1)
  {

    auto max_job = jobs->linkedList.begin();
    int max_idx = -1;

    for (auto it = jobs->linkedList.begin(); it != jobs->linkedList.end(); it++)
    {
      if (it->isStopped == true)
      {
        max_job = it;
        max_idx = it->JobID;
      }
    }
    if (max_idx == -1)
    {
      std::cerr << "smash error: bg: there is no stopped jobs to resume" << std::endl;
      return;
    }
    std::cout << max_job->command->cmd << " : " << max_job->PID << std::endl;
    max_job->isStopped = false;
    if (kill(max_job->PID, 18) == -1)
    {
      perror("smash error: kill failed");
      return;
    }
  }
}
QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}
void QuitCommand::execute()
{

  bool killFlag = false;
  if (cargs >= 2 && strcmp("kill", args[1]) == 0)
  {
    killFlag = true;
  }
  if (cargs < 2 || (cargs >= 2 && !killFlag))
  {
    exit(0);
  }
  else
  {
    jobs->removeFinishedJobs();
    std::cout << "smash: sending SIGKILL signal to " << jobs->linkedList.size() << " jobs:" << std::endl;
    for (auto it = jobs->linkedList.begin(); it != jobs->linkedList.end(); it++)
    {
      cout << it->PID << ": " << it->command->cmd << endl;
      if (kill(it->PID, 9) == -1)
      {
        perror("smash error: kill failed");

        return;
      }
    }
    exit(0);
  }
}
PipeCommand::PipeCommand(const char *cmd_line) : Command(cmd_line) //pipe command constructor
{
  int pipe_idx = 0;
  for (int i = 0; i < (int)strlen(cmd_line); i++)
  {
    if (cmd_line[i] == '|')
    {
      pipe_idx = i;
    }
  }
  char command1[pipe_idx + 1];
  char command2[strlen(cmd_line) - pipe_idx + 1];
  strcpy(command1, cmd_line);
  command1[pipe_idx] = '\0';
  strcpy(command2, cmd_line + pipe_idx + 1);

  pipe_commands[0] = std::string(command1);

  pipe_commands[1] = std::string(command2);
}
void PipeCommand::execute()
{
  int fd[2];
  if (pipe(fd) == -1)
  {
    perror("smash error: pipe failed");
    return;
  }
  pid_t writer_pid = fork();
  if (writer_pid == -1)
  {
    perror("smash error: fork failed");
    return;
  }

  // son #1 (writer)
  if (writer_pid == 0)
  {
    if (setpgrp() == -1)
    {
      perror("smash error: setpgrp failed");
      exit(0);
    }
    if (dup2(fd[1], 1) == -1)
    {
      perror("smash error: dup2 failed");
      exit(0);
    }

    if (close(fd[0]) == -1) //closing pipe
    {
      perror("smash error: close failed");
      exit(0);
    }
    if (close(fd[1]) == -1) //closing pipe
    {
      perror("smash error: close failed");
      exit(0);
    }

    Command *cmd = SmallShell::getInstance().CreateCommand(pipe_commands[0].c_str());
    cmd->execute();
    exit(0);
  }
  //(reader)
  pid_t reader_pid = fork();
  if (reader_pid == -1)
  {
    perror("smash error: fork failed");
    return;
  }
  // son #2 (reader)
  if (reader_pid == 0)
  {
    if (setpgrp() == -1)
    {
      perror("smash error: setpgrp failed");
      exit(0);
    }

    if (dup2(fd[0], 0) == -1)
    {
      perror("smash error: dup2 failed");
      exit(0);
    }
    if (close(fd[1]) == -1) //closing pipe
    {
      perror("smash error: close failed");
      exit(0);
    }
    if (close(fd[0]) == -1) //closing pipe
    {
      perror("smash error: close failed");
      exit(0);
    }
    Command *cmd = SmallShell::getInstance().CreateCommand(pipe_commands[1].c_str());
    cmd->execute();
    exit(0);
  }
  //father
  if (close(fd[1]) == -1) //closing pipe
  {
    perror("smash error: close failed");
    exit(0);
  }
  if (close(fd[0]) == -1) //closing pipe
  {
    perror("smash error: close failed");
    exit(0);
  }

  if (waitpid(writer_pid, NULL, WUNTRACED) == -1)
  {
    perror("smash error: waitpid5 failed");
    exit(0);
  }
  if (waitpid(reader_pid, NULL, WUNTRACED) == -1)
  {
    perror("smash error: waitpid6 failed");
    exit(0);
  }
}

Pipe2Command::Pipe2Command(const char *cmd_line) : Command(cmd_line) //pipe and &command constructor
{
  int pipe_idx = 0;
  for (int i = 0; i < (int)strlen(cmd_line) - 1; i++)
  {
    if (cmd_line[i] == '|' && cmd_line[i + 1] == '&')
    {
      pipe_idx = i;
    }
  }
  char command1[pipe_idx + 1];
  strcpy(command1, cmd_line);
  command1[pipe_idx] = '\0';

  char command2[strlen(cmd_line) - pipe_idx + 2];
  strcpy(command2, cmd_line + pipe_idx + 2);

  pipe_commands[0] = std::string(command1);

  pipe_commands[1] = std::string(command2);
}

void Pipe2Command::execute()
{
  int fd[2];
  if (pipe(fd) == -1)
  {
    perror("smash error: pipe failed");
    return;
  }
  pid_t writer_pid = fork();
  if (writer_pid == -1)
  {
    perror("smash error: fork failed");
    return;
  }

  // son #1 (writer)
  if (writer_pid == 0)
  {
    if (setpgrp() == -1)
    {
      perror("smash error: setpgrp failed");
      exit(0);
    }
    if (dup2(fd[1], 2) == -1)
    {
      perror("smash error: dup2 failed");
      exit(0);
    }

    if (close(fd[0]) == -1) //closing reading channel
    {
      perror("smash error: close failed");
      exit(0);
    }
    if (close(fd[1]) == -1) //closing reading channel
    {
      perror("smash error: close failed");
      exit(0);
    }

    Command *cmd = SmallShell::getInstance().CreateCommand(pipe_commands[0].c_str());
    cmd->execute();
    exit(0);
  }
  //(reader)
  pid_t reader_pid = fork();
  if (reader_pid == -1)
  {
    perror("smash error: fork failed");
    return;
  }
  // son #2 (reader)
  if (reader_pid == 0)
  {
    if (setpgrp() == -1)
    {
      perror("smash error: setpgrp failed");
      exit(0);
    }

    if (dup2(fd[0], 0) == -1)
    {
      perror("smash error: dup2 failed");
      exit(0);
    }
    if (close(fd[1]) == -1) //closing writing channel
    {
      perror("smash error: close failed");
      exit(0);
    }
    if (close(fd[0]) == -1) //closing reading channel
    {
      perror("smash error: close failed");
      exit(0);
    }
    Command *cmd = SmallShell::getInstance().CreateCommand(pipe_commands[1].c_str());
    cmd->execute();
    exit(0);
  }
  //father
  if (close(fd[1]) == -1) //closing pipe
  {
    perror("smash error: close failed");
    exit(0);
  }
  if (close(fd[0]) == -1) //closing pipe
  {
    perror("smash error: close failed");
    exit(0);
  }

  if (waitpid(writer_pid, NULL, WUNTRACED) == -1)
  {
    perror("smash error: waitpid6 failed");
    exit(0);
  }
  if (waitpid(reader_pid, NULL, WUNTRACED) == -1)
  {
    perror("smash error: waitpid7 failed");
    exit(0);
  }
}
RedirectionCommand::RedirectionCommand(const char *cmd_line) : Command(cmd_line)
{
  int symb_idx = 0;
  for (int i = 0; i < (int)strlen(cmd_line); i++)
  {
    if (cmd_line[i] == '>')
    {
      symb_idx = i;
    }
  }

  char command1[symb_idx + 1];
  strcpy(command1, cmd_line);
  command1[symb_idx] = '\0';
  //add trim
  char command2[strlen(cmd_line) - symb_idx + 1];
  strcpy(command2, cmd_line + symb_idx + 1);

  red_commands[1] = std::string(command2); // file name

  red_commands[0] = std::string(command1);
}
void RedirectionCommand::execute()
{

  std::string file_name = _trim(red_commands[1]);
  int fd = open(file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);

  if (fd == -1)
  {
    perror("smash error: open failed");
    return;
  }
  // saving temp standart output so we can recreate it later

  int temp;
  temp = dup(1);

  if (temp == -1)
  {

    perror("smash error: dup failed");

    return;
  }

  if (dup2(fd, 1) == -1) // redricting standart outpot to file
  {
    perror("smash error: dup2 failed");
    return;
  }
  //closing file

  if (close(fd) == -1)
  {

    perror("smash error: close failed");

    return;
  }

  // running the command
  Command *cmd = SmallShell::getInstance().CreateCommand(red_commands[0].c_str());
  cmd->execute();

  //redricting to standart outpot again
  int err = dup2(temp, 1);

  if (err == -1)
  {
    perror("smash error: dup2 failed");

    return;
  }

  //closing the temp standart outpot
  if (close(temp) == -1)
  {

    perror("smash error: close failed");
    return;
  }
}
Redirection2Command::Redirection2Command(const char *cmd_line) : Command(cmd_line)
{
  int symb_idx = 0;
  for (int i = 0; i < (int)strlen(cmd_line) - 1; i++)
  {
    if (cmd_line[i] == '>' && cmd_line[i + 1] == '>')
    {
      symb_idx = i;
    }
  }

  char command1[symb_idx + 1];
  strcpy(command1, cmd_line);
  command1[symb_idx] = '\0';

  char command2[strlen(cmd_line) - symb_idx + 1];
  strcpy(command2, cmd_line + symb_idx + 2);

  red_commands[1] = std::string(command2); // file name

  red_commands[0] = std::string(command1);
}
void Redirection2Command::execute()
{
  int fd;
  std::string file_name = _trim(red_commands[1]);
  if ((fd = open(file_name.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666)) == -1)
  {
    perror("smash error: open failed");
    return;
  }
  // saving temp standart ouppot so we can recreate it later

  int temp;
  temp = dup(1);
  if (temp == -1)
  {

    perror("smash error: dup failed");

    return;
  }

  if (dup2(fd, 1) == -1) // redricting standart outpot to file
  {
    perror("smash error: dup2 failed");
    return;
  }
  //closing file

  if (close(fd) == -1)
  {

    perror("smash error: close failed");

    return;
  }

  // running the command
  Command *cmd = SmallShell::getInstance().CreateCommand(red_commands[0].c_str());
  cmd->execute();
  //redricting to standart outpot again
  int err = dup2(temp, 1);

  if (err == -1)
  {
    perror("smash error: dup2 failed");

    return;
  }

  //closing the temp standart outpot
  if (close(temp) == -1)
  {

    perror("smash error: close failed");
    return;
  }
}

TimeoutCommand::TimeoutCommand(const char *cmd_line) : Command(cmd_line) {} //TimeoutCommand command constructor
void TimeoutCommand::execute()
{
  if (cargs < 3 || stoi(args[1]) <= 0)
  {
    std::cerr << "smash error: timeout: invalid arguments" << std::endl;
    return;
  }

  string read_command = "";
  // bool initbg = false;
  for (int i = 2; i < cargs; i++)
  {
    read_command += args[i];
    read_command += " ";
  }
  int duration = stoi(args[1]);
  // if (init_bg)
  // {
  //   unsigned int idx = read_command.find_last_not_of(WHITESPACE);
  //   if (read_command[idx] == '&')
  //   {
  //     read_command = read_command.substr(0, idx);
  //   }
  // }

  exChild = SmallShell::getInstance().CreateCommand(read_command.c_str());
  exChild->isTimed = true;
  //set alarm
  unsigned int nextAlarm = alarm(duration);
  if (nextAlarm != 0 && (int)nextAlarm < duration)
  {
    alarm(nextAlarm);
  }

  SmallShell::getInstance().timedList.addJob(this, false, duration);
  // SmallShell::getInstance().timedList.addJob(exChild, false, duration);
  exChild->execute();
  if (init_bg)
  {
    this->PID = exChild->PID;

    SmallShell::getInstance().jobList.removeFinishedJobs();
    SmallShell::getInstance().jobList.addJob(this);
  }
}

HeadCommand::HeadCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void HeadCommand::execute()
{
  int fileArg = 1;
  int N = 10;
  size_t byts_written = 0;

  if (cargs == 1)
  {
    std::cerr << "smash error: head: not enough arguments" << std::endl;

    return;
  }
  if (cargs > 2 && args[1][0] == '-')
  {
    N = -(stoi(args[1]));
    fileArg = 2;
  }

  // std::ifstream fs;
  // fs.open(args[fileArg], std::ifstream::in);
  std::ifstream fs(args[fileArg]);
  if (fs.fail()) // (fs.rdstate() & ifstream::failbit) != 0) //!(fs.is_open()))
  {
    perror("smash error: open failed");
    return;
  }

  std::string line;
  for (int i = 0; i < N; i++)
  {
    line.clear();
    if (!std::getline(fs, line))
    {
      if (fs.eof()) //(fs.rdstate() & ifstream::eofbit) != 0) //ios_base::eofbit == true)
      {
        break;
      }
      else
      {
        perror("smash error: read failed");
      }
    }
    if (!fs.eof())
    {
      line.append(1, '\n');
    }
    byts_written = write(1, line.c_str(), line.size());
    if (byts_written != line.size())
    {
      fs.close();
      perror("smash error: write failed");
      return;
    }
  }
  fs.close();
  return;
}
