#include <iostream>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xdbe.h>
#include <cmath>
#include <X11/keysym.h>

using namespace std;

// globals 
struct Global {
    Display* dpy;
    Window win;
    GC gc;
    XdbeBackBuffer backBuffer;
    XdbeSwapInfo swapInfo;
    Atom wm_delete_window;
    int xres, yres;
    float wave_offset; 
    float ocean_offset; 
    float sun = 40; 
    float brightness; 
    float sunSpeed = 0.5f; 
    float ocean_vertical_offset;
    float wave_vertical_offset;
} g;

void draw_ocean();
void draw_sun();
void draw_sky();

// struct for colors 
struct Color {
    unsigned char r, g, b;
};

// convert RGB colors to X11 format 
unsigned long toXColor(Display* display, Color color) {
    return (color.r << 16) | (color.g << 8) | color.b;
}

// interpolates between two RGB colors 
unsigned long interpolateColors(unsigned long color1, unsigned long color2, float t) {
    int r1 = (color1 >> 16) & 0xFF;
    int g1 = (color1 >> 8) & 0xFF;
    int b1 = color1 & 0xFF;

    int r2 = (color2 >> 16) & 0xFF;
    int g2 = (color2 >> 8) & 0xFF;
    int b2 = color2 & 0xFF;

    int r = static_cast<int>(r1 + (r2 - r1) * t);
    int g = static_cast<int>(g1 + (g2 - g1) * t);
    int b = static_cast<int>(b1 + (b2 - b1) * t);

    return (r << 16) | (g << 8) | b;
}

// determines the sun color based on its position
unsigned long getSunColor(float sun) {
    if(sun > g.yres / 2) {
        float t = (sun - g.yres / 2) / (g.yres / 2); 
        return interpolateColors(toXColor(
            g.dpy, {255, 140, 0}), toXColor(g.dpy, {255, 69, }), t); 
    } else {
        return toXColor(g.dpy, {255, 140, 0}); 
    }
}

// adjusts the brightness of the colors 
unsigned long adjustColorBrightness(Color color, float brightness) {
    int r = min(255, static_cast<int>(color.r * brightness));
    int g = min(255, static_cast<int>(color.g * brightness));
    int b = min(255, static_cast<int>(color.b * brightness));

    return (r << 16) | (g << 8) | b;
}

void cleanup_xwindows(void) {
    if(!XdbeDeallocateBackBufferName(g.dpy, g.backBuffer)) {
        cerr << "Error: unable to deallocate back buffer." << endl;
    }

    XFreeGC(g.dpy, g.gc);
    XDestroyWindow(g.dpy, g.win);
    XCloseDisplay(g.dpy);
}

void set_window_title() {
    char title[256];
    sprintf(title, "Sunset Animation (%d x %d)", g.xres, g.yres);
    XStoreName(g.dpy, g.win, title);
}

