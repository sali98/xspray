//
//  DebugView.h
//  Xspray
//
//  Created by Sébastien Métrot on 6/30/13.
//
//

#pragma once

class DebugView : public nuiLayout
{
public:
  DebugView();
  virtual ~DebugView();

  virtual void Built();

  nuiSignal0<> GoHome;
private:
  nuiEventSink<DebugView> mEventSink;
  nuiSlotsSink mSlotSink;

  void OnProcessPaused();
  void OnProcessRunning();
  void UpdateProcess();

  void OnChooseApplication(const nuiEvent& rEvent);
  void OnStart(const nuiEvent& rEvent);
  void OnStartLOCAL();
  void OnStartIOS();

  void OnPause(const nuiEvent& rEvent);
  void OnContinue(const nuiEvent& rEvent);
  void OnStepIn(const nuiEvent& rEvent);
  void OnStepOver(const nuiEvent& rEvent);
  void OnStepOut(const nuiEvent& rEvent);
  void OnThreadSelectionChanged(const nuiEvent& rEvent);
  void Loop();
  void UpdateVariablesForCurrentFrame();
  void OnModuleFileSelectionChanged(const nuiEvent& rEvent);
  void OnModuleSymbolSelectionChanged(const nuiEvent& rEvent);
  void OnProcessConnected();

  void OnCloseTab(const nuiEvent& event);
  
  void OnVariableSelectionChanged(const nuiEvent& rEvent);

  void OnLineSelected(const nglPath& rPath, float X, float Y, int32 line, bool ingutter);

  void ShowSource(const nglPath& rPath, int32 line, int32 col);

  void OnDeviceConnected(Xspray::iOSDevice& device);
  void OnDeviceDisconnected(Xspray::iOSDevice& device);

  void OnHandleSTDIO(const nuiEvent& event);

  nglThreadDelegate* mpDebuggerEventLoop;

  nuiTreeView* mpThreads;
  nuiTreeView* mpModulesFiles;
  nuiTreeView* mpModulesSymbols;
  nuiTreeView* mpVariables;
  nuiWidget* mpTransport;
  nuiButton* mpChooseApplication;
  nuiComboBox* mpArchitecturesCombo;
  nuiComboBox* mpDevicesCombo;
  nuiButton* mpStart;
  nuiButton* mpPause;
  nuiButton* mpContinue;
  nuiButton* mpStepIn;
  nuiButton* mpStepOver;
  nuiButton* mpStepOut;
  nuiTabView* mpFilesTabView;
  nuiText* mpOutput;
  nuiText* mpErrors;

  nuiTreeNodePtr mpArchitectures;
  nuiTreeNodePtr mpDevices;

  GraphView* mpGraphView;

  void SelectProcess(lldb::SBProcess process);
  void SelectThread(lldb::SBThread thread);
  void SelectFrame(lldb::SBFrame frame);
  void UpdateVariables(lldb::SBFrame frame);

  void UpdateArchitectures(std::vector<nglString>& rArchis);

  std::map<nglString, nuiWidget*> mFiles;
};

