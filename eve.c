/*
 **********************************
 * C implementation of converting *
 * .memon files to .eve files.    *
 *        By fishguy6564          *
 **********************************
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "eve.h"

u32* measures;
u32* hakus;
u32 measure_amount = 0;
u32 haku_amount = 0;

u32 toEveTail[12] = {
    0x4,
    0x7,
    0x5,
    0x6,
    0x8,
    0xB,
    0x9,
    0xA,
    0xC,
    0xF,
    0xD,
    0xE
};

u32 max(int a, int b){
    return (a > b) ? a : b;
}

u32 timingConversion(u32 tick, float BPM, float offset, u32 resolution){
    return round(((60*tick)/(BPM*resolution)-offset)*300);
}

u32 toEveBPM(float BPM){
    return (u32)((60 * (u32)pow(10, 6))/ BPM);
}

void writeToFile(FILE* file, u32 tick, char* keyword, u32 data){
    fprintf(file, "%*d,%-8s,%*d\n",8, tick, keyword,  8, data);
}

u32 getLongNoteInterval(MemonInfo memon_info){
    u32 maxInterval = 0;
    for(int i = 0; i < memon_info.noteCount; i++){
        if(memon_info.notes[i].noteLength > 0){
            u32 interval = memon_info.notes[i].noteLength + memon_info.notes[i].timing;
            if(interval > maxInterval) maxInterval = interval;
        }
    }
    return maxInterval;
}

void addHaku(u32 tick){
    haku_amount++;
    hakus = (u32*)realloc(hakus, haku_amount * sizeof(u32));
    hakus[haku_amount - 1] = tick;
}

void addMeasure(u32 tick){
    measure_amount++;
    measures = (u32*)realloc(measures, measure_amount * sizeof(u32));
    measures[measure_amount - 1] = tick;
}

/*
 **************************************************
 * Does all the memon to eve conversion stuff,    *
 * although in a pretty messy way, but it gets    *
 * the job done. Thanks to Stepland's script for  *
 * documentation on how to convert the timing of  *
 * notes properly.                                *
 **************************************************
 */
void convert(char* filename, MemonInfo memon_info){

    FILE* outfile;
    outfile = fopen(filename, "w");

    float BPM = memon_info.info.BPM;
    float offset = memon_info.info.offset;
    u32 resolution = memon_info.resolution;
    u32 noteCount = memon_info.noteCount;
    u32 eveBPM = toEveBPM(memon_info.info.BPM);
    u32 skippedBeats = (u32)max(0, ceil(offset * BPM/60));
    u32 current_measure = 0;
    u32 current_haku = 0;
    u32 last_note_measure;

    u32 startingBeat = timingConversion(resolution*skippedBeats, BPM, offset, resolution);
    u32 longInterval = getLongNoteInterval(memon_info);
    u32 lastNote = memon_info.notes[noteCount - 1].timing;

    //Determines the end timing based on a long note or normal note
    if(longInterval > lastNote) lastNote = longInterval;
    last_note_measure = (lastNote / resolution - skippedBeats) / 4;

    //Calculates the tick the END note should be placed
    u32 endTick = timingConversion((resolution*(4*(last_note_measure+2)+skippedBeats)), BPM, offset, resolution);

    //Gets ticks for HAKU and MEASURE and appends them to a list
    for(int i = 0; i < last_note_measure+3; i++){
        addMeasure(timingConversion((((i*4)+skippedBeats)*resolution),
                                    BPM, offset, resolution));
        for(int j = 0; j < 4; j++){
            addHaku(timingConversion((((i*4)+skippedBeats+j)*resolution),
                                     BPM, offset, resolution));
        }
    }

    //Setting beginning of eve file
    writeToFile(outfile, measures[current_measure++], "MEASURE", 0);
    writeToFile(outfile, hakus[current_haku++], "HAKU", 0);
    writeToFile(outfile, 0, "TEMPO", eveBPM);

    //Formats .eve file
    for(int i = 0; i < noteCount; i++){
        u32 tick = timingConversion(memon_info.notes[i].timing, BPM, offset, resolution);
        if(tick >= startingBeat){
            u32 measureVal = 0;
            u32 hakuVal = 0;

            //Determines where the HAKU and MEASURE should be placed
            while(1){
                measureVal = measures[current_measure++];
                hakuVal = hakus[current_haku++];

                if(tick < measureVal && tick < hakuVal){
                    current_measure--;
                    current_haku--;
                    break;
                }

                if(measureVal > hakuVal){
                    writeToFile(outfile, hakuVal, "HAKU", 0);
                    if(tick > measureVal) writeToFile(outfile, measureVal, "MEASURE", 0);
                    else current_measure--;
                }else{
                    writeToFile(outfile, measureVal, "MEASURE", 0);
                    if(tick > hakuVal) writeToFile(outfile, hakuVal, "HAKU", 0);
                    else current_haku--;
                }
            }

            //Writes normal and long notes
            if(memon_info.notes[i].noteLength == 0){
                writeToFile(outfile, tick, "PLAY", memon_info.notes[i].panel);
            }
            else{
               u32 length = (u32)((memon_info.notes[i].noteLength * 60 * 300) / (u32)(resolution*BPM));
               u32 tailValue = toEveTail[memon_info.notes[i].tail];
               u32 longValue = length * 0x100 + tailValue * 0x10 + memon_info.notes[i].panel;
               writeToFile(outfile, tick, "LONG", longValue);
            }
        }
    }

    //Write out remaining HAKU and MEASURE if any remain
    u32 measureVal = 0;
    u32 hakuVal = 0;
    while(1){
        if(current_measure >= measure_amount && current_haku < haku_amount){
            while(current_haku < haku_amount) writeToFile(outfile, hakus[current_haku++], "HAKU", 0);
            break;
        }
        if(current_measure >= measure_amount && current_haku >= haku_amount) break;

        if(current_measure < measure_amount) measureVal = measures[current_measure++];
        if(current_haku < haku_amount) hakuVal = hakus[current_haku++];

        if(measureVal > hakuVal){
            writeToFile(outfile, hakuVal, "HAKU", 0);
            current_measure--;
        }else{
            if(measureVal == endTick) writeToFile(outfile, endTick, "END", 0); //END is always right before MEASURE with the same tick (?)
            writeToFile(outfile, measureVal, "MEASURE", 0);
            current_haku--;
        }
    }

    fclose(outfile);
}
