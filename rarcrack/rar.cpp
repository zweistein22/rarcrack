#include "rar.hpp"

#if !defined(RARDLL)
int rarmain(int argc, char *argv[])
{

#ifdef _UNIX
  setlocale(LC_ALL,"");
#endif

  
  

#ifdef SFX_MODULE
  std::wstring ModuleName;
#ifdef _WIN_ALL
  ModuleName=GetModuleFileStr();
#else
  CharToWide(argv[0],ModuleName);
#endif
#endif

#ifdef _WIN_ALL
  SetErrorMode(SEM_NOALIGNMENTFAULTEXCEPT|SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);


#endif

#if defined(_WIN_ALL) && !defined(SFX_MODULE)
  // Must be initialized, normal initialization can be skipped in case of
  // exception.
  POWER_MODE ShutdownOnClose=POWERMODE_KEEP;
#endif

  try 
  {
  
    // Use std::unique_ptr to free Cmd in case of exception.
    std::unique_ptr<CommandData> Cmd(new CommandData);
#ifdef SFX_MODULE
    Cmd->Command=L"X";
    char *Switch=argc>1 ? argv[1]:NULL;
    if (Switch!=NULL && Cmd->IsSwitch(Switch[0]))
    {
      int UpperCmd=etoupper(Switch[1]);
      switch(UpperCmd)
      {
        case 'T':
        case 'V':
          Cmd->Command[0]=UpperCmd;
          break;
        case '?':
          Cmd->OutHelp(RARX_SUCCESS);
          break;
      }
    }
    Cmd->AddArcName(ModuleName);
    Cmd->ParseDone();
    Cmd->AbsoluteLinks=true; // If users runs SFX, he trusts an archive source.
#else // !SFX_MODULE
    if (!strcmp(argv[3],"-p10")) {
        int a = 1;
    }
    Cmd->ParseCommandLine(true,argc,argv);
    if (!Cmd->ConfigDisabled)
    {
      Cmd->ReadConfig();
      Cmd->ParseEnvVar();
    }
    Cmd->ParseCommandLine(false,argc,argv);
#endif

#if defined(_WIN_ALL) && !defined(SFX_MODULE)
    ShutdownOnClose=Cmd->Shutdown;
    if (ShutdownOnClose!=POWERMODE_KEEP)
      ShutdownCheckAnother(true);
#endif

    uiInit(Cmd->Sound);
    InitLogOptions(Cmd->LogName,Cmd->ErrlogCharset);
    ErrHandler.SetSilent(Cmd->AllYes || Cmd->MsgStream==MSG_NULL);

    Cmd->OutTitle();
    Cmd->ProcessCommand();
    RAR_EXIT rarexit = ErrHandler.GetErrorCode();
    if (Cmd->DllError == ERAR_BAD_PASSWORD)  return RARX_BADPWD;
    if (Cmd->DllError == ERAR_SUCCESS) {
        return RARX_SUCCESS;
    }
    return rarexit;
  }
  catch (RAR_EXIT ErrCode)
  {
    ErrHandler.SetErrorCode(ErrCode);
  }
  catch (std::bad_alloc&)
  {
    ErrHandler.MemoryErrorMsg();
    ErrHandler.SetErrorCode(RARX_MEMORY);
  }
  catch (...)
  {
    ErrHandler.SetErrorCode(RARX_FATAL);
  }

#if defined(_WIN_ALL) && !defined(SFX_MODULE)
  if (ShutdownOnClose!=POWERMODE_KEEP && ErrHandler.IsShutdownEnabled() &&
      !ShutdownCheckAnother(false))
    Shutdown(ShutdownOnClose);
#endif
  ErrHandler.MainExit=true;
  CloseLogOptions();
  RAR_EXIT rv=  ErrHandler.GetErrorCode();
  //printf("Main exit: rv=%i", rv);
  return rv;
}
#endif


