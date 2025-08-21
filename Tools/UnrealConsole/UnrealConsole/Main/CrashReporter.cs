using System;
using System.IO;
using System.Diagnostics;
using System.Collections.Generic;
using System.Text;

namespace UnrealConsole
{
    class CrashReporter
    {
        public CrashReporter()
        {

        }

        /**
         * Finds Identifier in SearchTarget and returns the rest of the line after Identifier
         */
        public string FindLine(string Identifier, string SearchTarget)
        {
            int IdentifierStartIndex = SearchTarget.IndexOf(Identifier);
            string ResultString = "";
            if (IdentifierStartIndex >= 0)
            {
                int EndIndex = SearchTarget.IndexOf("\n", IdentifierStartIndex);
                if (EndIndex < 0)
                {
                    EndIndex = SearchTarget.Length;
                }
                int StartIndex = IdentifierStartIndex + Identifier.Length;
                ResultString = SearchTarget.Substring(StartIndex, EndIndex - StartIndex);
            }
            else
            {
                ResultString = "not found";
            }
            return ResultString;
        }

        /**
         * Removes some junk that the PS3 appends to the assert message
         * Does nothing on other platforms
         */
        public string FormatAssertMessage(string AssertMessage, string LogFileContents)
        {
            //find the first "lv2", cut off anything after that
            int PS3CrapStartIndex = AssertMessage.IndexOf("lv2");
            //find the "[PS3Callstack" garbage
            int PS3MoreCrapStartIndex = AssertMessage.IndexOf("[PS3Callstack");

            if (PS3MoreCrapStartIndex < PS3CrapStartIndex && PS3MoreCrapStartIndex != -1
                || PS3CrapStartIndex == -1)
            {
                PS3CrapStartIndex = PS3MoreCrapStartIndex;
            }

            string FormattedAssert;
            if (PS3CrapStartIndex >= 0)
            {
                FormattedAssert = AssertMessage.Substring(0, PS3CrapStartIndex - 0);
            }
            else
            {
                FormattedAssert = AssertMessage;
            }

            //assert message can't be empty, web site formatting depends on there being at least one line
            if (FormattedAssert.Length < 3)
            {
                int EndAssertIndex = LogFileContents.IndexOf(AssertMessage);
                if (EndAssertIndex > 0)
                {
                    //assume the assert is in the last couple of log lines before AssertMessage
                    int StartAssertIndex = 0;
                    int PreviousLineIndex = LogFileContents.LastIndexOf("\n", EndAssertIndex - 1);
                    if (PreviousLineIndex > 0)
                    {
                        StartAssertIndex = PreviousLineIndex;
                        PreviousLineIndex = LogFileContents.LastIndexOf("\n", PreviousLineIndex - 1);
                        if (PreviousLineIndex > 0)
                        {
                            StartAssertIndex = PreviousLineIndex;
                            PreviousLineIndex = LogFileContents.LastIndexOf("\n", PreviousLineIndex - 1);
                            if (PreviousLineIndex > 0)
                            {
                                StartAssertIndex = PreviousLineIndex;
                            }
                        }
                    }
                    FormattedAssert = LogFileContents.Substring(StartAssertIndex, EndAssertIndex - StartAssertIndex);
                }
                else
                {
                    FormattedAssert = "Couldn't find assert message";
                }
            }
            return FormattedAssert;
        }

        /**
         * Formats the callstack to be consistent with VS Studio callstacks
         * This is necessary so the web site can format it correctly when viewing.
         */
        public string FormatCallStack(string CallStack)
        {
            int LineEndIndex = 0;
            int FunctionEndIndex = CallStack.IndexOf(" ", LineEndIndex);
            int FunctionParamsStartIndex = CallStack.IndexOf("(", LineEndIndex);
            int FunctionParamsEndIndex = CallStack.IndexOf(")", LineEndIndex);

            while (FunctionEndIndex >= 0 && LineEndIndex >= 0)
            {
                if (FunctionParamsStartIndex < FunctionEndIndex
                    && FunctionParamsStartIndex >= 0
                    && FunctionParamsStartIndex < FunctionParamsEndIndex)
                {
                    //strip out the function parameters since they complicate the crash report web site's callstack formatting significantly
                    //todo: handle the case where the function parameters have nested parentheses
                    CallStack = CallStack.Substring(0, FunctionParamsStartIndex + 1) + CallStack.Substring(FunctionParamsEndIndex);
                }
                else
                {
                    //functions are denoted by the trailing "()", add it since it doesn't exist
                    CallStack = CallStack.Insert(FunctionEndIndex, "()");
                }

                int FilenameStartIndex = CallStack.IndexOf("[", FunctionEndIndex);
                if (FilenameStartIndex >= 0)
                {
                    //filenames start with "[File="
                    CallStack = CallStack.Insert(FilenameStartIndex + 1, "File=");
                }
                
                //find the end of the line
                LineEndIndex = CallStack.IndexOf("\n", FunctionEndIndex);
                if (LineEndIndex >= 0)
                {
                    //search for the end of the next function by finding the first space, but start at the end of the last line
                    //but if we find a ( first, then the function is already good to go
                    FunctionEndIndex = CallStack.IndexOf(" ", LineEndIndex + 1);
                    FunctionParamsStartIndex = CallStack.IndexOf("(", LineEndIndex + 1);
                    FunctionParamsEndIndex = CallStack.IndexOf(")", LineEndIndex + 1);
                }
            }
            
            return CallStack;
        }

