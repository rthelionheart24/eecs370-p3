/**
 * EECS 370 Project 3
 * Pipeline Simulator
 *
 * This fragment should be used to modify your project 1 simulator to simulator
 * a pipeline
 *
 * Make sure *not* to modify printState or any of the associated functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8       /* number of machine registers */

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR will not implemented for Project 3 */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDStruct
{
    int instr;
    int pcPlus1;
} IFIDType;

typedef struct IDEXStruct
{
    int instr;
    int pcPlus1;
    int readRegA;
    int readRegB;
    int offset;
    int hazard_loc1;
    int hazard_reg1;
    int hazard_loc2;
    int hazard_reg2;
} IDEXType;

typedef struct EXMEMStruct
{
    int instr;
    int branchTarget;
    int aluResult;
    int readRegB;
} EXMEMType;

typedef struct MEMWBStruct
{
    int instr;
    int writeData;
} MEMWBType;

typedef struct WBENDStruct
{
    int instr;
    int writeData;
} WBENDType;

typedef struct stateStruct
{
    int pc;
    int instrMem[NUMMEMORY];
    int dataMem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
    IFIDType IFID;
    IDEXType IDEX;
    EXMEMType EXMEM;
    MEMWBType MEMWB;
    WBENDType WBEND;
    int cycles; /* number of cycles run so far */
    int buffer[3];
} stateType;

int field0(int instruction)
{
    return ((instruction >> 19) & 0x7);
}

int field1(int instruction)
{
    return ((instruction >> 16) & 0x7);
}

int field2(int instruction)
{
    return (instruction & 0xFFFF);
}

int opcode(int instruction)
{
    return (instruction >> 22);
}

int convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1 << 15))
    {
        num -= (1 << 16);
    }
    return (num);
}

