
/*
  NUI3 - C++ cross-platform GUI framework for OpenGL based applications
  Copyright (C) 2002-2003 Sebastien Metrot

  licence: see nui3/LICENCE.TXT
*/

#include "nui.h"
#include "MainWindow.h"
#include "Application.h"
#include "nuiCSS.h"
#include "nuiVBox.h"

using namespace lldb;

/*
 * MainWindow
 */

MainWindow::MainWindow(const nglContextInfo& rContextInfo, const nglWindowInfo& rInfo, bool ShowFPS, const nglContext* pShared )
  : mEventSink(this),
    nuiMainWindow(rContextInfo, rInfo, pShared, nglPath(ePathCurrent))
{
  //mClearBackground = false;
  SetDebugMode(true);
  EnableAutoRotation(true);

#ifdef _DEBUG_
  nglString t = "DEBUG";
#else
  nglString t = "RELEASE";
#endif
  nuiLabel* pLabel = new nuiLabel(t);
  pLabel->SetPosition(nuiBottom);
  AddChild(pLabel);

  LoadCSS(_T("rsrc:/css/main.css"));

  nuiWidget* pDebugger = nuiBuilder::Get().CreateWidget("Debugger");
  NGL_ASSERT(pDebugger);
  AddChild(pDebugger);

  mpThreads = (nuiTreeView*)pDebugger->SearchForChild("Threads", true);
  NGL_ASSERT(mpThreads);

  mpVariables = (nuiTreeView*)pDebugger->SearchForChild("Variables", true);
  NGL_ASSERT(mpVariables);
  mpVariables->EnableSubElements(2);

  nuiButton* pStart = (nuiButton*)pDebugger->SearchForChild("StartStop", true);
  nuiButton* pPause = (nuiButton*)pDebugger->SearchForChild("Pause", true);
  nuiButton* pContinue = (nuiButton*)pDebugger->SearchForChild("Continue", true);
  //nuiButton* pStop = (nuiButton*)pDebugger->SearchForChild("Stop", true);

  mEventSink.Connect(pStart->Activated, &MainWindow::OnStart);
  mEventSink.Connect(pPause->Activated, &MainWindow::OnPause);
  mEventSink.Connect(pContinue->Activated, &MainWindow::OnContinue);

  mEventSink.Connect(mpThreads->SelectionChanged, &MainWindow::OnThreadSelectionChanged);
}

MainWindow::~MainWindow()
{
}


#define FONT_SIZE 35

void MainWindow::OnCreation()
{
  //EnableAutoRotation(false);
  //SetRotation(90);
  //SetState(nglWindow::eMaximize);

}

void MainWindow::OnClose()
{
  if (GetNGLWindow()->IsInModalState())
    return;
  
  
  App->Quit();
}

bool MainWindow::LoadCSS(const nglPath& rPath)
{
  nglIStream* pF = rPath.OpenRead();
  if (!pF)
  {
    NGL_OUT(_T("Unable to open CSS source file '%ls'\n"), rPath.GetChars());
    return false;
  }

  nuiCSS* pCSS = new nuiCSS();
  bool res = pCSS->Load(*pF, rPath);
  delete pF;

  if (res)
  {
    nuiMainWindow::SetCSS(pCSS);
    return true;
  }

  NGL_OUT(_T("%ls\n"), pCSS->GetErrorString().GetChars());

  delete pCSS;
  return false;
}

void MyLogOutputCallback(const char * str, void *baton)
{
  printf("LLDB> %s", str);
}

const char* GetStateName(StateType s)
{
  switch (s)
  {
    case eStateInvalid:
      return "eStateInvalid";
      break;
    case eStateUnloaded:
      return "eStateUnloaded";
      break;
    case eStateConnected:
      return "eStateConnected";
      break;
    case eStateAttaching:
      return "eStateAttaching";
      break;
    case eStateLaunching:
      return "eStateLaunching";
      break;
    case eStateStopped:
      return "eStateStopped";
      break;
    case eStateRunning:
      return "eStateRunning";
      break;
    case eStateStepping:
      return "eStateStepping";
      break;
    case eStateCrashed:
      return "eStateCrashed";
      break;
    case eStateDetached:
      return "eStateDetached";
      break;
    case eStateExited:
      return "eStateExited";
      break;
    case eStateSuspended:
      return "eStateSuspended";
      break;
  }

  return "WTF??";
}