        /**
         * Dumps out crash report information into temporary files and then starts up the autoreporter app and passes relevant 
         * filenames on the commandline.
         */
        public string SendCrashReport(string AssertMessage, string TranslatedCallstack, string TTYOutput, string PlatformName)
        {
            string ResultMessage = "Crash Report Successful!";

            try
            {
                //note: these are replicated in FOutputDeviceWindowsError::HandleError()
                //the dump format must be recognized by the autoreporter app, ReportFile::ParseReportFile()
                const string ReportDumpVersion = "3";
                const string ReportDumpFilename = "UE3AutoReportDump.txt";
                const string AutoReportExe = "AutoReporter.exe";
                const string LogFileName = "UnrealConsoleLogTransfer.txt";

                //use this string to identify the start of the current session
                int LogStartIndex = TTYOutput.LastIndexOf("Init: Version");
                //if we couldn't find the start of session identifier, just use the whole TTY output
                if (LogStartIndex < 0)
                {
                    LogStartIndex = 0;
                }
                //read the log out of the TTY output
                string LogFileContents = TTYOutput.Substring(LogStartIndex);
                
                //write out the log to a temporary file
                File.WriteAllText(LogFileName, LogFileContents);

                string CrashReportDump = "";
                CrashReportDump += ReportDumpVersion + "\0";

                string ComuterName = System.Windows.Forms.SystemInformation.ComputerName;
                //make the name consistent with appComputerName()
                ComuterName = ComuterName.Replace("-", "");
                CrashReportDump += ComuterName + "\0";

                string UserName = System.Windows.Forms.SystemInformation.UserName;
                //make the name consistent with appUserName()
                UserName = UserName.Replace(".", "");
                CrashReportDump += UserName + "\0";

                //skip Game name for now
                CrashReportDump += "n/a" + "\0";

                //platform
                CrashReportDump += PlatformName + "\0";

                //skip language for now
                CrashReportDump += "int" + "\0";

                //build up a date string consistent with appSystemTimeString()
                //"2006.10.11-13.50.53"
                string UE3SystemTimeString = DateTime.Now.Year.ToString() + ".";
                UE3SystemTimeString += DateTime.Now.Month.ToString() + ".";
                UE3SystemTimeString += DateTime.Now.Day.ToString() + "-";
                UE3SystemTimeString += DateTime.Now.Hour.ToString() + ".";
                UE3SystemTimeString += DateTime.Now.Minute.ToString() + ".";
                UE3SystemTimeString += DateTime.Now.Second.ToString();
                CrashReportDump += UE3SystemTimeString + "\0";

                //parse the engine version out of the TTY
                string EngineVersion = FindLine("Version:", LogFileContents);
                CrashReportDump += EngineVersion + "\0";

                //skip changelist version
                CrashReportDump += "0" + "\0";

                //parse the commandline out of the log
                string CommandLine = FindLine("Command line:", LogFileContents);
                CrashReportDump += CommandLine + "\0";

                //skip base directory
                CrashReportDump += "n/a" + "\0";

                //format the callstack consistent with VS Studio
                string FormattedCallStack = FormatCallStack(TranslatedCallstack);
                string FormattedAssertMessage = FormatAssertMessage(AssertMessage, LogFileContents);
                CrashReportDump += FormattedAssertMessage + "\n" + FormattedCallStack + "\0";

                //assume we're in game mode
                CrashReportDump += "Game" + "\0";

                //write out the temporary dump file with the accumulated information
                File.WriteAllText(ReportDumpFilename, CrashReportDump, Encoding.Unicode);

                //send the temporary file names on the commandline, protect against spaces in the path names by surrounding with ""
                string AutoreportCommandline = "\"" + ReportDumpFilename + "\" \"" + LogFileName + "\" \"" + "noIniDumpYet" + "\" \"" + "noMiniDumpYet" + "\"";
                
                Process.Start(AutoReportExe, AutoreportCommandline);
            }
            catch (Exception e)
            {
                ResultMessage = "Couldn't send the crash report: " + e.Message;
            }
            return ResultMessage;
        }
    }
}
