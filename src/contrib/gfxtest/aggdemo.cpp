extern "C" {
#include "device.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
extern void sleep(int);
}
#include "agg.h"
/*
 * C++ linkage - AGG Lite Rasterizer Demo
 *
 * 1/8/2011 ghaerr
 */

static double random(double min, double max)
{
    int r = (rand() << 15) | rand();
    return ((r & 0xFFFFFFF) / double(0xFFFFFFF + 1)) * (max - min) + min;
}

void draw_outline_ellipse(agg::rasterizer& ras, double x,  double y, double rx, double ry, double width)
{
    int i;

    // draw outer circle
    rx += width / 2.0;
    ry += width / 2.0;
    ras.move_to_d(x + rx, y);

    for(i = 1; i < 360; i++)
    {
        double a = double(i) * M_PI / 180.0;
        ras.line_to_d(x + cos(a) * rx, y + sin(a) * ry);
    }

    // draw inner circle in opposite direction
    rx -= width;
    ry -= width;
    ras.move_to_d(x + rx, y);

    for(i = 359; i > 0; i--)
    {
        double a = double(i) * M_PI / 180.0;
        ras.line_to_d(x + cos(a) * rx, y + sin(a) * ry);
    }
}

void draw_filled_ellipse(agg::rasterizer& ras, double x,  double y, double rx, double ry)
{
    int i;
    ras.move_to_d(x + rx, y);

    // Here we have a fixed number of approximation steps, namely 360
    // while in reality it's supposed to be smarter.
    for(i = 1; i < 360; i++)
    {
        double a = double(i) * M_PI / 180.0;
        ras.line_to_d(x + cos(a) * rx, y + sin(a) * ry);
    }
}

void draw_line(agg::rasterizer& ras, double x1, double y1, double x2, double y2, double width)
{

    double dx = x2 - x1;
    double dy = y2 - y1;
    double d = sqrt(dx*dx + dy*dy);
    
    dx = width * (y2 - y1) / d;
    dy = width * (x2 - x1) / d;

    ras.move_to_d(x1 - dx,  y1 + dy);
    ras.line_to_d(x2 - dx,  y2 + dy);
    ras.line_to_d(x2 + dx,  y2 - dy);
    ras.line_to_d(x1 + dx,  y1 - dy);
}

int main(int ac, char **av)
{
	PSD psd = GdOpenScreenExt(FALSE);	// open with no clear to black
	MWCOORD width = psd->xvirtres;
	MWCOORD height = psd->yvirtres;

    // Create the rendering buffer 
    agg::rendering_buffer rbuf(psd->addr, width, height, width * 4);

    // Create the renderer and the rasterizer
    agg::renderer<agg::span_rgba32> ren(rbuf);
    agg::rasterizer ras;

    // Setup the rasterizer
    ras.gamma(1.3);
    ras.filling_rule(agg::fill_even_odd);
    ren.clear(agg::rgba8(255, 255, 255));

    int i, j;

    // Draw random polygons
    for(i = 0; i < 10; i++)
    {
        int n = rand() % 6 + 3;

        // Make the polygon. One can call move_to() more than once. 
        // In this case the rasterizer behaves like Win32 API PolyPolygon().
        ras.move_to_d(random(-30, rbuf.width() + 30), 
                      random(-30, rbuf.height() + 30));

        for(j = 1; j < n; j++)
        {
            ras.line_to_d(random(-30, rbuf.width() + 30), 
                          random(-30, rbuf.height() + 30));
        }

        // Render
        ras.render(ren, agg::rgba8(rand() & 0xFF, 
                                   rand() & 0xFF, 
                                   rand() & 0xFF, 
                                   rand() & 0xFF));
    }

    // Draw random ellipses
    for(i = 0; i < 50; i++)
    {
	if (i & 1)
            draw_outline_ellipse(ras, 
                     random(-30, rbuf.width()  + 30), 
                     random(-30, rbuf.height() + 30),
                     random(3, 50), random(3, 50),
                     random(.1, 20));
	else
            draw_filled_ellipse(ras, 
                     random(-30, rbuf.width()  + 30), 
                     random(-30, rbuf.height() + 30),
                     random(3, 50), random(3, 50));

        ras.render(ren, agg::rgba8(rand() & 0xFF, rand() & 0xFF, rand() & 0xFF, (rand() & 0x7F) + 100));
    }

    // Draw random straight lines
    for(i = 0; i < 20; i++)
    {
        draw_line(ras, 
                  random(-30, rbuf.width()  + 30), 
                  random(-30, rbuf.height() + 30),
                  random(-30, rbuf.width()  + 30), 
                  random(-30, rbuf.height() + 30),
                  random(0.1, 10));
        ras.render(ren, agg::rgba8(rand() & 0x7F, rand() & 0x7F, rand() & 0x7F));
    }

    draw_line(ras, 10.0, 10.0, 40.0, 40.0, 10.0);

    ras.move_to_d(10.0, 10.0);
    ras.line_to_d(20.0, 10.0);
    ras.line_to_d(20.0, 20.0);
    ras.line_to_d(10.0, 20.0);

    ras.move_to_d(10.0, 10.0);
    ras.line_to_d(15.0, 15.0);
    ras.line_to_d(10.0, 20.0);
    ras.line_to_d(5.0, 5.0);

    ras.render(ren, agg::rgba8(rand() & 0xFF, rand() & 0xFF, rand() & 0xFF, 255));

    // send image to Microwindows
	psd->Update(psd, 0, 0, width, height);

    //MWIMAGEHDR	image;
    //GR_GC_ID gc = GrNewGC();
    //image.flags = 0;
    //image.width = width;
    //image.height = height;
    //image.planes = 1;
    //image.bpp = 32;
    //image.data_format = MWIF_RGBA8888;
    //image.pitch = width * 4;
    //image.imagebits = (MWUCHAR *)buf;
    //image.palette = 0;
    //image.palsize = 0;
    //image.transcolor = MWNOCOLOR;
    //GrDrawImageBits(wid, gc, 0, 0, &image);
    //GrDestroyGC(gc);

	GdFixCursor(psd);				// for drawVLine/drawHLine
	psd->PreSelect(psd);
	sleep(120);
	return 0;
}