const char* GetStopReasonName(StopReason reason)
{
  switch (reason)
  {
    case eStopReasonInvalid:
      return "Invalid";
    case eStopReasonNone:
      return "None";
    case eStopReasonTrace:
      return "Trace";
    case eStopReasonBreakpoint:
      return "Breakpoint";
    case eStopReasonWatchpoint:
      return "Watchpoint";
    case eStopReasonSignal:
      return "Signal";
    case eStopReasonException:
      return "Exception";
    case eStopReasonExec:
      return "Exec";
    case eStopReasonPlanComplete:
      return "Plan complete";
    case eStopReasonThreadExiting:
      return "Thread exiting";
  }

  return "WTF";
}

void PrintDebugState(SBProcess& rProcess)
{
  int threadcount = rProcess.GetNumThreads();
  rProcess.Stop();
  for (int i = 0; i < threadcount; i++)
  {
    SBThread thread(rProcess.GetThreadAtIndex(i));
    int framecount = thread.GetNumFrames();
    StopReason reason = thread.GetStopReason();
    SBStream st;
    bool res = thread.GetStatus (st);
    NGL_OUT("Thread %d (%s) %d frames (stop reason: %s)\n", i, thread.GetName(), framecount, GetStopReasonName(reason));
    for (int j = 0; j < framecount; j++)
    {
      SBFrame frame(thread.GetFrameAtIndex(j));
      NGL_OUT("\t%d - %s\n", j, frame.GetFunctionName());
    }
//    printf("State:\n%s\n", st.GetData());
  }
}


bool BPCallback (void *baton,
                 SBProcess &process,
                 SBThread &thread,
                 SBBreakpointLocation &location)
{
  PrintDebugState(process);
  return true;
}



void MainWindow::OnStart(const nuiEvent& rEvent)
{
  nglPath p("/Users/meeloo/work/build/Noodlz-cwmoeooodusxmealpbjorzgwidxy/Build/Products/Default/YaLiveD.app");
//  TestMain2(p.GetChars());
//  return;

  DebuggerContext& rContext(GetDebuggerContext());

  StateType state = rContext.mProcess.GetState();
  if (state == eStateRunning)
  {
    rContext.mProcess.Kill();
    rContext.mProcess.Clear();
    rContext.mDebugger.Clear();
    return;
  }

  // Create a debugger instance so we can create a target
  const char *channel = "lldb";
  const char *categories[] =
  {
    //strm->Printf("Logging categories for 'lldb':\n"
//     "all", // - turn on all available logging categories\n"
     "api", // - enable logging of API calls and return values\n"
     "break", // - log breakpoints\n"
//     "commands", // - log command argument parsing\n"
//     "default", // - enable the default set of logging categories for liblldb\n"
//     "dyld", // - log shared library related activities\n"
//     "events", // - log broadcaster, listener and event queue activities\n"
//     "expr", // - log expressions\n"
//     "object", // - log object construction/destruction for important objects\n"
//     "module", // - log module activities such as when modules are created, detroyed, replaced, and more\n"
     "process", // - log process events and activities\n"
//     "script", // - log events about the script interpreter\n"
     "state",  //- log private and public process state changes\n"
     "step", // - log step related activities\n"
     "symbol", // - log symbol related issues and warnings\n"
     "target", // - log target events and activities\n"
     "thread", // - log thread events and activities\n"
//     "types", // - log type system related activities\n"
//     "unwind", // - log stack unwind activities\n"
//     "verbose", // - enable verbose logging\n"
//     "watch", // - log watchpoint related activities\n");
    NULL
  };
  //rContext.mDebugger.SetLoggingCallback(MyLogOutputCallback, NULL);
//  rContext.mDebugger.EnableLog(channel, categories);
  //rContext.mDebugger.SetAsync(false);

  mpDebuggerEventLoop = new nglThreadDelegate(nuiMakeDelegate(this, &MainWindow::Loop));
  mpDebuggerEventLoop->Start();


  if (rContext.mDebugger.IsValid())
  {
    // Create a target using the executable.
    rContext.mTarget = rContext.mDebugger.CreateTargetWithFileAndArch (p.GetChars(), "i386");
    if (rContext.mTarget.IsValid())
    {
      static SBBreakpoint breakpoint = rContext.mTarget.BreakpointCreateByName("main");
      //breakpoint.SetCallback(BPCallback, 0);

      SBError error;
#if 1
      rContext.mProcess = rContext.mTarget.LaunchSimple (NULL, NULL, "~/");
#else
      const char **argv = NULL;
      const char **envp = NULL;
      const char *working_directory = "~/";
      char *stdin_path = NULL;
      char *stdout_path = NULL;
      char *stderr_path = NULL;
      uint32_t launch_flags = 0; //eLaunchFlagDebug;
      bool stop_at_entry = false;
      SBListener listener = rContext.mDebugger.GetListener();
      rContext.mProcess = rContext.mTarget.Launch (listener,
                     argv,
                     envp,
                     stdin_path,
                     stdout_path,
                     stderr_path,
                     working_directory,
                     launch_flags,
                     stop_at_entry,
                     error);
#endif

    }
  }

  //UpdateProcess();
  NGL_OUT("Start\n");
  
}

