
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <list>
#include <fcntl.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define DEFAULT_PROMPT "smash"

class Command
{
  

public:
  char *cmd;
  char **args;
  int cargs;
  pid_t PID;
  bool isTimed;
  const bool init_bg; //if initialized as background
  Command(const char *cmd_line,bool isTimed=false);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  // virtual void cleanup();
};

class BuiltInCommand : public Command
{
public:
  BuiltInCommand(const char *cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command
{
public:
  ExternalCommand(const char *cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command
{
  // TODO: Add your data members
public:
  std::string pipe_commands[2];
  PipeCommand(const char *cmd_line) ;
  virtual ~PipeCommand() {}
  void execute() override;
};
class Pipe2Command : public Command
{
  // TODO: Add your data members
public:
  std::string pipe_commands[2];
  Pipe2Command(const char *cmd_line) ;
  virtual ~Pipe2Command() {}
  void execute() override;
};

class RedirectionCommand : public Command
{
  // TODO: Add your data members
public:
  std::string red_commands[2];// red_commands[1] will be file name
  explicit RedirectionCommand(const char *cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};
class Redirection2Command : public Command
{
  // TODO: Add your data members
public:
  std::string red_commands[2];// red_commands[1] will be file name
  explicit Redirection2Command(const char *cmd_line);
  virtual ~Redirection2Command() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand
{
  // TODO: Add your data members public:
public:
  char **lastDir;
  ChangeDirCommand(const char *cmd_line, char **plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand
{
public:
  GetCurrDirCommand(const char *cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
public:
  ShowPidCommand(const char *cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};
class JobsList;

class QuitCommand : public BuiltInCommand
{
  // TODO: Add your data members public:
  public:
  JobsList *jobs;
  QuitCommand(const char *cmd_line, JobsList *jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class JobsList
{
  // List is ordered by jobID, and order should
  // be maintained for each push/pull/insert/remove

  enum State
  {
    Stopped,
    Running,
    Finished
  };

public:
  class JobEntry
  {
    // TODO: Add your data members
  public:
    const int JobID;
    pid_t PID;
    Command *command;
    State state;
    time_t StoppedTime;
    time_t BirthTime;
    int Duration;
    bool isStopped;
    JobEntry(int JobID,pid_t PID,Command *cmd, bool isStopped, int duration=0);
    
  };

private:
  int maxJobId = 0;
  
  // TODO: Add your data members
public:

  std::list<JobEntry> linkedList;
  
  JobsList()=default;
  
  
  ~JobsList()=default;
  void addJob(Command *cmd, bool isStopped= false, int duration = 0,int job_id_ =-1);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  void removeTimedoutJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  void removeJobByPid(pid_t jobPid);
  bool findJobByPid(pid_t jobPid);
  JobEntry *getLastJob(int *lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand
{
  
public:
  JobsList* jobs;
  JobsCommand(const char *cmd_line, JobsList *jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
   JobsList *jobs;
  KillCommand(const char *cmd_line, JobsList *jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  
  ForegroundCommand(const char *cmd_line, JobsList *jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
   JobsList *jobs;
};

class BackgroundCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
   JobsList *jobs;
  BackgroundCommand(const char *cmd_line, JobsList *jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
  
};

class TimeoutCommand : public Command
{
public:
  Command* exChild=NULL;
  TimeoutCommand(const char *cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class HeadCommand : public BuiltInCommand
{
public:
  HeadCommand(const char *cmd_line);
  virtual ~HeadCommand() {}
  void execute() override;
};

class SmallShell
{

private:
  SmallShell();

public:
 
  pid_t forground_PID;
  Command* forground_cmd;
  JobsList jobList;
  JobsList timedList;
  char *lastDir;
  std::string prompt;
   int forground_job_id;
   pid_t smash_PID;
  Command *CreateCommand(const char *cmd_line);
  SmallShell(SmallShell const &) = delete;     //d disable copy ctor
  void operator=(SmallShell const &) = delete; // disable = operator
  static SmallShell &getInstance()             // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char *cmd_line);
  void changePrompt(const char *cmd_line);
  bool isPipeCommand(const char *cmd_line);
  bool isPipe2Command(const char *cmd_line);
  bool isRedirectionCommand(const char *cmd_line);
  bool isRedirection2Command(const char *cmd_line);

};

class ChpromptCommand : public BuiltInCommand
{
public:
  ChpromptCommand(const char *str);
  virtual ~ChpromptCommand() {}
  void execute() override;
};

#endif //SMASH_COMMAND_H_