void printInstruction(int instr)
{

    char opcodeString[10];

    if (opcode(instr) == ADD)
    {
        strcpy(opcodeString, "add");
    }
    else if (opcode(instr) == NOR)
    {
        strcpy(opcodeString, "nor");
    }
    else if (opcode(instr) == LW)
    {
        strcpy(opcodeString, "lw");
    }
    else if (opcode(instr) == SW)
    {
        strcpy(opcodeString, "sw");
    }
    else if (opcode(instr) == BEQ)
    {
        strcpy(opcodeString, "beq");
    }
    else if (opcode(instr) == JALR)
    {
        strcpy(opcodeString, "jalr");
    }
    else if (opcode(instr) == HALT)
    {
        strcpy(opcodeString, "halt");
    }
    else if (opcode(instr) == NOOP)
    {
        strcpy(opcodeString, "noop");
    }
    else
    {
        strcpy(opcodeString, "data");
    }
    printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
           field2(instr));
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (i = 0; i < statePtr->numMemory; i++)
    {
        printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++)
    {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("\tIFID:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IFID.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
    printf("\tIDEX:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IDEX.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
    printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
    printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
    printf("\t\toffset %d\n", statePtr->IDEX.offset);
    printf("\tEXMEM:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->EXMEM.instr);
    printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
    printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
    printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);
    printf("\tMEMWB:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->MEMWB.instr);
    printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);
    printf("\tWBEND:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->WBEND.instr);
    printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
}

int run(stateType state)
{
    while (1)
    {

        printState(&state);

        /* check for halt */
        if (opcode(state.MEMWB.instr) == HALT)
        {
            printf("machine halted\n");
            printf("total of %d cycles executed\n", state.cycles);
            exit(0);
        }

        stateType newState = state;
        newState.cycles++;

        /* --------------------- IF stage --------------------- */
        newState.IFID.pcPlus1 = state.pc + 1;
        newState.IFID.instr = state.instrMem[state.pc];
        newState.pc = state.pc + 1;

        /* --------------------- ID stage --------------------- */

        newState.IDEX.hazard_loc1 = -1;
        newState.IDEX.hazard_reg1 = -1;
        newState.IDEX.hazard_loc2 = -1;
        newState.IDEX.hazard_reg2 = -1;

        if (opcode(state.IFID.instr) == ADD || opcode(state.IFID.instr) == NOR)
        {
            for (size_t i = 0; i < 3; i++)
            {
                if (state.buffer[i] == field0(state.IFID.instr))
                {
                    newState.IDEX.hazard_loc1 = i + 1;
                    newState.IDEX.hazard_reg1 = state.buffer[i];
                    break;
                }
            }
            for (size_t i = 0; i < 3; i++)
            {
                if (state.buffer[i] == field1(state.IFID.instr))
                {
                    newState.IDEX.hazard_loc2 = i + 1;
                    newState.IDEX.hazard_reg2 = state.buffer[i];
                    break;
                }
            }

            newState.buffer[2] = state.buffer[1];
            newState.buffer[1] = state.buffer[0];
            newState.buffer[0] = field2(state.IFID.instr);
        }
        else if (opcode(state.IFID.instr) == LW)
        {
            for (size_t i = 0; i < 3; i++)
            {
                if (state.buffer[i] == field0(state.IFID.instr))
                {
                    newState.IDEX.hazard_loc1 = i + 1;
                    newState.IDEX.hazard_reg1 = state.buffer[i];
                    break;
                }
            }

            newState.buffer[2] = state.buffer[1];
            newState.buffer[1] = state.buffer[0];
            newState.buffer[0] = field1(state.IFID.instr);
        }
        else if (opcode(state.IFID.instr) == SW)
        {
            for (size_t i = 0; i < 3; i++)
            {
                if (state.buffer[i] == field0(state.IFID.instr))
                {
                    newState.IDEX.hazard_loc1 = i + 1;
                    newState.IDEX.hazard_reg1 = state.buffer[i];
                    break;
                }
            }
            for (size_t i = 0; i < 3; i++)
            {
                if (state.buffer[i] == field1(state.IFID.instr))
                {
                    newState.IDEX.hazard_loc2 = i + 1;
                    newState.IDEX.hazard_reg2 = state.buffer[i];
                    break;
                }
            }

            newState.buffer[2] = state.buffer[1];
            newState.buffer[1] = state.buffer[0];
            newState.buffer[0] = -1;
        }
        else if (opcode(state.IFID.instr) == BEQ)
        {
            for (size_t i = 0; i < 3; i++)
            {
                if (state.buffer[i] == field0(state.IFID.instr))
                {
                    newState.IDEX.hazard_loc1 = i + 1;
                    newState.IDEX.hazard_reg1 = state.buffer[i];
                    break;
                }
            }
            for (size_t i = 0; i < 3; i++)
            {
                if (state.buffer[i] == field1(state.IFID.instr))
                {
                    newState.IDEX.hazard_loc2 = i + 1;
                    newState.IDEX.hazard_reg2 = state.buffer[i];
                    break;
                }
            }

            newState.buffer[2] = state.buffer[1];
            newState.buffer[1] = state.buffer[0];
            newState.buffer[0] = -1;
        }
        else if (opcode(state.IFID.instr) == NOOP)
        {
            newState.buffer[2] = state.buffer[1];
            newState.buffer[1] = state.buffer[0];
            newState.buffer[0] = -1;
        }

        newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
        newState.IDEX.instr = state.IFID.instr;
        newState.IDEX.readRegA = state.reg[field0(state.IFID.instr)];
        newState.IDEX.readRegB = state.reg[field1(state.IFID.instr)];
        newState.IDEX.offset = convertNum(field2(state.IFID.instr));

        if (opcode(state.IDEX.instr) == LW && (newState.IDEX.hazard_loc1 == 1 || newState.IDEX.hazard_loc2 == 1))
        {
            newState.IDEX.instr = NOOPINSTRUCTION;
            newState.IFID = state.IFID;
            newState.pc = state.pc;

            newState.IDEX.hazard_loc1 = newState.IDEX.hazard_loc1 == 1 ? newState.IDEX.hazard_loc1 + 1 : newState.IDEX.hazard_loc1;
            newState.IDEX.hazard_loc2 = newState.IDEX.hazard_loc2 == 1 ? newState.IDEX.hazard_loc2 + 1 : newState.IDEX.hazard_loc2;
        }

        /* --------------------- EX stage --------------------- */
        // ! Data/control hazard portion needed

        newState.EXMEM.instr = state.IDEX.instr;
        newState.EXMEM.readRegB = state.IDEX.readRegB;

        int src1 = state.IDEX.readRegA;
        int src2 = state.IDEX.readRegB;
        int offset = state.IDEX.offset;

        if (opcode(newState.EXMEM.instr) == ADD || opcode(newState.EXMEM.instr) == NOR)
        {
            if (state.IDEX.hazard_loc1 == 1)
            {
                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.EXMEM.aluResult : src1;
            }
            else if (state.IDEX.hazard_loc1 == 2)
            {

                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.MEMWB.writeData : src1;
            }
            else if (state.IDEX.hazard_loc1 == 3)
            {

                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.WBEND.writeData : src1;
            }

            if (state.IDEX.hazard_loc2 == 1)
            {
                src2 = field1(newState.EXMEM.instr) == state.IDEX.hazard_reg2 ? state.EXMEM.aluResult : src2;
            }
            else if (state.IDEX.hazard_loc2 == 2)
            {

                src2 = field1(newState.EXMEM.instr) == state.IDEX.hazard_reg2 ? state.MEMWB.writeData : src2;
            }
            else if (state.IDEX.hazard_loc2 == 3)
            {

                src2 = field1(newState.EXMEM.instr) == state.IDEX.hazard_reg2 ? state.WBEND.writeData : src2;
            }

            newState.EXMEM.aluResult = opcode(newState.EXMEM.instr) == ADD ? src1 + src2 : ~(src1 | src2);
        }

        else if (opcode(newState.EXMEM.instr) == LW)
        {

            if (state.IDEX.hazard_loc1 == 1)
            {
                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.EXMEM.aluResult : src1;
            }
            else if (state.IDEX.hazard_loc1 == 2)
            {
                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.MEMWB.writeData : src1;
            }
            else if (state.IDEX.hazard_loc1 == 3)
            {
                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.WBEND.writeData : src1;
            }

            newState.EXMEM.aluResult = src1 + offset;
        }
        else if (opcode(newState.EXMEM.instr) == SW)
        {

            if (state.IDEX.hazard_loc1 == 1)
            {
                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.EXMEM.aluResult : src1;
            }
            else if (state.IDEX.hazard_loc1 == 2)
            {
                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.MEMWB.writeData : src1;
            }
            else if (state.IDEX.hazard_loc1 == 3)
            {
                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.WBEND.writeData : src1;
            }

            if (state.IDEX.hazard_loc2 == 1)
            {
                newState.EXMEM.readRegB = field1(newState.EXMEM.instr) == state.IDEX.hazard_reg2 ? state.EXMEM.aluResult : src2;
            }
            else if (state.IDEX.hazard_loc2 == 2)
            {
                newState.EXMEM.readRegB = field1(newState.EXMEM.instr) == state.IDEX.hazard_reg2 ? state.MEMWB.writeData : src2;
            }
            else if (state.IDEX.hazard_loc2 == 3)
            {
                newState.EXMEM.readRegB = field1(newState.EXMEM.instr) == state.IDEX.hazard_reg2 ? state.WBEND.writeData : src2;
            }

            newState.EXMEM.aluResult = src1 + offset;
        }
        else if (opcode(newState.EXMEM.instr) == BEQ)
        {
            if (state.IDEX.hazard_loc1 == 1)
            {
                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.EXMEM.aluResult : src1;
            }
            else if (state.IDEX.hazard_loc1 == 2)
            {
                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.MEMWB.writeData : src1;
            }
            else if (state.IDEX.hazard_loc1 == 3)
            {
                src1 = field0(newState.EXMEM.instr) == state.IDEX.hazard_reg1 ? state.WBEND.writeData : src1;
            }

            if (state.IDEX.hazard_loc2 == 1)
            {
                src2 = field1(newState.EXMEM.instr) == state.IDEX.hazard_reg2 ? state.EXMEM.aluResult : src2;
            }
            else if (state.IDEX.hazard_loc2 == 2)
            {
                src2 = field1(newState.EXMEM.instr) == state.IDEX.hazard_reg2 ? state.MEMWB.writeData : src2;
            }
            else if (state.IDEX.hazard_loc2 == 3)
            {
                src2 = field1(newState.EXMEM.instr) == state.IDEX.hazard_reg2 ? state.WBEND.writeData : src2;
            }

            if (src1 == src2)
            {
                newState.EXMEM.aluResult = 1;
            }
            else
            {
                newState.EXMEM.aluResult = 0;
            }

            newState.EXMEM.branchTarget = state.IDEX.pcPlus1 + offset;
        }
        else
        {
        }

        /* --------------------- MEM stage --------------------- */
        // ! Data/control hazard portion needed

        if (opcode(state.EXMEM.instr) == ADD || opcode(state.EXMEM.instr) == NOR)
        {
            newState.MEMWB.writeData = state.EXMEM.aluResult;
        }
        else if (opcode(state.EXMEM.instr) == LW)
        {
            newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];
        }
        else if (opcode(state.EXMEM.instr) == SW)
        {
            newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.readRegB;
        }
        else if (opcode(state.EXMEM.instr) == BEQ)
        {
            if (state.EXMEM.aluResult == 1)
            {
                newState.IFID.instr = NOOPINSTRUCTION;
                newState.IDEX.instr = NOOPINSTRUCTION;
                newState.EXMEM.instr = NOOPINSTRUCTION;

                newState.buffer[0] = -1;
                newState.buffer[1] = -1;
                newState.buffer[2] = -1;

                newState.IDEX.hazard_loc1 = -1;
                newState.IDEX.hazard_loc2 = -1;
                newState.IDEX.hazard_reg1 = -1;
                newState.IDEX.hazard_reg2 = -1;

                newState.pc = state.EXMEM.branchTarget;
            }
            // else
            // {
            //     newState.pc = state.pc + 1;
            // }
        }

        newState.MEMWB.instr = state.EXMEM.instr;

        /* --------------------- WB stage --------------------- */

        if (opcode(state.MEMWB.instr) == ADD || opcode(state.MEMWB.instr) == NOR)
        {
            int dst = field2(state.MEMWB.instr);
            newState.reg[dst] = state.MEMWB.writeData;
        }
        else if (opcode(state.MEMWB.instr) == LW)
        {
            int dst = field1(state.MEMWB.instr);
            newState.reg[dst] = state.MEMWB.writeData;
        }

        newState.WBEND.instr = state.MEMWB.instr;
        newState.WBEND.writeData = state.MEMWB.writeData;

        state = newState; /* this is the last statement before end of the loop.
        It marks the end of the cycle and updates the
        current state with the values calculated in this
        cycle */
    }
}

int main(int argc, char *argv[])
{
    char line[1000];
    FILE *filePtr;
    stateType state;

    filePtr = fopen(argv[1], "r");

    if (filePtr == NULL)
    {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    for (size_t i = 0; i < NUMMEMORY; i++)
    {
        state.instrMem[i] = 0;
        state.dataMem[i] = 0;
    }

    /* read the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, 1000, filePtr) != NULL;
         state.numMemory++)
    {

        if (sscanf(line, "%d", state.instrMem + state.numMemory) != 1)
        {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.instrMem[state.numMemory]);
    }

    for (size_t d = 0; d < NUMMEMORY; d++)
    {
        state.dataMem[d] = state.instrMem[d];
    }

    state.pc = 0;
    state.cycles = 0;
    state.buffer[0] = -1;
    state.buffer[1] = -1;
    state.buffer[2] = -1;

    for (size_t r = 0; r < NUMREGS; r++)
    {
        state.reg[r] = 0;
    }

    state.IFID.instr = NOOPINSTRUCTION;
    state.IFID.pcPlus1 = 0;

    state.IDEX.instr = NOOPINSTRUCTION;
    state.IDEX.pcPlus1 = 0;
    state.IDEX.readRegA = 0;
    state.IDEX.readRegB = 0;
    state.IDEX.offset = 0;
    state.IDEX.hazard_loc1 = -1;
    state.IDEX.hazard_loc2 = -1;
    state.IDEX.hazard_reg1 = -1;
    state.IDEX.hazard_reg2 = -1;

    state.EXMEM.instr = NOOPINSTRUCTION;
    state.EXMEM.aluResult = 0;
    state.EXMEM.branchTarget = 0;
    state.EXMEM.readRegB = 0;

    state.MEMWB.instr = NOOPINSTRUCTION;
    state.MEMWB.writeData = 0;

    state.WBEND.instr = NOOPINSTRUCTION;
    state.WBEND.writeData = 0;

    run(state);

    return (0);
}