void MainWindow::OnPause(const nuiEvent& rEvent)
{
  DebuggerContext& rContext(GetDebuggerContext());
  SBError error;
  StateType state = rContext.mProcess.GetState();
  NGL_OUT("State on pause: %s\n", GetStateName(state));
  error = rContext.mProcess.Stop();
}

void MainWindow::OnContinue(const nuiEvent& rEvent)
{
  DebuggerContext& rContext(GetDebuggerContext());
  SBError error;
  StateType state = rContext.mProcess.GetState();
  NGL_OUT("State on continue: %s\n", GetStateName(state));
  error = rContext.mProcess.Continue();
}

void MainWindow::Loop()
{
  DebuggerContext& rContext(GetDebuggerContext());
  while (true)
	{
    if (rContext.mProcess.IsValid())
    {
      SBEvent evt;
      rContext.mDebugger.GetListener().WaitForEvent(UINT32_MAX, evt);
      StateType state = SBProcess::GetStateFromEvent(evt);

      if (SBProcess::GetRestartedFromEvent (evt))
      {
        size_t num_reasons = SBProcess::GetNumRestartedReasonsFromEvent(evt);
        if (num_reasons > 0)
        {
          // FIXME: Do we want to report this, or would that just be annoyingly chatty?
          if (num_reasons == 1)
          {
            char message[1024];
            const char *reason = SBProcess::GetRestartedReasonAtIndexFromEvent (evt, 0);
            ::snprintf (message, sizeof(message), "Process %lld stopped and restarted: %s\n",
                                          rContext.mProcess.GetProcessID(), reason ? reason : "<UNKNOWN REASON>");
            printf("%s", message);
          }
          else
          {
            char message[1024];
            ::snprintf (message, sizeof(message), "Process %lld stopped and restarted, reasons:\n",
                                          rContext.mProcess.GetProcessID());
            printf("%s", message);
            for (size_t i = 0; i < num_reasons; i++)
            {
              const char *reason = SBProcess::GetRestartedReasonAtIndexFromEvent (evt, i);
              ::snprintf(message, sizeof(message), "\t%s\n", reason ? reason : "<UNKNOWN REASON>");
              printf("%s", message);
            }
          }
        }
      }

      if (SBProcess::GetRestartedFromEvent(evt))
        continue;
      switch (state)
      {
        case eStateInvalid:
          printf("StateInvalid\n"); break;
        case eStateDetached:
          printf("StateDetached\n"); break;
        case eStateCrashed:
          printf("StateCrashed\n"); break;
        case eStateUnloaded:
          printf("StateUnloaded\n"); break;
        case eStateExited:
          printf("StateExited\n"); break;
          return;
        case eStateConnected:
          printf("StateConnected\n"); break;
        case eStateAttaching:
          printf("StateAttaching\n"); break;
        case eStateLaunching:
          printf("StateLaunching\n"); break;
        case eStateRunning:
          //printf("StateRunning\n"); break;
        case eStateStepping:
          printf("StateStepping\n"); break;
          continue;
        case eStateStopped:
        case eStateSuspended:
        {
          printf("StateStopped or StateSuspended\n");
          nuiAnimation::RunOnAnimationTick(nuiMakeTask(this, &MainWindow::UpdateProcess));
        }
          break;
			}
		}
  }
}