void init_xwindows(int w, int h) {
    g.xres = w;
    g.yres = h;
    g.wave_offset = 0;
    g.ocean_offset = 0;

    XSetWindowAttributes attributes;
    int major, minor;
    XdbeBackBufferAttributes* backAttr;

    g.dpy = XOpenDisplay(NULL);
    if(!g.dpy) {
        cerr << "Unable to open X display." << endl;
        exit(1);
    }

    attributes.event_mask = ExposureMask | StructureNotifyMask |
                            PointerMotionMask | ButtonPressMask |
                            ButtonReleaseMask | KeyPressMask | KeyReleaseMask;
    attributes.backing_store = Always;
    attributes.save_under = True;
    attributes.override_redirect = False;
    attributes.background_pixel = 0x00000000;

    Window root = DefaultRootWindow(g.dpy);
    g.win = XCreateWindow(g.dpy, root, 0, 0, g.xres, g.yres, 0,
            CopyFromParent, InputOutput, CopyFromParent,
            CWBackingStore | CWOverrideRedirect | CWEventMask |
            CWSaveUnder | CWBackPixel, &attributes);
    g.gc = XCreateGC(g.dpy, g.win, 0, NULL);

    if(!XdbeQueryExtension(g.dpy, &major, &minor)) {
        cerr << "Error: unable to fetch Xdbe Version.\n" << endl;
        XFreeGC(g.dpy, g.gc);
        XDestroyWindow(g.dpy, g.win);
        XCloseDisplay(g.dpy);
        exit(1);
    }

    g.backBuffer = XdbeAllocateBackBufferName(g.dpy, g.win, XdbeUndefined);
    backAttr = XdbeGetBackBufferAttributes(g.dpy, g.backBuffer);
    g.swapInfo.swap_window = backAttr->window;
    g.swapInfo.swap_action = XdbeUndefined;
    XFree(backAttr);

    set_window_title();
    XMapWindow(g.dpy, g.win);
    XRaiseWindow(g.dpy, g.win);

    g.wm_delete_window = XInternAtom(g.dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(g.dpy, g.win, &g.wm_delete_window, 1);
}

void render() {
    GC gc = g.gc;

   // move the sun down the screen 

   // if the sun has goes below 2/3 reset it

    draw_sky();
    draw_sun();
    draw_ocean();

    XdbeSwapBuffers(g.dpy, &g.swapInfo, 1);
}

void draw_ocean() {
    // calculate the brightness of the ocean based on the sun's position

    Color oceanColor = {0, 0, 255};
    
    // adjust the brightness of the ocean color 
    unsigned long oceanFinalColor = toXColor(g.dpy, oceanColor);

    // set the ocean color and draw the ocean with a sin wave here
    XSetForeground(g.dpy, g.gc, oceanFinalColor);
    XFillRectangle(g.dpy, g.backBuffer, g.gc,
                   0, g.yres / 2, g.xres, g.yres / 2);

    
    // draw the wave
    // set the color for the wave
    // loop through each x pixel 
    // use a sine function for the wave 
    // draw a wave line
    //XDrawLine(g.dpy, g.backBuffer, g.gc, i, g.yres / 2 + waveHeight, i, g.yres / 2 + waveHeight + 10);

    // make the wave and ocean move 
}

void draw_sky() {
    float skyHeight = g.yres * 0.6f;

    // set the brightness 

    for (int i = 0; i < skyHeight; i++) {
        float t = static_cast<float>(i) / skyHeight;

        // create the sunset sky here 

        // the sky should be this color if the sun is above 1/3 of the screen
        unsigned long skyColor = interpolateColors(
            toXColor(g.dpy, {135, 206, 235}),  
            toXColor(g.dpy, {65, 105, 225}),  
            t
        );

        // apply brightness to the skyColor

        XSetForeground(g.dpy, g.gc, skyColor);
        XFillRectangle(g.dpy, g.backBuffer, g.gc, 0, i, g.xres, 1);
    }
}

void draw_sun() {

    // need to do on your own :)

}

int check_keys(XEvent *e) {
    if (e->type != KeyPress && e->type != KeyRelease)
        return 0;
    int key = XLookupKeysym(&e->xkey, 0);

    if (e->type == KeyPress) {
        switch(key) {
            case XK_Escape:
                return 1;
            // if + or = is pressed, increase the speed of the sun
            // if - is pressed, decrease the speed of the sun
        }
    }

    return 0;
}

int main() {
    init_xwindows(1024, 768);
    
    while(true) {
        XEvent e;
        while (XPending(g.dpy)) {
            XNextEvent(g.dpy, &e);

            if (e.type == ClientMessage && e.xclient.data.l[0] == g.wm_delete_window) {
                cleanup_xwindows();
                return 0;
            }

            if (e.type == KeyPress) {
                if (check_keys(&e)) {
                    cleanup_xwindows();
                    return 0;
                }
            }
        }
        render();
        usleep(10000);
    }
    return 0;
}