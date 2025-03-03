#include <stdio.h>
#include <graphics.h>
#include <stdlib.h>
#include "0.kpgparse.c"
#include "0.raster.c"

int gif(double d) //greatest integer function
{
    if(d<0 && (int)d>d)
        return (int)d-1;
    return (int)d;
}

int numCol, numSL, top, left;
int *SL=NULL; //single scan line

typedef struct edge
{
    double x, dx;
    int dy;
    struct edge *next;
}EDGE;

EDGE **buckets=NULL, *ael=NULL;

void fillBucket(SHAPE shape)
{
    int dy;
    double dx;
    EDGE *add;
    int x1=shape->x;
    int y1=shape->y;
    int x2=shape->next->x;
    int y2=shape->next->y;
	refLine(x1, y1, x2, y2); //polygon boundaries
    if(y2==y1) return; //horizontal lines need not be added
    if(y2>y1) { int t=x1; x1=x2; x2=t;
					t=y1; y1=y2; y2=t; } //P1 is the upper point
    dy = y1-y2;
    dx=(x2-x1)*1.0/dy;
    if(y1<top-numSL || y2>top) return; //totally above or below
	if(y1>top) //clipping needed
	{
		x1 += dx*(y1-top); //calculate
		dy = top-y2;
		y1 = top;
	}
	add = (EDGE*)malloc(sizeof(EDGE));
	add->x=x1+dx/2;
	add->dx=dx;
	add->dy=dy;
	add->next=buckets[top-gif(y1-0.5)]; //add to the bucket using half interval scan lines
	buckets[top-gif(y1-0.5)]=add;
}

void edgeFlag()
{
	EDGE *cur;
	int i, j, flag;
	for(i=0; i<numSL; ++i)
	{
	    for(j=0; j<numCol; ++j) SL[j]=0; //clear the Scan Line
		for(cur=buckets[i]; cur; cur=cur->next)
		    if(cur->dy!=0) //add to active edge list
		    {
		        EDGE *temp = (EDGE*)malloc(sizeof(EDGE));
		        temp->x=cur->x;
		        temp->dx=cur->dx;
		        temp->dy=cur->dy;
		        temp->next=ael;
		        ael=temp;
		    }
		for(cur=ael; cur; cur=cur->next)
		    if(cur->dy > 0)
		    {
		        flag = gif(cur->x+0.5)+left;
				SL[flag]=!SL[flag]; //edge flag
		        --cur->dy; //update
		        cur->x+=cur->dx;
		    }
		flag=0;
		for(j=0; j<numCol; ++j)
		{
		    if(SL[j]) flag=!flag;
		    if(flag) fillPixel(j-left, top-i); //display Scan Line
		}
	}
}

int main(int argc, char **argv)
{    
    SHAPE T, cur;
	int i;
	int gd=DETECT,gm;
	FILE *kpg;
	if(argc<2)
	{
	    printf("[ ERROR ] Usage : <.kpg>_format_input_file\n");
	    return 1;
	}
    kpg = fopen(argv[1], "rx");
    if(!kpg)
    {
	    printf("[ ERROR ] Usage : <.kpg>_format_input_file\n");
	    return 1;
	}
	initgraph(&gd, &gm, NULL); //initialize graphics
	grid();
	numSL = (top=raster.centerY/raster.width) + (480-raster.centerY)/raster.width; //number of Scan Lines
	SL = (int*)malloc(sizeof(int)*(numCol=(left=raster.centerX/raster.width)+(640-raster.centerX)/raster.width)); //Number of Pixels per Line
	--top; --left; --numCol;
	buckets = (EDGE**)malloc(sizeof(EDGE*)*numSL); //initialize buckets
	for(i=0; i<numSL; ++i) buckets[i]=NULL;
	if(!(T=expectPoly(kpg)))
	{
        printf("\nInvalid File Format\n");
        fclose(kpg);
        return 1;
    }
	for(cur=T; cur->next; cur=cur->next)
	    fillBucket(cur); //add lines to respective buckets
    cur->next=T;
    fillBucket(cur);
    cur->next=NULL;
    erase(T);
    fclose(kpg);
    edgeFlag(); //call algorithm
    getchar();
    closegraph();
	for(i=0; i<numSL; ++i) //free memory
	{
		EDGE *E=buckets[i];
		while(E)
		{
			buckets[i]=E;
			E=E->next;
			free(buckets[i]);
		}
	}
	free(buckets);
	free(ael);
	free(SL);
    return 0;
}
