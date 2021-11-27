#ifndef TYPES_H
#define TYPES_H


typedef struct {
  int port;
  int currentState;
} Relay;

typedef struct {
  int Hour;
  int Minute;
  int Second;
  int Duration; // In seconds
} Scheduling;

#endif
