#include <stdio.h>

struct datetime {
  int year,month,day,hour,minute,second;
};

void init_datetime(struct datetime *dt, int year, int month, int day, int hour, int minute, int second) {
  dt->year = year;
  dt->month = month;
  dt->day = day;
  dt->hour = hour;
  dt->minute = minute;
  dt->second = second;
}

void printDatetime(struct datetime *dt) {
  printf("%d:%d:%d %d/%d/%d \n", dt->hour,dt->minute,dt->second,dt->day,dt->month,dt->year);
}

void datetime_set_data(struct datetime *dt, int year, int month, int day) {
  dt->year=year;
  dt->month=month;
  dt->day=day;
}

void datetime_set_time(struct datetime *dt, int hour, int minute, int second) {
  dt->hour = hour;
  dt->minute = minute;
  dt->second = second;
}

struct timerange {
  struct datetime start,slutt;
};

void init_timerange(struct timerange *tr, struct datetime start, struct datetime slutt) {
  tr->start = start;
  tr->slutt = slutt;
}

void printTimerange(struct timerange *tr) {
  printDatetime(&tr->start);
  printDatetime(&tr->slutt);
}

int main() {
  struct datetime dt;
  struct datetime dt2;
  init_datetime(&dt, 2002,12,11,12,13,0);
  init_datetime(&dt2, 2010,12,11,12,13,0);
  printDatetime(&dt);
  datetime_set_time(&dt,23,10,56);
  datetime_set_data(&dt,2009,6,31);
  printDatetime(&dt);
  struct timerange tr;
  init_timerange(&tr,dt,dt2);
  printTimerange(&tr);
}
