#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <alloca.h>
#include <X11/Xlib.h>

// Global variable declarations
static Display *display;

// Function prototypes
static void setStatus(char* str);
static char* twoDigitConvertConvert(char str[]);
static char* getTime();
static int getBattery();
static char* readFile(FILE *file, const int beginning, const int end);
static int findChar(FILE *file, const int offset, const char character);
static int getFreeMemInKilobyte();
static int getFreeMemInMegabyte();
static int getCpuUsage();

static void
setStatus(char *str)
{
	XStoreName(display, DefaultRootWindow(display), str);
	XSync(display, False);
}

// EX. 9 to 09
static char*
twoDigitConvert(char str[])
{
	char *temp = malloc(2 * sizeof(char));
	strcpy(temp, str);

	if(str[1] == '\0')
	{
		temp[0] = '0';
		temp[1] = str[0];
		temp[2] = '\0';
	}
	return temp;
}

static char*
getTime()
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char *datetime = malloc(8 * sizeof(char));
	
	char *hour = (char*)alloca(2 * sizeof(char));
	char *min  = (char*)alloca(2 * sizeof(char));
	char *sec  = (char*)alloca(2 * sizeof(char));
	sprintf(hour, "%d", tm.tm_hour);
	sprintf(min,  "%d", tm.tm_min);
	sprintf(sec,  "%d", tm.tm_sec);
	hour = twoDigitConvert(hour);
	min =  twoDigitConvert(min);
	sec =  twoDigitConvert(sec);

	sprintf(datetime, "%s:%s:%s", hour, min, sec);
	free(hour);
	free(min);
	free(sec);

	return datetime;
}

static int
getBattery()
{
	FILE *energy_full = fopen("/sys/class/power_supply/BAT0/energy_full", "r"), 
	*energy_now = fopen("/sys/class/power_supply/BAT0/energy_now", "r");
	
	int maxcharge, charge;
	fscanf(energy_full, "%d", &maxcharge);
	fscanf(energy_now, "%d", &charge);

	fclose(energy_full);
	fclose(energy_now);
	
	return (int)((float)charge*100 / (float)maxcharge);
}

static char*
readFile(FILE *file, const int beginning, const int end)
{
	const size_t strlen = end-beginning;

	char *str = malloc((strlen+1) * sizeof(char));
	fseek(file, beginning, SEEK_SET);
	
	for(int place = 0; place <= strlen; place++)
	{
		str[place] = fgetc(file);
	}
	str[strlen+1] = '\0';

	return str;
}

static int 
findChar(FILE *file, const int offset, const char character) 
{
	int place = offset;
	fseek(file, offset, SEEK_SET);

	while(fgetc(file) != character)
	{
		place++;
	} 

	return place;
}

static int
getFreeMemInKilobyte()
{
	FILE *memfile = fopen("/proc/meminfo", "r");

	const int beginning = findChar(memfile, 0, '\n')+19;
	const int end = findChar(memfile, beginning, '\n')-1;
	char *memchar = readFile(memfile, beginning, end);

	int freemem = atoi(memchar);
	free(memchar);
	fclose(memfile);

	return freemem;
}

static int
getFreeMemInMegabyte()
{
	return getFreeMemInKilobyte() / 1024;
}

static int getCpuUsage()
{
	FILE *statfile = fopen("/proc/stat", "r");

	
	fclose(statfile);

}

int
main()
{
	display = XOpenDisplay(NULL);
	//printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	
	// Update loop
	while(True) 
	{

		char *datetime = getTime();
		int batterypercent = getBattery();
		int freemem = getFreeMemInMegabyte();

		char *statusmessage = alloca(24 * sizeof(char));
		sprintf(statusmessage, " BAT: %d% | RAM: %dMB | %s ", batterypercent, freemem, datetime);

		setStatus(statusmessage);
		
		free(datetime);

		sleep(1);
	}
	
	XCloseDisplay(display);
	return 0;	
}
