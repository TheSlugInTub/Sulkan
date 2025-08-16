#pragma once

typedef struct 
{ 
    double startTime;
} skTimer;

void skTimer_Start(skTimer* timer);
void skTimer_PrintSeconds(const char* string, skTimer timer);
double skTimer_GetSeconds(skTimer timer);
