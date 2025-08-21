// enable this iot get timestamps,threadid info for allocations
//#define EXTRA_INFO

using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.IO;
using System.Collections;
using ConsoleInterface;
using System.Text;

namespace MemoryProfiler
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }

    /**
     * Data associated with each node to open the proper file and select line.
     */
    public class FNodePayload
    {
        /** Name of file */
        public string Filename;
        /** Line number to select */
        public int LineNumber;

        /** Index of filename in name array. */
        public int FilenameIndex;
        /** Index of function in name array. */
        public int FunctionIndex;
        /** Index of address in callstack address array. */
        public int AddressIndex;

        /**
         * Constructor, initializing all members to passed in values.
         */
        public FNodePayload(string InFilename, int InLineNumber, int InFilenameIndex, int InFunctionIndex, int InAddressIndex)
        {
            Filename = InFilename;
            LineNumber = InLineNumber;
            FilenameIndex = InFilenameIndex;
            FunctionIndex = InFunctionIndex;
            AddressIndex = InAddressIndex;
        }
    }

    /**
     * Big endian version of binary reader.
     */
    public class BinaryReaderBigEndian : BinaryReader
    {
        /**
         * Constructor, passing arguments to base class.
         */
        public BinaryReaderBigEndian(Stream Input)
            : base(Input)
        {
        }

        /**
         * Reads & swaps bytes array of size count.
         */
        private byte[] ReadSwappedBytes(int Count)
        {
            byte[] SwappedBytes = new byte[Count];
            for (int i = Count - 1; i >= 0; i--)
            {
                SwappedBytes[i] = ReadByte();
            }
            return SwappedBytes;
        }

        /**
         * Reads an UInt16 from stream and returns it.
         */
        public override ushort ReadUInt16()
        {
            return BitConverter.ToUInt16(ReadSwappedBytes(2), 0);
        }

        /**
         * Reads an Int16 from stream and returns it.
         */
        public override short ReadInt16()
        {
            return BitConverter.ToInt16(ReadSwappedBytes(2), 0);
        }

        /**
         * Reads an UInt32 from stream and returns it.
         */
        public override int ReadInt32()
        {
            return BitConverter.ToInt32(ReadSwappedBytes(4), 0);
        }

        /**
         * Reads an Int32 from stream and returns it.
         */
        public override uint ReadUInt32()
        {
            return BitConverter.ToUInt32(ReadSwappedBytes(4), 0);
        }

        /**
         * Reads an UInt64 from stream and returns it.
         */
        public override long ReadInt64()
        {
            return BitConverter.ToInt64(ReadSwappedBytes(8), 0);
        }

        /**
         * Reads an Int64 from stream and returns it.
         */
        public override ulong ReadUInt64()
        {
            return BitConverter.ToUInt64(ReadSwappedBytes(8), 0);
        }

        /**
         * Reads a float from stream and returns it.
         */
        public override float ReadSingle()
        {
            return BitConverter.ToSingle(ReadSwappedBytes(4), 0);
        }
    }

    /**
     * Parser class, turning input stream into call graph view.
     */
    public class FStreamParser
    {
        /**
         * The lower 2 bits of a pointer are piggy-bagged to store what kind of data follows it. This enum lists
         * the possible types.
         */
        public enum EProfilingPayloadType
        {
            TYPE_Malloc = 0,
            TYPE_Free = 1,
            TYPE_Realloc = 2,
            TYPE_Other = 3,
            // Don't add more than 4 values - we only have 2 bits to store this.
        }

        /**
         *  The the case of TYPE_Other, this enum determines the subtype of the token.
         */
        public enum EProfilingPayloadSubType
        {
            SUBTYPE_EndOfStreamMarker = 0,
            SUBTYPE_TagStart = 1,
            SUBTYPE_TagStop = 2,
            SUBTYPE_Unknown = 3,
        }

        /**
         * Variable sized token emitted by capture code. The parsing code ReadNextToken deals with this and updates
         * internal data. The calling code is responsible for only accessing member variables associated with the type.
         */
        public class FStreamToken
        {
            /** Type of token */
            public EProfilingPayloadType Type;
            /** Subtype of token if it's of TYPE_Other */
            public EProfilingPayloadSubType SubType;
            /** Pointer in the caes of alloc/ free */
            public UInt32 Pointer;
            /** Old pointer in the case of realloc */
            public UInt32 OldPointer;
            /** New pointer in the case of realloc */
            public UInt32 NewPointer;
            /** Index into callstack array */
            public Int32 CallStackIndex;
            /** Size of allocation in alloc/ realloc case */
            public Int32 Size;
#if EXTRA_INFO
            /** Current time of operation in seconds relative to first */
            public float CurrentTime;
            /** ThreadId of code causing token to be emitted */
            public UInt32 ThreadId;
#endif
            /** Payload if type is TYPE_Other. */
            public UInt32 Payload;

            /** 
             * Updates the token with data read from passed in stream and returns whether we've reached the end.
             */
            public bool ReadNextToken(FStreamParser StreamParser)
            {
                bool bReachedEndOfStream = false;

                // Local copy of binary stream from stream parser.
                BinaryReader BinaryStream = StreamParser.BinaryStream;

                // Read the pointer and convert to token type by looking at lowest 2 bits. Pointers are always
                // 4 byte aligned so need to clear them again after the conversion.
                Pointer = BinaryStream.ReadUInt32();
                int TokenType = ((int)Pointer) & 3;
                Pointer = (UInt32)((long)Pointer & ~3);
                SubType = EProfilingPayloadSubType.SUBTYPE_Unknown;

                // Serialize based on toke type.
                switch (TokenType)
                {
                    // Malloc
                    case (int)EProfilingPayloadType.TYPE_Malloc:
                        Type = EProfilingPayloadType.TYPE_Malloc;
                        CallStackIndex = BinaryStream.ReadInt32();
                        Size = BinaryStream.ReadInt32();
#if EXTRA_INFO
                        CurrentTime = BinaryStream.ReadSingle();
                        ThreadId = BinaryStream.ReadUInt32();
#endif
                        break;
                    // Free
                    case (int)EProfilingPayloadType.TYPE_Free:
                        Type = EProfilingPayloadType.TYPE_Free;
                        CallStackIndex = BinaryStream.ReadInt32();
#if EXTRA_INFO
                        CurrentTime = BinaryStream.ReadSingle();
                        ThreadId = BinaryStream.ReadUInt32();
#endif
                        break;
                    // Realloc
                    case (int)EProfilingPayloadType.TYPE_Realloc:
                        Type = EProfilingPayloadType.TYPE_Realloc;
                        OldPointer = Pointer;
                        NewPointer = BinaryStream.ReadUInt32();
                        CallStackIndex = BinaryStream.ReadInt32();
                        Size = BinaryStream.ReadInt32();
#if EXTRA_INFO
                        CurrentTime = BinaryStream.ReadSingle();
                        ThreadId = BinaryStream.ReadUInt32();
#endif
                        break;
                    // Other
                    case (int)EProfilingPayloadType.TYPE_Other:
                        Type = EProfilingPayloadType.TYPE_Other;
                        // Read subtype.
                        switch (BinaryStream.ReadInt32())
                        {
                            // End of stream!
                            case (int)EProfilingPayloadSubType.SUBTYPE_EndOfStreamMarker:
                                SubType = EProfilingPayloadSubType.SUBTYPE_EndOfStreamMarker;
                                bReachedEndOfStream = true;
                                break;
                            case (int)EProfilingPayloadSubType.SUBTYPE_TagStart:
                                SubType = EProfilingPayloadSubType.SUBTYPE_TagStart;
                                break;
                            case (int)EProfilingPayloadSubType.SUBTYPE_TagStop:
                                SubType = EProfilingPayloadSubType.SUBTYPE_TagStop;
                                break;
                            default:
                                throw new InvalidDataException();
                        }
                        Payload = BinaryStream.ReadUInt32();

                        // Set bit associated with serialized tag if we're starting to tag it.
                        if (SubType == EProfilingPayloadSubType.SUBTYPE_TagStart)
                        {
                            StreamParser.CurrentTags |= Payload;
                        }
                        // Clear bit associated with serialized tag if we're stopping to tag it.
                        else if (SubType == EProfilingPayloadSubType.SUBTYPE_TagStop)
                        {
                            StreamParser.CurrentTags &= ~Payload;
                        }

                        break;
                }

                return !bReachedEndOfStream;
            }
        }

        /**
         * Encapsulates callstack information
         * 
         * @warning: This class requires the callstack addresses to already have been initialized.
         */
        public class FCallStack
        {
            /** CRC of callstack pointers. */
            public Int32 CRC;

            /** Callstack as indices into address list, from bottom to top. */
            public Int32[] SortedAddressIndices;

            /** Array of unique address indices (file/function/line combo) */
            public List<int> AddressIndices;
            /** Array of unique filename indices */
            public List<int> FilenameIndices;
            /** Array of unique function indices */
            public List<int> FunctionIndices;

            /**
             * Constructor
             * 
             * @param	BinaryStream	Stream to read from
             * @param	StreamParser	Parser object containing callstack address array used.
             */
            public FCallStack(BinaryReader BinaryStream, FStreamParser StreamParser)
            {
                // This class requires the callstack addresses to already have been initialized so we can retrieve the filename and 
                // function indices.
                if (StreamParser.CallStackAddressArray.Length == 0)
                {
                    throw new InvalidDataException();
                }

                // Read CRC of original callstack.
                CRC = BinaryStream.ReadInt32();

                // Create new arrays.
                AddressIndices = new List<int>();
                FilenameIndices = new List<int>();
                FunctionIndices = new List<int>();
                SortedAddressIndices = new Int32[FProfileDataHeader.BackTraceDepth];

                // Read call stack address indices and parse into arrays.
                for (int i = 0; i < FProfileDataHeader.BackTraceDepth; i++)
                {
                    int AddressIndex = BinaryStream.ReadInt32();

                    // Copy verbatim into sorted call stack indices array.
                    SortedAddressIndices[i] = AddressIndex;

                    // An address index of -1 means that we've reached the end/ top.
                    if (AddressIndex != -1)
                    {
                        // Look up function and file name index from callstack address array via AddressIndex.
                        int FilenameIndex = StreamParser.CallStackAddressArray[AddressIndex].FilenameIndex;
                        int FunctionIndex = StreamParser.CallStackAddressArray[AddressIndex].FunctionIndex;

                        // Uniquely add indices.
                        if (!AddressIndices.Contains(AddressIndex))
                        {
                            AddressIndices.Add(AddressIndex);
                        }
                        if (!FilenameIndices.Contains(FilenameIndex))
                        {
                            FilenameIndices.Add(FilenameIndex);
                            // Also populate associated mapping.
                            if (!StreamParser.FilenameIndexToSummaryMap.ContainsKey(FilenameIndex))
                            {
                                StreamParser.FilenameIndexToSummaryMap.Add(FilenameIndex, new FAllocationSummary());
                            }
                        }
                        if (!FunctionIndices.Contains(FunctionIndex))
                        {
                            FunctionIndices.Add(FunctionIndex);
                            // Also populate associated mapping.
                            if (!StreamParser.FunctionIndexToSummaryMap.ContainsKey(FunctionIndex))
                            {
                                StreamParser.FunctionIndexToSummaryMap.Add(FunctionIndex, new FAllocationSummary());
                            }
                        }
                    }
                }
            }
        }

        /**
         * Information about an address in a callstack.
         */
        public class FCallStackAddress
        {
            /** Program counter */
            public Int64 ProgramCounter;
            /** Index of module in name array */
            public Int32 ModuleIndex;
            /** Index of filename in name array */
            public Int32 FilenameIndex;
            /** Index of function in name array */
            public Int32 FunctionIndex;
            /** Line number */
            public Int32 LineNumber;
            /** Symbol displacement */
            public Int32 SymbolDisplacement;

            /**
             * Constructor, initializing indices to a passed in name index and other values to 0
             * 
             * @param	NameIndex	Name index to propagate to all indices.
             */
            public FCallStackAddress(int NameIndex)
            {
                ProgramCounter = 0;
                ModuleIndex = NameIndex;
                FilenameIndex = NameIndex;
                FunctionIndex = NameIndex;
                LineNumber = 0;
                SymbolDisplacement = 0;
            }

            /**
             * Serializing constructor.
             * 
             * @param	BinaryStream	Stream to serialize data from.
             */
            public FCallStackAddress(BinaryReader BinaryStream, FStreamParser StreamParser, bool bAllowSymbolLookup)
            {
                ProgramCounter = BinaryStream.ReadInt64();
                ModuleIndex = BinaryStream.ReadInt32();
                FilenameIndex = BinaryStream.ReadInt32();
                FunctionIndex = BinaryStream.ReadInt32();
                LineNumber = BinaryStream.ReadInt32();
                SymbolDisplacement = BinaryStream.ReadInt32();

                // Program counter is the only valid serialized data.
                if (StreamParser.ConsoleTools != null)
                {
                    // Debug info call stack elements will have a NULL program counter; don't replace the strings for them.
                    if( bAllowSymbolLookup && ProgramCounter != 0 )
                    {
                        // Look up symbol info via console support DLL.
                        string Filename = "";
                        string Function = "";

                        StreamParser.ConsoleTools.ResolveAddressToSymboInfo((uint)ProgramCounter, ref Filename, ref Function, ref LineNumber);

                        // Look up filename index.
                        if (StreamParser.NameToIndexMap.ContainsKey(Filename))
                        {
                            // Use existing entry.
                            FilenameIndex = StreamParser.NameToIndexMap[Filename];
                        }
                        // Not found, so we use global name index to set new one.
                        else
                        {
                            // Set name in map associated with global ever increasing index.
                            FilenameIndex = StreamParser.CurrentNameIndex++;
                            StreamParser.NameToIndexMap.Add(Filename, FilenameIndex);
                        }

                        // Look up filename index.
                        if (StreamParser.NameToIndexMap.ContainsKey(Function))
                        {
                            // Use existing entry.
                            FunctionIndex = StreamParser.NameToIndexMap[Function];
                        }
                        // Not found, so we use global name index to set new one.
                        else
                        {
                            // Set name in map associated with global ever increasing index.
                            FunctionIndex = StreamParser.CurrentNameIndex++;
                            StreamParser.NameToIndexMap.Add(Function, FunctionIndex);
                        } 
                    } 

                    ModuleIndex = 0;
                    SymbolDisplacement = 0;
                }
            }

            /** Converts address to human readable string. */
            public String ToString(FStreamParser StreamParser)
            {
                return StreamParser.NameArray[FunctionIndex] + "    " + StreamParser.NameArray[FilenameIndex] + ":" + LineNumber.ToString();
            }
        }

        /**
         * Allocation summary for a given scope, e.g. file, function or function + line.
         */
        public class FAllocationSummary
        {
            /** Total size of allocations, both active and freed. */
            public long SizeTotal;
            /** Size of active allocations. */
            public long SizeActive;
            /** Total number of allocations (lifetime) */
            public int TotalMallocCount;
            /** Total number of deallocations (lifetime) */
            public int TotalFreeCount;
            /** Number of active allocations */
            public int ActiveAllocationCount;

            /**
             * Constructor, 0 initializing all member variables.
             */
            public FAllocationSummary()
            {
                SizeTotal = 0;
                SizeActive = 0;
                TotalMallocCount = 0;
                TotalFreeCount = 0;
                ActiveAllocationCount = 0;
            }

            /**
             * Update summary with an allocation
             *
             * @param	Size	Size of allocation.
             */
            public void UpdateMalloc(int Size)
            {
                SizeTotal += Size;
                SizeActive += Size;
                TotalMallocCount++;
                ActiveAllocationCount++;
            }

            /**
             * Update allocation summay with deallocation.
             * 
             * @param	Size					Size of freed allocation
             * @param	bIsOriginalCallSite		Whether this summary is the one freeing or the one that originally allocated it
             */
            public void UpdateFree(int Size, bool bIsOriginalCallSite)
            {
                // Summary for code performing original allocation
                if (bIsOriginalCallSite)
                {
                    SizeActive -= Size;
                    ActiveAllocationCount--;
                }
                // Summary for code calling free.
                else
                {
                    TotalFreeCount++;
                }
            }

            /** Converts important parts of allocation summary into string form. */
            public override String ToString()
            {
                return SizeActive.ToString("0,0").PadLeft(12, ' ') + "  " + ActiveAllocationCount.ToString("0,0").PadLeft(9, ' ');
            }
        }

        /**
         * Header written by capture tool
         */
        public class FProfileDataHeader
        {
            /** Magic to ensure we're opening the right file.	*/
            public UInt32 Magic;
            /** Version number to detect version mismatches.	*/
            public UInt32 Version;
            /** Platform that was captured.						*/
            public UInt32 Platform;
            /** Back trace depth.								*/
            static public UInt32 BackTraceDepth;

            /** Offset in file for name table.					*/
            public UInt32 NameTableOffset;
            /** Number of name table entries.					*/
            public UInt32 NameTableEntries;

            /** Offset in file for callstack address table.		*/
            public UInt32 CallStackAddressTableOffset;
            /** Number of callstack address entries.			*/
            public UInt32 CallStackAddressTableEntries;

            /** Offset in file for callstack table.				*/
            public UInt32 CallStackTableOffset;
            /** Number of callstack entries.					*/
            public UInt32 CallStackTableEntries;

            /** Offset in file for datafile list.				*/
            public UInt32 DataFilesOffset; 
            /** Number of datafiles entries.					*/
            public UInt32 DataFilesNum; 

            /**
             * Constructor, serializing header from passed in stream.
             * 
             * @param	BinaryStream	Stream to serialize header from.
             */
            public FProfileDataHeader(BinaryReader BinaryStream)
            {
                // Magic and version info, currently ignored.
                Magic = BinaryStream.ReadUInt32();
                Version = BinaryStream.ReadUInt32();
                // Platform and max backtrace depth.
                Platform = BinaryStream.ReadUInt32();
                BackTraceDepth = BinaryStream.ReadUInt32();

                // Name table offset in file and number of entries.
                NameTableOffset = BinaryStream.ReadUInt32();
                NameTableEntries = BinaryStream.ReadUInt32();

                // CallStack address table offset and number of entries.
                CallStackAddressTableOffset = BinaryStream.ReadUInt32();
                CallStackAddressTableEntries = BinaryStream.ReadUInt32();

                // CallStack table offset and number of entries.
                CallStackTableOffset = BinaryStream.ReadUInt32();
                CallStackTableEntries = BinaryStream.ReadUInt32();

                // DataFiles offset and number of entries.
                DataFilesOffset = BinaryStream.ReadUInt32();
                DataFilesNum = BinaryStream.ReadUInt32();
            }
        }

        /**
         * Helper class encapsulating information for an active allocation and associated pointer.
         */
        public class FPointerInfo
        {
            /** Size of allocation. */
            public int Size;
            /** Index of callstack that performed allocation. */
            public int CallStackIndex;
            /** Tags set when pointer was first allocated, might be different than curren in case of realloc. */
            public UInt32 OriginalTags;
            /** Tags at the time of last modification to pointer. */
            public UInt32 CurrentTags;

            /** Constructor, initializing all member variables. */
            public FPointerInfo()
            {
                Size = -1;
                CallStackIndex = -1;
                OriginalTags = 0;
                CurrentTags = 0;
            }
        };

        /** Console support DLL interface. */
        private ConsoleInterface.DLLInterface ConsoleTools;

        /** File stream feeding binary stream. */
        public FileStream ParserFileStream;
        /** Whether or not the ParserFileStream is in big endian **/
        public bool bParserFileStreamIsBigEndian;
        /** Binary stream to read from. */
        public BinaryReader BinaryStream;
        /** Offset of start of stream in file. */
        public long StartOfStream;

        /** Current index for name index map. */
        private int CurrentNameIndex;
        /** Mapping from name to index. */
        private Dictionary<string, int> NameToIndexMap;
        /** Name table */
        private String[] NameArray;
        /** CallStack array */
        private FCallStack[] CallStackArray;
        /* Summaries associated with callstack, 1:1 mapping with callstack array. */
        private FAllocationSummary[] CallStackSummaryArray;
        /** Unique callstack address array */
        private FCallStackAddress[] CallStackAddressArray;
        /** Summaries associated with callstack addresses, 1:1 mapping with callstack address array */
        private FAllocationSummary[] CallStackAddressSummaryArray;

        /** Pointer to size map for keeping track of allocation sizes */
        private Dictionary<UInt32, FPointerInfo> PointerToPointerInfoMap;

        /** Mapping from filename index to associated allocation summary */
        private Dictionary<int, FAllocationSummary> FilenameIndexToSummaryMap;
        /** Mapping from function index to associated allocation summary */
        private Dictionary<int, FAllocationSummary> FunctionIndexToSummaryMap;

        /** This is the list of data files that we need to read in to get the actual call stacks **/
        public String[] DataFilesArray;

        /** Bitmask of currently active memory tags. */
        private UInt32 CurrentTags;

        private UInt32 TagsToTrack = 0;

        /**
         * Gets a call stack address.
         */
        public FCallStackAddress GetCallStackAddressFromAddressIndex(int AddressIndex)
        {
            return CallStackAddressArray[AddressIndex];
        }

        /**
         * Code dealing with updating stats with malloc information for passed in token.
         * 
         * @param	StreamToken		Token to handle allocation for
         * @param	OriginalTags	Tags to propagate to original tags setting for pointer
         */
        void HandleMalloc(FStreamToken StreamToken, UInt32 OriginalTags)
        {
            // Disregard requests that didn't result in an allocation.
            if (StreamToken.Pointer != 0 &&
                StreamToken.Size > 0)
            {
                // Lookup callstack in array.
                FCallStack CallStack = CallStackArray[StreamToken.CallStackIndex];

                // Keep track of size associated with pointer and also current callstack as we need to update it when freeing the pointer.
                FPointerInfo PointerInfo = new FPointerInfo();
                PointerInfo.Size = StreamToken.Size;
                PointerInfo.CallStackIndex = StreamToken.CallStackIndex;
                PointerInfo.OriginalTags = OriginalTags;
                PointerInfo.CurrentTags = CurrentTags;

                try
                {
                    // Only track new allocations for current tag.
                    if ((PointerInfo.CurrentTags & PointerInfo.OriginalTags & TagsToTrack) != 0 || TagsToTrack == 0)
                    {
                        PointerToPointerInfoMap.Add(StreamToken.Pointer, PointerInfo);

                        // Update callstack summary.
                        CallStackSummaryArray[StreamToken.CallStackIndex].UpdateMalloc(StreamToken.Size);

                        // Update callstack address index summary.
                        foreach (int AddressIndex in CallStack.AddressIndices)
                        {
                            FAllocationSummary AllocationSummary = CallStackAddressSummaryArray[AddressIndex];
                            AllocationSummary.UpdateMalloc(StreamToken.Size);
                        }
                        // Update filename address summary
                        foreach (int FilenameIndex in CallStack.FilenameIndices)
                        {
                            FAllocationSummary AllocationSummary = FilenameIndexToSummaryMap[FilenameIndex];
                            AllocationSummary.UpdateMalloc(StreamToken.Size);
                        }
                        // Update function address summary
                        foreach (int FunctionIndex in CallStack.FunctionIndices)
                        {
                            FAllocationSummary AllocationSummary = FunctionIndexToSummaryMap[FunctionIndex];
                            AllocationSummary.UpdateMalloc(StreamToken.Size);
                        }
                    }
                }
                catch (System.ArgumentException)
                {
                    // Log the callstack to allow debugging bad application behavior.
					LogCallStack(StreamToken.CallStackIndex);
                }
            }
        }

        /**
         * Helper function called from other HandleFree implementation.
         */
        void HandleFree(int CallStackIndex, int Size, bool bIsOriginalCallSite)
        {
            FCallStack CallStack = CallStackArray[CallStackIndex];

            // Update callstack summary.
            CallStackSummaryArray[CallStackIndex].UpdateFree(Size, bIsOriginalCallSite);

            // Update callstack address index summary.
            foreach (int AddressIndex in CallStack.AddressIndices)
            {
                FAllocationSummary AllocationSummary = CallStackAddressSummaryArray[AddressIndex];
                AllocationSummary.UpdateFree(Size, bIsOriginalCallSite);
            }
            // Update filename address summary
            foreach (int FilenameIndex in CallStack.FilenameIndices)
            {
                FAllocationSummary AllocationSummary = FilenameIndexToSummaryMap[FilenameIndex];
                AllocationSummary.UpdateFree(Size, bIsOriginalCallSite);
            }
            // Update function address summary
            foreach (int FunctionIndex in CallStack.FunctionIndices)
            {
                FAllocationSummary AllocationSummary = FunctionIndexToSummaryMap[FunctionIndex];
                AllocationSummary.UpdateFree(Size, bIsOriginalCallSite);
            }
        }

        /**
         * Code dealing with updating stats with free information for passed in token. 
         * 
         * @return	OriginalTags property of freed pointer.
         */
        UInt32 HandleFree(FStreamToken StreamToken)
        {
            UInt32 OriginalTags = CurrentTags;
            //@todo: We currently seem to free/ realloc pointers we didn't allocate, which seems to be a bug that needs to be fixed in the engine.			
            // More or less gracefully handle freeing pointers that either never have been allocated or are already freed.
            try
            {
                // Disregard freeing a NULL pointer.
                if (PointerToPointerInfoMap.ContainsKey(StreamToken.Pointer))
                {
                    // Look up size and callstack associated with this pointer.
                    FPointerInfo PointerInfo = PointerToPointerInfoMap[StreamToken.Pointer];
                    int Size = PointerInfo.Size;
                    int AllocationCallStackIndex = PointerInfo.CallStackIndex;
                    int FreeCallStackIndex = StreamToken.CallStackIndex;
                    OriginalTags = PointerInfo.OriginalTags;

                    // Only track allocations we also handled in Malloc.
                    // (Always handle frees even if allocation doesn't match tag)
                    //if( (PointerInfo.OriginalTags & TagsToTrack) != 0 ||TagsToTrack == 0 )
                    {
                        // Handle free for both original callsite and current one.
                        HandleFree(AllocationCallStackIndex, Size, true);
                        HandleFree(FreeCallStackIndex, Size, false);
                    }

                    // Remove mapping as the pointer is now freed.
                    PointerToPointerInfoMap.Remove(StreamToken.Pointer);
                }
            }
            catch (System.Collections.Generic.KeyNotFoundException)
            {
                // Log the callstack to allow debugging bad application behavior.
                LogCallStack(StreamToken.CallStackIndex);
            }
            return OriginalTags;
        }

        /**
         * Helper function for logging a callstack given the passed in index.
         */
        public void LogCallStack(int CallStackIndex)
        {
            foreach (int AddressIndex in CallStackArray[CallStackIndex].SortedAddressIndices)
            {
                if (AddressIndex != -1)
                {
                    FCallStackAddress Address = CallStackAddressArray[AddressIndex];
                    Console.WriteLine("{0} {1}", NameArray[Address.FilenameIndex], NameArray[Address.FunctionIndex]);
                }
            }
        }

        /**
         * Helper to efficiently add an Key/ Value pair to a "multi-map". The multi-map is simulated by a mapping
         * from Key to array of Values.
         */
        void AddIndexToMap(Dictionary<int, List<int>> Map, int Key, int Index)
        {
            if (Map.ContainsKey(Key))
            {
                List<int> Indices = Map[Key];
                if (!Indices.Contains(Index))
                {
                    Indices.Add(Index);
                }
            }
            // Add new filename -> function index mapping.
            else
            {
                List<int> Indices = new List<int>();
                Indices.Add(Index);
                Map.Add(Key, Indices);
            }
        }

        /**
         * Sort helper for filenames, sorting by active size.
         */
        public int FilenameComparer(int A, int B)
        {
            return (int)(FilenameIndexToSummaryMap[B].SizeActive - FilenameIndexToSummaryMap[A].SizeActive);
        }

        /**
         * Sort helper for functions, sorting by active size.
         */
        public int FunctionComparer(int A, int B)
        {
            return (int)(FunctionIndexToSummaryMap[B].SizeActive - FunctionIndexToSummaryMap[A].SizeActive);
        }

        /**
         * Sort helper for addresses, sorting by active size.
         */
        public int AddressComparer(int A, int B)
        {
            return (int)(CallStackAddressSummaryArray[B].SizeActive - CallStackAddressSummaryArray[A].SizeActive);
        }

        public void InitParser(String Filename, UInt32 InTagsToTrack, bool bDeferSymbolLookup)
        {
            TagsToTrack = InTagsToTrack;

            // Create binary reader and file info object from filename.
            ParserFileStream = File.OpenRead(Filename);

            BinaryStream = new BinaryReader(ParserFileStream);
            // Read header.
            FProfileDataHeader ProfileDataHeader = new FProfileDataHeader(BinaryStream);

            const UInt32 MagicNumber = 0xDA15F7D8;
            if (ProfileDataHeader.Magic != MagicNumber)
            {
                //try again with big endian reader
                ParserFileStream.Seek(0, SeekOrigin.Begin);
                BinaryStream = new BinaryReaderBigEndian(ParserFileStream);
                bParserFileStreamIsBigEndian = true;
                ProfileDataHeader = new FProfileDataHeader(BinaryStream);
				if (ProfileDataHeader.Magic == 0)
				{
					System.Console.WriteLine("PS3 Memory Log Invalid Header: possibly closed during serializing?");
					throw new InvalidDataException();
				}
            }

            if (ProfileDataHeader.Magic == MagicNumber)
            {
                // Initialize arrays and helpers.
                CurrentNameIndex = 0;
                NameToIndexMap = new Dictionary<string, int>();
                NameArray = new String[ProfileDataHeader.NameTableEntries];
                CallStackArray = new FCallStack[ProfileDataHeader.CallStackTableEntries];
                CallStackSummaryArray = new FAllocationSummary[ProfileDataHeader.CallStackTableEntries];
                CallStackAddressArray = new FCallStackAddress[ProfileDataHeader.CallStackAddressTableEntries];
                CallStackAddressSummaryArray = new FAllocationSummary[ProfileDataHeader.CallStackAddressTableEntries];
                FilenameIndexToSummaryMap = new Dictionary<int, FAllocationSummary>();
                FunctionIndexToSummaryMap = new Dictionary<int, FAllocationSummary>();
                PointerToPointerInfoMap = new Dictionary<UInt32, FPointerInfo>();
                DataFilesArray = new String[ProfileDataHeader.DataFilesNum];


                // Keep track of current position as it's where the stream starts.
                StartOfStream = ParserFileStream.Position;

                // Seek to name table and serialize it.
                ParserFileStream.Seek(ProfileDataHeader.NameTableOffset, SeekOrigin.Begin);
                for (int NameIndex = 0; NameIndex < ProfileDataHeader.NameTableEntries; NameIndex++)
                {
                    UInt32 Length = BinaryStream.ReadUInt32();
                    NameArray[NameIndex] = new String(BinaryStream.ReadChars((int)Length));
                }

                // We're dealing with a console platform.
                if (ProfileDataHeader.Platform != 1)
                {
                    ConsoleTools = new ConsoleInterface.DLLInterface();
                    ConsoleTools.ActivatePlatform("PS3");
                    ConsoleTools.LoadSymbols(0, "D:\\DEV\\UT3\\UnrealEngine3-UT3-PS3\\Binaries\\UTGAME-PS3Release.xelf");

                    // Propagate existing name entries.
                    for (int NameIndex = 0; NameIndex < NameArray.Length; NameIndex++)
                    {
                        NameToIndexMap.Add(NameArray[NameIndex], NameIndex);
                    }
                    CurrentNameIndex = NameArray.Length;
                }

                // Seek to callstack address array and serialize it.                
                ParserFileStream.Seek(ProfileDataHeader.CallStackAddressTableOffset, SeekOrigin.Begin);
                for (int AddressIndex = 0; AddressIndex < ProfileDataHeader.CallStackAddressTableEntries; AddressIndex++)
                {
                    CallStackAddressArray[AddressIndex] = new FCallStackAddress(BinaryStream, this, !bDeferSymbolLookup);
                    CallStackAddressSummaryArray[AddressIndex] = new FAllocationSummary();
                }

                // Seek to callstack array and serialize it.
                ParserFileStream.Seek(ProfileDataHeader.CallStackTableOffset, SeekOrigin.Begin);
                for (int CallStackIndex = 0; CallStackIndex < ProfileDataHeader.CallStackTableEntries; CallStackIndex++)
                {
                    CallStackArray[CallStackIndex] = new FCallStack(BinaryStream, this);
                    CallStackSummaryArray[CallStackIndex] = new FAllocationSummary();
                }

                // Seek to data files and serialize it.
                ParserFileStream.Seek( ProfileDataHeader.DataFilesOffset, SeekOrigin.Begin );
                for ( int DataFilesIndex = 0; DataFilesIndex < ProfileDataHeader.DataFilesNum; DataFilesIndex++ )
                {
                    UInt32 Length = BinaryStream.ReadUInt32();
                    DataFilesArray[DataFilesIndex] = new String( BinaryStream.ReadChars( (int)Length ) );
                }


                // Unload symbols if they were loaded above.
                if (ConsoleTools != null)
                {
                    // Create new name array based on dictionary.
                    NameArray = new String[CurrentNameIndex];
                    foreach (KeyValuePair<string, int> NameMapping in NameToIndexMap)
                    {
                        NameArray[NameMapping.Value] = NameMapping.Key;
                    }

                    // Unload symbols now that we're done with them.
                    if( !bDeferSymbolLookup )
                    {
                        ConsoleTools.UnloadAllSymbols();
                    }
                }

                // Seek to beginning of stream.
                ParserFileStream.Seek(StartOfStream, SeekOrigin.Begin);
            }
        }

        public void ParseTokenStream()
        {
            // Seek to beginning of stream.
            ParserFileStream.Seek(StartOfStream, SeekOrigin.Begin);

            // Parse tokens till we reach the end of the stream.
            FStreamToken StreamToken = new FStreamToken();
            while (StreamToken.ReadNextToken(this))
            {
                FCallStack CallStack = CallStackArray[StreamToken.CallStackIndex];

                switch (StreamToken.Type)
                {
                    // Malloc
                    case EProfilingPayloadType.TYPE_Malloc:
                        HandleMalloc(StreamToken, CurrentTags);
                        break;
                    // Free
                    case EProfilingPayloadType.TYPE_Free:
                        HandleFree(StreamToken);
                        break;
                    // Realloc
                    case EProfilingPayloadType.TYPE_Realloc:
                        UInt32 OriginalTags = HandleFree(StreamToken);
                        StreamToken.Pointer = StreamToken.NewPointer;
                        HandleMalloc(StreamToken, OriginalTags);
                        break;
                    // Status/ payload.
                    case EProfilingPayloadType.TYPE_Other:
                        break;
                    // Unhandled.
                    default:
                        throw new InvalidDataException();
                }
            }
        }

        public class DumpedInfo
        {
            public DumpedInfo(int InTotalSize, int InTotalCount)
            {
                TotalSize = InTotalSize;
                TotalCount = InTotalCount;
            }

            public int TotalSize = 0;
            public int TotalCount = 0;
        }


        /**
         * This function will take two call stacks and then evaluate them to make certain they are equal.
         * Equal is defined as:
         * ( ( Address.FunctionIndex == Address2.FunctionIndex )
         *   && ( Address.LineNumber == Address2.LineNumber )
         *   && ( Address.FilenameIndex == Address2.FilenameIndex )
         *    )
         *
         * We need to do this because some callstacks are really huge and will be slightly different towards 
         * the top of the call chain even tho they are all ending up at the same leaking mem location.  So in those
         * cases the CRC will not be correct.
         * 
         * So we count the number of entries in one callstack  and then compare that number to the number that matched in the other callstack.
         * If those match we are the same leak.  Also because we are not able to store infinite storage on the backend we say:  
         * if you match FProfileDataHeader.BackTraceDepth-2 then you also match
         * 
         */
        public bool IsSameCallStack(FCallStack A, FCallStack B)
        {
            bool Retval = false;

            int ACount = 0;
            int BCount = 0;

            foreach (int AddressIndex in A.SortedAddressIndices)
            {
                if (AddressIndex != -1)
                {
                    ACount++;
                    FCallStackAddress Address = CallStackAddressArray[AddressIndex];
                    foreach (int AddressIndex2 in B.SortedAddressIndices)
                    {
                        // TextStream.WriteLine("{0} Line={1} [{2}]", NameArray[Address.FunctionIndex], Address.LineNumber, NameArray[Address.FilenameIndex]);

                        if (AddressIndex2 != -1)
                        {
                            FCallStackAddress Address2 = CallStackAddressArray[AddressIndex2];

                            if ((Address.FunctionIndex == Address2.FunctionIndex)
                                && (Address.LineNumber == Address2.LineNumber)
                                && (Address.FilenameIndex == Address2.FilenameIndex)
                                )
                            {
                                BCount++;
                                //Console.WriteLine(" part manual match {0} {1} {2}", Address.FunctionIndex, Address.LineNumber, Address.FilenameIndex);
                                break;
                            }
                        }
                    }
                }
            }

            if ((ACount == BCount)
                || (BCount > (FProfileDataHeader.BackTraceDepth - 5))  // some callstacks are really huge and will be slightly different towards the top of the call chain even tho they are all ending up at the same leaking mem location
                )
            {
                Retval = true;
            }
            else
            {
                // Console.WriteLine(" counts did not match {0} {1}", ACount, BCount);
            }

            return Retval;
        }



        public void ParseFileToDump(string Filename,FStreamParser StreamParser, bool bNeedsSymbolLookup)
        {
            string DumpFilename = Filename + "-" + TagsToTrack + ".dump";

            List<UInt32> DumpedPointerPtrs = new List<UInt32>();
            Dictionary<UInt32, DumpedInfo> DumpedPointerInfos = new Dictionary<UInt32, DumpedInfo>();

            StreamWriter TextStream = new StreamWriter(DumpFilename);
            
            ParseTokenStream();

            TextStream.WriteLine("TAGS USED:");
            for (int i = 0; i < 32; i++)
            {
                if ((TagsToTrack & (1 << i)) != 0)
                {
                    TextStream.WriteLine("TAG={0}", i);
                }
            }
            foreach (KeyValuePair<UInt32, FPointerInfo> PtrEnty in PointerToPointerInfoMap)
            {
                FPointerInfo PtrInfo = PtrEnty.Value;
                if (PtrInfo.Size > 0)
                {
                    bool bAlreadyDumped = false;
                    foreach (UInt32 DumpedEntryPtr in DumpedPointerPtrs)
                    {
                        FPointerInfo DumpedPtrInfo;
                        if (PointerToPointerInfoMap.TryGetValue(DumpedEntryPtr, out DumpedPtrInfo))
                        {
                            FCallStack DumpedCallStack = CallStackArray[DumpedPtrInfo.CallStackIndex];
                            FCallStack CallStack = CallStackArray[PtrInfo.CallStackIndex];

                            // don't delete commented out to check in but want to integrate this into the tool
                            //bool bCRCMatched = (DumpedCallStack.CRC == CallStack.CRC);
                            //bool bManualWalkOfCallStackMatched = IsSameCallStack(DumpedCallStack, CallStack);

                            //if (bManualWalkOfCallStackMatched == true)

                            if( DumpedCallStack.CRC == CallStack.CRC )
                            {
                                bAlreadyDumped = true;
                                DumpedInfo DumpInfo;
                                if (DumpedPointerInfos.TryGetValue(DumpedEntryPtr, out DumpInfo))
                                {
                                    DumpInfo.TotalCount++;
                                    DumpInfo.TotalSize += PtrInfo.Size;
                                }
                                break;
                            }


                            // don't delete commented out to check in but want to integrate this into the tool
                            //// do test to see if the crc and the actual manual looking at the stack match
                            //if (bCRCMatched == true)
                            //{
                            //    //Console.WriteLine(" CRC Match: {0} {1}", DumpedEntryPtr, PtrInfo.Size);
                            //}

                            //if (bManualWalkOfCallStackMatched == true)
                            //{
                            //    // Console.WriteLine(" ManualCallStack Match: {0} {1}", DumpedEntryPtr, PtrInfo.Size);
                            //}

                            //if (bCRCMatched != bManualWalkOfCallStackMatched)
                            //{
                            //    Console.WriteLine(" BADNESSSSSS Match: {0} {1} {2} {3}", bCRCMatched, bManualWalkOfCallStackMatched, DumpedEntryPtr, PtrInfo.Size);
                            //   Console.WriteLine(CallstackToString(DumpedCallStack));
                            //    Console.WriteLine(CallstackToString(CallStack));
                            //}

                            //if (bManualWalkOfCallStackMatched == true)
                            //{
                            //    break;
                            //}
                        }
                    }

                    if (!bAlreadyDumped)
                    {
                        DumpedPointerPtrs.Add(PtrEnty.Key);
                        DumpedInfo DumpInfo = new DumpedInfo(PtrInfo.Size, 1);
                        DumpedPointerInfos.Add(PtrEnty.Key, DumpInfo);
                    }
                }
            }

            foreach (KeyValuePair<UInt32, DumpedInfo> PtrToPrint in DumpedPointerInfos)
            {
                DumpedInfo DumpInfo = PtrToPrint.Value;
                TextStream.WriteLine("");
                TextStream.WriteLine("Size={0} NumAllocs={1}", DumpInfo.TotalSize, DumpInfo.TotalCount);

                FPointerInfo PtrInfo;
                if (PointerToPointerInfoMap.TryGetValue(PtrToPrint.Key, out PtrInfo))
                {
                    FCallStack CallStack = CallStackArray[PtrInfo.CallStackIndex];
                    foreach (int AddressIndex in CallStack.SortedAddressIndices)
                    {
                        if (AddressIndex != -1)
                        {
                            FCallStackAddress Address = CallStackAddressArray[AddressIndex];                                          
                            if( bNeedsSymbolLookup && 
                                StreamParser != null && 
                                StreamParser.ConsoleTools != null )
                            {
                               string AddrFilename = "";
                               string AddrFunction = "";
                               Int32 AddrLineNumber = 0;

                               StreamParser.ConsoleTools.ResolveAddressToSymboInfo((uint)Address.ProgramCounter, ref AddrFilename, ref AddrFunction, ref AddrLineNumber);
                                TextStream.WriteLine("{0} Line={1} [{2}]", AddrFunction, AddrLineNumber, AddrFilename);
                            }
                            else
                            {
                                TextStream.WriteLine("{0} Line={1} [{2}]", NameArray[Address.FunctionIndex], Address.LineNumber, NameArray[Address.FilenameIndex]);
                            }
                        }
                    }
                }
            }

            if( bNeedsSymbolLookup &&
                StreamParser != null &&
                StreamParser.ConsoleTools != null)
            {
                ConsoleTools.UnloadAllSymbols();
            }

            TextStream.Close();
        }


        /** Helper function to just print out the call stack **/
        public string CallstackToString(FCallStack CallStack)
        {
            StringBuilder RetvalBuilder = new StringBuilder();

            foreach (int AddressIndex in CallStack.SortedAddressIndices)
            {
                if (AddressIndex != -1)
                {
                    FCallStackAddress Address = CallStackAddressArray[AddressIndex];
                    RetvalBuilder.Append(NameArray[Address.FunctionIndex] + " ");
                    RetvalBuilder.Append(Address.LineNumber + " ");
                    RetvalBuilder.Append(NameArray[Address.FilenameIndex] + " ");
                    RetvalBuilder.Append("\n");

                    //TextStream.WriteLine("{0} Line={1} [{2}]", NameArray[Address.FunctionIndex], Address.LineNumber, NameArray[Address.FilenameIndex]);
                }
            }


            return RetvalBuilder.ToString();
        }


        /**
         * Parse passed in file into passed in treeview.
         */
        public void ParseFileFunctionLine(FTreeListView FileFunctionLineView)
        {
            // Empty graph as we might be opening a stream for the second time.
            FileFunctionLineView.Items.Clear();

            ParseTokenStream();

            // Suspend display activity while we modify it.
            FileFunctionLineView.BeginUpdate();

            // Mapping from file names to associated functions.
            Dictionary<int, List<int>> FilenameIndexToFunctionIndicesMap = new Dictionary<int, List<int>>();
            // Mapping from function names to associated addresses.
            Dictionary<int, List<int>> FunctionIndexToAddressIndicesMap = new Dictionary<int, List<int>>();

            // Populate mappings with data.
            for (int AddressIndex = 0; AddressIndex < CallStackAddressArray.Length; AddressIndex++)
            {
                FCallStackAddress Address = CallStackAddressArray[AddressIndex];
                AddIndexToMap(FilenameIndexToFunctionIndicesMap, Address.FilenameIndex, Address.FunctionIndex);
                AddIndexToMap(FunctionIndexToAddressIndicesMap, Address.FunctionIndex, AddressIndex);
            }

            // Sort filenames by active size.
            List<int> SortedFilenameIndices = new List<int>();
            Dictionary<int, FAllocationSummary>.KeyCollection FilenameIndexCollection = FilenameIndexToSummaryMap.Keys;
            foreach (int FilenameIndex in FilenameIndexCollection)
            {
                SortedFilenameIndices.Add(FilenameIndex);
            }
            SortedFilenameIndices.Sort(FilenameComparer);

            const string UNREAL_DEV_DIR = "\\Development\\Src\\";
            string NodeName;
            int UnrealDirIndex = 0;

            // Iterate over all filenames, populating graph.
            foreach (int FilenameIndex in SortedFilenameIndices)
            {
                FAllocationSummary FilenameSummary = FilenameIndexToSummaryMap[FilenameIndex];

                // Only add filenames with active allocations above threshold to avoid slowing down everything.
                if (FilenameSummary.SizeActive != 0)
                {
                    NodeName = NameArray[FilenameIndex];
                    UnrealDirIndex = NodeName.IndexOf(UNREAL_DEV_DIR, StringComparison.OrdinalIgnoreCase);
                    if (UnrealDirIndex != -1)
                    {
                        NodeName = NodeName.Substring(UnrealDirIndex + UNREAL_DEV_DIR.Length);
                    }

                    FTreeListViewItem FilenameNode = new FTreeListViewItem(NodeName);
                    FilenameNode.SubItems.Add(FilenameSummary.SizeActive.ToString());
                    FilenameNode.SubItems.Add(FilenameSummary.ActiveAllocationCount.ToString());

                    FNodePayload FilenamePayload = new FNodePayload(NameArray[FilenameIndex], 0, FilenameIndex, -1, -1);
                    FilenameNode.Tag = FilenamePayload;
                    FileFunctionLineView.Items.Add(FilenameNode);

                    // Sort functions by active size.
                    List<int> FunctionIndices = FilenameIndexToFunctionIndicesMap[FilenameIndex];
                    FunctionIndices.Sort(FunctionComparer);

                    // Iterate over all contained functions, adding them to tree.
                    foreach (int FunctionIndex in FunctionIndices)
                    {
                        FAllocationSummary FunctionSummary = FunctionIndexToSummaryMap[FunctionIndex];

                        if (FunctionSummary.SizeActive != 0)
                        {
                            FTreeListViewItem FunctionNode = new FTreeListViewItem(NameArray[FunctionIndex]);
                            FunctionNode.BackColor = System.Drawing.Color.FromArgb(192, 221, 255);
                            FunctionNode.SubItems.Add(FunctionSummary.SizeActive.ToString());
                            FunctionNode.SubItems.Add(FunctionSummary.ActiveAllocationCount.ToString());

                            FNodePayload FunctionPayload = new FNodePayload(NameArray[FilenameIndex], 0, FilenameIndex, FunctionIndex, -1);
                            FunctionNode.Tag = FunctionPayload;
                            FilenameNode.Nodes.Add(FunctionNode);

                            // Sort addresses by active size.
                            List<int> AddressIndices = FunctionIndexToAddressIndicesMap[FunctionIndex];
                            AddressIndices.Sort(AddressComparer);

                            // Iterate over all contained addresses, adding them to tree.
                            foreach (int AddressIndex in AddressIndices)
                            {
                                FAllocationSummary AddressSummary = CallStackAddressSummaryArray[AddressIndex];
                                if (AddressSummary.SizeActive != 0)
                                {
                                    FTreeListViewItem AddressNode = new FTreeListViewItem("Line: " + CallStackAddressArray[AddressIndex].LineNumber.ToString());
                                    AddressNode.BackColor = System.Drawing.Color.FromArgb(192, 255, 221);
                                    AddressNode.SubItems.Add(AddressSummary.SizeActive.ToString());
                                    AddressNode.SubItems.Add(AddressSummary.ActiveAllocationCount.ToString());

                                    FNodePayload AddressPayload = new FNodePayload(NameArray[FilenameIndex], CallStackAddressArray[AddressIndex].LineNumber, FilenameIndex, FunctionIndex, AddressIndex);
                                    AddressNode.Tag = AddressPayload;
                                    FunctionNode.Nodes.Add(AddressNode);
                                }
                            }
                        }

                    }
                }
            }

            // Update is complete, time to render again.
            FileFunctionLineView.EndUpdate();
        }

        private FCallStackEntryNode AddNodeToGraph(FCallStackEntryNode Parent, int AddressIndex)
        {
            // Check whether address is already in graph.
            foreach (FCallStackEntryNode ChildNode in Parent.Nodes)
            {
                // Tag is used to store address index.
                int NodeAddressIndex = (int)ChildNode.Tag;
                // Check whether it matches and return node as new parent if it does.
                if (NodeAddressIndex == AddressIndex)
                {
                    return ChildNode;
                }
            }

            // We need to add address if we've made it here.
            FCallStackAddress Address = CallStackAddressArray[AddressIndex];
            FAllocationSummary AllocationSummary = CallStackAddressSummaryArray[AddressIndex];
            FCallStackEntryNode Node = new FCallStackEntryNode(NameArray[Address.FunctionIndex]);
            Node.SubItems.Add(AllocationSummary.SizeActive.ToString());
            Node.SubItems.Add(AllocationSummary.ActiveAllocationCount.ToString());
            Node.SubItems.Add(NameArray[Address.FilenameIndex]);

            Node.Tag = AddressIndex;
            Parent.Nodes.Add(Node);

            // Return new node as parent for subsequent calls.
            return Node;
        }

        public void ParseCallGraph(FNodePayload Payload, FTreeListView CallGraphView)
        {
            // Avoid redundant work while we're populating it.
            CallGraphView.BeginUpdate();

            // Remove existing data.
            CallGraphView.Items.Clear();

            // Only handle displaying callstack for addresses.
            if (Payload.AddressIndex != -1)
            {
                // Create a list of all callstacks containing address.
                List<int> CallStackIndices = new List<int>();
                for (int CallStackIndex = 0; CallStackIndex < CallStackArray.Length; CallStackIndex++)
                {
                    // Disregard callstacks with no active allocations.
                    if (CallStackSummaryArray[CallStackIndex].ActiveAllocationCount > 0)
                    {
                        // Check fo match.
                        FCallStack CallStack = CallStackArray[CallStackIndex];
                        if (CallStack.AddressIndices.Contains(Payload.AddressIndex))
                        {
                            CallStackIndices.Add(CallStackIndex);
                        }
                    }
                }

                // Display with address as root.
                FCallStackAddress Address = CallStackAddressArray[Payload.AddressIndex];
                FAllocationSummary AllocationSummary = CallStackAddressSummaryArray[Payload.AddressIndex];
                FCallStackEntryNode RootNode = new FCallStackEntryNode(NameArray[Address.FunctionIndex]);
                RootNode.SubItems.Add(AllocationSummary.SizeActive.ToString());
                RootNode.SubItems.Add(AllocationSummary.ActiveAllocationCount.ToString());
                RootNode.SubItems.Add(NameArray[Address.FilenameIndex]);

                RootNode.Tag = Payload.AddressIndex;
                CallGraphView.Items.Add(RootNode);

                // Iterate ove callstacks and add them to graph.
                foreach (int CallStackIndex in CallStackIndices)
                {
                    FCallStack CallStack = CallStackArray[CallStackIndex];

                    // Find index of address
                    int BaseDepth = 0;
                    while (CallStack.SortedAddressIndices[BaseDepth] != Payload.AddressIndex)
                    {
                        BaseDepth++;
                    }

                    // Keep track of each new node's parent.
                    FCallStackEntryNode Parent = RootNode;

                    // Add subsequent addresses to graph.
                    for (int Depth = BaseDepth - 1; Depth >= 0; Depth--)
                    {
                        // Add address with parent at relative depth, returning new node as parent for next depth.
                        Parent = AddNodeToGraph(Parent, CallStack.SortedAddressIndices[Depth]);
                    }
                }
            }

            // We're done updating.
            CallGraphView.EndUpdate();
        }
    }
}