void MainWindow::UpdateProcess()
{
  DebuggerContext& rContext(GetDebuggerContext());
  //PrintDebugState(rContext.mProcess);

  ProcessTree* pTree = new ProcessTree(rContext.mProcess);
  pTree->Acquire();
  pTree->Open(true);
  mpThreads->SetTree(pTree);

}

void MainWindow::SelectProcess(lldb::SBProcess process)
{
  NGL_OUT("Selected Process\n");
}

void MainWindow::SelectThread(lldb::SBThread thread)
{
  NGL_OUT("Selected Thread\n");
}

void MainWindow::UpdateVariables(lldb::SBFrame frame)
{
  nuiTreeNode* pTree = new nuiTreeNode("Variables");

  lldb::SBValueList args;
  lldb::SBValueList locals;
  args = frame.GetVariables(
                            true, //bool arguments,
                            false, //bool locals,
                            false, //bool statics,
                            false); //bool in_scope_only);

  locals = frame.GetVariables(
                              false, //bool arguments,
                              true, //bool locals,
                              false, //bool statics,
                              false); //bool in_scope_only);

  uint32_t count = 0;
  count = args.GetSize();
  nuiTreeNode* pArgNode = new nuiTreeNode("Arguments");
  pTree->AddChild(pArgNode);
  for (uint32 i = 0; i < count; i++)
  {
    lldb::SBValue val = args.GetValueAtIndex(i);
    nuiTreeNode* pNode = new VariableNode(val);
    pArgNode->AddChild(pNode);
    //NGL_OUT("%d %s %s \n", i, val.GetTypeName(), val.GetName());
  }
  pArgNode->Open(true);

  count = locals.GetSize();
  nuiTreeNode* pLocalNode = new nuiTreeNode("Locals");
  pTree->AddChild(pLocalNode);
  for (uint32 i = 0; i < count; i++)
  {
    lldb::SBValue val = locals.GetValueAtIndex(i);
    nuiTreeNode* pNode = new VariableNode(val);
    pLocalNode->AddChild(pNode);
    //NGL_OUT("%d %s %s \n", i, val.GetTypeName(), val.GetName());
  }
  pLocalNode->Open(true);

  pTree->Open(true);
  mpVariables->SetTree(pTree);
}

void MainWindow::SelectFrame(lldb::SBFrame frame)
{
  NGL_OUT("Selected Frame\n");

  UpdateVariables(frame);
}

void MainWindow::OnThreadSelectionChanged(const nuiEvent& rEvent)
{
  ProcessTree* pNode = (ProcessTree*)mpThreads->GetSelectedNode();
  if (!pNode)
  {
    return;
  }

  ProcessTree::Type type = pNode->GetType();
  switch (type)
  {
    case ProcessTree::eProcess:
      // Nothing to do for now
      SelectProcess(pNode->GetProcess());
      break;
    case ProcessTree::eThread:
      // Select the thread
      SelectThread(pNode->GetThread());
      break;
    case ProcessTree::eFrame:
      // Select the frame
      SelectFrame(pNode->GetFrame());
      break;
  }
}
