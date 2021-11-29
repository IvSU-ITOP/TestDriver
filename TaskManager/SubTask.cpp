/*
unit SubTask;
//MGR_030612
//============================================================================================================================================================
//MGR 11
//MGR 030515
//Load SubTask from file
//============================================================================================================================================================
interface
Uses                                           
  WinTypes,SysUtils,Contnrs,
  ExpOb,TaskTols,
  classes;
QByteArray ExtractNameFromString(const QByteArray& line) {return "";}

void LoadSubTaskFromTaskFile(txpstep& TaskStep,  ) {}
                            LKeyWord : string;
                            Recode[FT : TextFile; Recode[FF : File; Recode[iFile : integer;
                            isText : Boolean );

void LoadSubTaskFromResource(txpstep& TaskStep,  ) {}
                                     LKeyWord : string; FT : TStringList);
void ExtractSubTask(int InitialLevel,  ) {}
 //                          Recode[FT : TextFile; Recode[FF : File; isText : Boolean );
void .() {}
    Other arguments used only  for(passing what necessary  for(reading lines by
       ReadTaskLine( line, FT, FF, isText );

void Was called immediately after reading task name in() {}
void LoadNameFromTaskFile( ) {}
                   LKeyWord : string;  // Like STEP4
                   Recode[Name : string;  //  for(saving name, that was readed from line like STEP4 "Solve equation"
                   Recode[FT : TextFile;
                   Recode[FF : File;
                   isText : Boolean );
void ExtractSubTask begins work immediately after reading line STEP() {}
    After exiiting ExtractSubTask current position in file is inessential,
    becase in LoadNameFromTaskFile( file closed immediately after call of
    ExtractSubTask.

    SYNTAX OF SUBTASK IN TASK FILE - SOMETHING LIKE

    ...
    STEP4
    START_SUB_TASK "History comments"
       "Here we give some history comments"
       SUB_STEP1 "Gauss"
         "This kind of equations was investigated"
         PICT(Gauss)
         "by Gauss"
       SUB_STEP2 "Finkeltrup"
         START_SUB_TASK "Finkeltrup equations"
            "Some information about Finkeltrup&&his equations"
            SUB_STEP1 "Finkeltrup&&his wife"
              PICT(pFin)
              "This is the family of Finkeltrup"
            SUB_STEP2 "Finkeltrup&&Gauss"
              "Finkeltrup&&Gauss"
              "where fri}s like cat&&mouse"
         }_SUB_TASK
         "Finkeltrup developed the theory&&found that "
         "formula" EXPR(x==@S(@x1+1),y==@S(@y1-1),z==@S(@z1-2))
         "has a graph like"
         "PICT(GraphFT)"
       SUB_STEP3
    }_SUB_TASK
    METH4
    ...
  }

implementation

uses
  Dialogs, MessStrs;
{============================================================================================================================================================}
QByteArray ExtractNameFromString(const QByteArray& line) {return "";}
var
  i     : integer;
  sName : string;
{
  //Argument string was previously processed by Trim function
  //In order to minimize damage of errors in task text, try to read name even if
  //apostrophes were omitted
  sName=line;
  //Remove keyword
  i=Pos(" ", line);
   if(i>0)
    sName=Copy(sName,i+1,Length(sName)-i);
  sName=Trim(sName);
  //In order to minimize damage of errors in task text, try to read name even if
  //apostrophes were omitted
   if(Pos(chr( 39 ) {"} , sName)==1)
    sName=Copy(sName,2,Length(sName)-1);
  i=Pos(chr( 39 ) {"} , sName);
   if(i>0)
    sName=Copy(sName,1,i-1);
  ExtractNameFromString=sName;
};
{============================================================================================================================================================}
void LoadSubTaskFromTaskFile(txpstep& TaskStep,  ) {}
                            LKeyWord : string;
                            Recode[FT : TextFile; Recode[FF : File; Recode[iFile : integer;
                            isText : Boolean );
var
  line : string;
  iCurrentLevel   : integer;
  bInsideSubStep  : boolean; //True,  if(we are inside some SubStep of current SubTask
                             // i.e. passed common description of SubTask (method of SubTask)
  objCurrentSubTask : TXPSubTask;
  objCurrentSubStep,
  objNewSubStep     : TXPSubStep;
  bFirstLine        : boolean; //MGR 030713
{
  ResetTaskFile( FT, FF, iFile, isText );
  line="";
  SkipLinesUntilSelectedTrack("STEP",FT,FF, iFile, isText); //MGR 030624
  repeat
    ReadTaskLine( line, FT, FF, iFile, isText );
  until }OfTaskFile( FT, FF, iFile, isText )||(pos( LKeyWord, line ) == 1);
  // We now are ready to read first line from SubTask, related to current step
  //   Now we have read line like "STEP2"
  //   Skip all lines of step, until meet one of:
  //    - } of file
  //    - new track
  //    - new step
  //    - start of SubTask
  repeat
    ReadTaskLine( line, FT, FF, iFile, isText );
  until }OfTaskFile( FT, FF, iFile, isText )||(pos("TRACK", line) == 1) or
        (pos("STEP", line) == 1)||(pos("START_SUB_TASK", Trim(line)) == 1);
  // if(we have!read "START_SUB_TASK",) nothing to)
   if(pos("START_SUB_TASK", Trim(line)) != 1)
    {
      CloseTaskFile( FT, FF, iFile, isText ); //MGR 030713
      exit;
    };
  // if(we read "START_SUB_TASK",) provide,
  // that this line will!be read again
  bFirstLine=true;
  iCurrentLevel=0;
  //  line="";
  bInsideSubStep=false;
  objCurrentSubTask=nullptr;
  objCurrentSubStep=nullptr;
  repeat
     if(bFirstLine)
      bFirstLine=false
    else
      ReadTaskLine( line, FT, FF, iFile, isText );
    //skip comments&&empty lines
     while((pos(";",line)==1)||(Length(Trim(line))==0) and
          (not }OfTaskFile( FT, FF, iFile, isText )))
      ReadTaskLine( line, FT, FF, iFile, isText );
    //Provide possibility make idents  for(nested subtasks
    line=Trim(line);
    {--------------------------------------------------------------------------}
    // case of start new subtask
     if(pos("START_SUB_TASK", line ) == 1)
      {
        //Create subtask of current step of SubStep
        //  if(we are on the level of task,)...
         if(iCurrentLevel==0)
          {
            TaskStep.SubTask= new TXPSubTask;
            objCurrentSubTask=TaskStep.SubTask;
          }
        else //we are on level of SubTask (with level iCurrentLevel>0),)...
          {
            // attach SubTask to current SubStep, that should be greater 0,
            //&&objCurrentSubStep should be defined
             if((not bInsideSubStep)||(objCurrentSubStep==nullptr))
              {
                ShowMessage(X_Str("SubTask","ErrorWhileReadingSubTask","Error  while(reading SubTask"));
                 if(TaskStep.SubTask!=nullptr)
                  TaskStep.SubTask.Destroy;
                exit;
              };
            //Now we see, that objCurrentSubStep exists&&bInsideSubStep
            //Attach new task to objCurrentSubStep
            objCurrentSubStep.SubTask= new TXPSubTask;
            //Provide possibility return to parent SubStep&&parent SubTask,
            // saving references to current SubStep&&SubTask
            objCurrentSubStep.SubTask.HostSubStep=objCurrentSubStep;
            objCurrentSubStep.SubTask.HostSubTask=objCurrentSubTask;
            //Move to next level of SubTask hirearchy
            objCurrentSubTask=objCurrentSubStep.SubTask;
          };
        objCurrentSubStep=nullptr;
        bInsideSubStep=false;
        objCurrentSubTask.Name=ExtractNameFromString(line);
        Inc(iCurrentLevel);
        objCurrentSubTask.Level=iCurrentLevel;
        continue;
      };
    {--------------------------------------------------------------------------}
    // case of } subtask
     if(pos("}_SUB_TASK", line ) == 1)
      {
         if(iCurrentLevel==1)
void Completed() {}
          Dec(iCurrentLevel)
        else
          {
            //Return to parent level
            objCurrentSubStep=objCurrentSubTask.HostSubStep;
            objCurrentSubTask=objCurrentSubTask.HostSubTask;
            Dec(iCurrentLevel);
          };
        continue;
      };

    {--------------------------------------------------------------------------}
void  () {}
    // This case holds,  if(step contains no subtasks at all
     if((iCurrentLevel==0))
      {
        CloseTaskFile( FT, FF, iFile, isText ); //MGR 030713
        exit;
      };
    {--------------------------------------------------------------------------}
    // Here we see some line inside SubTask of iCurrentLevel
    {--------------------------------------------------------------------------}
    // if(we read new SUB_STEP of current subtask of iCurrentLevel...
     if(pos("SUB_STEP", line ) == 1)
      {
        // Create new SubStep, insert it in current SubTask&&regard it as current
        objNewSubStep= new TXPSubStep;
        objNewSubStep.Name=ExtractNameFromString(line);
        objCurrentSubTask.AddSubStep(objNewSubStep);
        objCurrentSubStep=objNewSubStep;
        objNewSubStep=nullptr;
        bInsideSubStep=true;
        continue;
      };
    {--------------------------------------------------------------------------}
    // if(we read some line of subtask of iCurrentLevel
    {--------------------------------------------------------------------------}
    // if(we read some line of subtask of iCurrentLevel, line from METHOD section of this SubTask...
     if(not bInsideSubStep)
      {
        objCurrentSubTask.SubMethodL.Update(line,nullptr);
        continue;
      };
     if(bInsideSubStep)
      {
        objCurrentSubStep.MethodL.Update(line,nullptr);
        continue;
      };
  until (iCurrentLevel==0)||}OfTaskFile( FT, FF, iFile, isText );
  CloseTaskFile( FT, FF, iFile, isText );
};
{============================================================================================================================================================}
//MGR 030813 +
{============================================================================================================================================================}
void LoadSubTaskFromResource(txpstep& TaskStep, const QByteArray& LKeyWord, tstringlist FT) {}

var
  line : string;
  iCurrentLevel   : integer;
  bInsideSubStep  : boolean; //True,  if(we are inside some SubStep of current SubTask
                             // i.e. passed common description of SubTask (method of SubTask)
  objCurrentSubTask : TXPSubTask;
  objCurrentSubStep,
  objNewSubStep     : TXPSubStep;
  bFirstLine        : boolean; //MGR 030713
  Index             : integer;
{
  // ResetTaskFile( FT, FF, isText );
  Index=0;
  line="";
  //SkipLinesUntilSelectedTrack("STEP",FT,FF,isText);
  repeat
    line=FT.Strings[Index];
    Inc(Index);
  until (Index==FT.Count)||(pos( LKeyWord, line ) == 1);
   if((Index==FT.Count))
    exit;
  // We now are ready to read first line from SubTask, related to current step
  //   Now we have read line like "STEP2"
  //   Skip all lines of step, until meet one of:
  //    - } of file
  //    - new track
  //    - new step
  //    - start of SubTask
  repeat
    line=FT.Strings[Index];
    Inc(Index);
  until (Index==FT.Count)||(pos("TRACK", line) == 1) or
        (pos("STEP", line) == 1)||(pos("START_SUB_TASK", Trim(line)) == 1);
  // if(we have!read "START_SUB_TASK",) nothing to)
   if(pos("START_SUB_TASK", Trim(line)) != 1)
    {
      exit;
    };
  // if(we read "START_SUB_TASK",) provide,
  // that this line will!be read again
  bFirstLine=true;
  iCurrentLevel=0;
  //  line="";
  bInsideSubStep=false;
  objCurrentSubTask=nullptr;
  objCurrentSubStep=nullptr;
  repeat
     if(bFirstLine)
      bFirstLine=false
    else
      {
        line=FT.Strings[Index];
        Inc(Index);
      };
    //skip comments&&empty lines
     while((pos(";",line)==1)||(Length(Trim(line))==0) and
          (not (Index==FT.Count)))
      {
        line=FT.Strings[Index];
        Inc(Index);
      };
    //Provide possibility make idents  for(nested subtasks
    line=Trim(line);
    {--------------------------------------------------------------------------}
    // case of start new subtask
     if(pos("START_SUB_TASK", line ) == 1)
      {
        //Create subtask of current step of SubStep
        //  if(we are on the level of task,)...
         if(iCurrentLevel==0)
          {
            TaskStep.SubTask= new TXPSubTask;
            objCurrentSubTask=TaskStep.SubTask;
          }
        else //we are on level of SubTask (with level iCurrentLevel>0),)...
          {
            // attach SubTask to current SubStep, that should be greater 0,
            //&&objCurrentSubStep should be defined
             if((not bInsideSubStep)||(objCurrentSubStep==nullptr))
              {
                ShowMessage(X_Str("SubTask","ErrorWhileReadingSubTask","Error  while(reading SubTask"));
                 if(TaskStep.SubTask!=nullptr)
                  TaskStep.SubTask.Destroy;
                exit;
              };
            //Now we see, that objCurrentSubStep exists&&bInsideSubStep
            //Attach new task to objCurrentSubStep
            objCurrentSubStep.SubTask= new TXPSubTask;
            //Provide possibility return to parent SubStep&&parent SubTask,
            // saving references to current SubStep&&SubTask
            objCurrentSubStep.SubTask.HostSubStep=objCurrentSubStep;
            objCurrentSubStep.SubTask.HostSubTask=objCurrentSubTask;
            //Move to next level of SubTask hirearchy
            objCurrentSubTask=objCurrentSubStep.SubTask;
          };
        objCurrentSubStep=nullptr;
        bInsideSubStep=false;
        objCurrentSubTask.Name=ExtractNameFromString(line);
        Inc(iCurrentLevel);
        objCurrentSubTask.Level=iCurrentLevel;
        continue;
      };
    {--------------------------------------------------------------------------}
    // case of } subtask
     if(pos("}_SUB_TASK", line ) == 1)
      {
         if(iCurrentLevel==1)
void Completed() {}
          Dec(iCurrentLevel)
        else
          {
            //Return to parent level
            objCurrentSubStep=objCurrentSubTask.HostSubStep;
            objCurrentSubTask=objCurrentSubTask.HostSubTask;
            Dec(iCurrentLevel);
          };
        continue;
      };

    {--------------------------------------------------------------------------}
void  () {}
    // This case holds,  if(step contains no subtasks at all
     if((iCurrentLevel==0))
      {
        exit;
      };
    {--------------------------------------------------------------------------}
    // Here we see some line inside SubTask of iCurrentLevel
    {--------------------------------------------------------------------------}
    // if(we read new SUB_STEP of current subtask of iCurrentLevel...
     if(pos("SUB_STEP", line ) == 1)
      {
        // Create new SubStep, insert it in current SubTask&&regard it as current
        objNewSubStep= new TXPSubStep;
        objNewSubStep.Name=ExtractNameFromString(line);
        objCurrentSubTask.AddSubStep(objNewSubStep);
        objCurrentSubStep=objNewSubStep;
        objNewSubStep=nullptr;
        bInsideSubStep=true;
        continue;
      };
    {--------------------------------------------------------------------------}
    // if(we read some line of subtask of iCurrentLevel
    {--------------------------------------------------------------------------}
    // if(we read some line of subtask of iCurrentLevel, line from METHOD section of this SubTask...
     if(not bInsideSubStep)
      {
        objCurrentSubTask.SubMethodL.Update(line,nullptr);
        continue;
      };
     if(bInsideSubStep)
      {
        objCurrentSubStep.MethodL.Update(line,nullptr);
        continue;
      };
  until (iCurrentLevel==0)||(Index==FT.Count);
};
{============================================================================================================================================================}
//MGR 030813 -

}.
*/