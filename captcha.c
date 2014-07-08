/* 2ch.ru captcha lookalike
 * 
 * written by noko3
 * License: GPLv2, see COPYING
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <time.h>
#include <locale.h>
#include <gd.h>
#include "lib/list.c"
#include "lib/hashtable.c"

#define BEZIER_STEPS 30
#define CAPTCHA_BG 0xeeeeee
#define CAPTCHA_FG 0x222222
#define CAPTCHA_BASE_W 400
#define CAPTCHA_BASE_H 100
#define CAPTCHA_SZ 40
#define CAPTCHA_LW 2
#define CAPTCHA_DIST 0.1
#define CAPTCHA_NLINES_MIN 2
#define CAPTCHA_NLINES_MAX 3
#define LETTERS "letters.txt"

uint _bezier_steps;
uint _captcha_base_w;
uint _captcha_base_h;
uint _captcha_sz;
uint _captcha_lw;
uint _captcha_nlines_min;
uint _captcha_nlines_max;
float _captcha_dist;
uint32_t _captcha_bg;
uint32_t _captcha_fg;
const char *_letters;

enum { T_LINE=0, T_CURVE=1 };

static inline int rand_i(int min, int max) {
    if(max <= min) return max;
    return min + rand()%(max-min);
}

struct howto {
    uint8_t type;
    float instr[8];
} __attribute((packed));
struct letter {
    float widthFactor;
    struct howto h2s[32];
} __attribute((packed));

struct pt { float x; float y; };

struct hash_table *letters;

static void readconf(const char *filename) {
    FILE *fp = fopen(filename, "r");
    fseek(fp, 0, SEEK_END);
    size_t fsz = ftell(fp);
    rewind(fp);
    char c;
    char buf[128] = {0}, buf2[128] = {0}, empty[128] = {0};
    uint i, j, x, h;
    struct letter *L = NULL;
    struct howto *h2 = NULL;
    list_node *item = NULL;
    int tmp, offset = 0;
    char *off;
    while (ftell(fp) < fsz) {
        fread(&c, 1, 1, fp);
        switch(c) {
            case '.':
                if(L == NULL) {
                    while(c != '\n')
                        fread(&c, 1, 1, fp);
                    continue;
                }
                offset = 0;
                i = 0;
                h2 = &(L->h2s[x++]);
                h2->type = T_CURVE;
                fseek(fp, 3, SEEK_CUR);
                while((c = fgetc(fp)) != '\n') {
                    buf[i++] = c;
                }
                off = buf;
                for(j=0; j<8; j++) {
                    h2->instr[j] = atof(off);
                    off = strchr(off+1, ' ');
                    if(!off) break;
                }
                
                memcpy(buf, empty, sizeof(char)*128);
                break;
            case '-':
                if(L == NULL) {
                    while(c != '\n')
                        fread(&c, 1, 1, fp);
                    continue;
                }
                offset = 0;
                i = 0;
                h2 = &(L->h2s[x++]);
                h2->type = T_LINE;
                fseek(fp, 3, SEEK_CUR);
                while((c = fgetc(fp)) != '\n') {
                    buf[i++] = c;
                }
                off = buf;
                for(j=0; j<4; j++) {
                    h2->instr[j] = atof(off);
                    off = strchr(off+1, ' ');
                    if(!off) break;
                }
                
                memcpy(buf, empty, sizeof(char)*128);
                break;
            case '\n': case EOF:
                if(L) {
                    htable_set(letters, (const char*)buf2, L, sizeof(struct letter));
                    free(L);
                    L = NULL;
                    memcpy(buf2, empty, 128);
                    memcpy(buf, empty, 128);
                }
                break;
            default:
                x = i = 0;
                while(c != '\n') {
                    buf[i++] = c;
                    fread(&c, 1, 1, fp);
                }
                L = malloc(sizeof(struct letter));
                for(i=0; i<32; L->h2s[i++].type=-1);
                L->widthFactor = atof(strchr(buf, ' ')+1);
                for(i=0; buf[i] != ' '; i++); //malformed string
                for(; i<128; buf[i++]=0);
                
                memcpy(buf2, buf, sizeof(char)*128);
                memcpy(buf, empty, sizeof(char)*128);
                break;
        }
    }
    fclose(fp);
}

static void get_xy(float x1, float y1, float x2, float y2, float t,
                   float *xR, float *yR) {
    *xR = x1 + t*(x2-x1);
    *yR = y1 + t*(y2-y1);
}

static void draw_bezier(gdImagePtr img,
                        float x1, float y1, float x2, float y2,
                        float x3, float y3, float x4, float y4,
                        uint x, uint y, uint w, uint h, uint32_t color,
                        uint steps) {
    int i;
    float my_prev_x, my_prev_y, t;
    float my_x1, my_x2, my_x3, my_x4, my_x5, my_x6;
    float my_y1, my_y2, my_y3, my_y4, my_y5, my_y6;
    my_prev_x = x1*w;
    my_prev_y = y1*h;
    for (i = 0; i < steps; i++) {
        t = (i+1)/(float)steps;
        get_xy(x1*w, y1*h, x2*w, y2*h, t, &my_x1, &my_y1);
        get_xy(x3*w, y3*h, x4*w, y4*h, t, &my_x2, &my_y2);
        get_xy(x2*w, y2*h, x3*w, y3*h, t, &my_x3, &my_y3);
        get_xy(my_x1, my_y1, my_x3, my_y3, t, &my_x4, &my_y4);
        get_xy(my_x3, my_y3, my_x2, my_y2, t, &my_x5, &my_y5);
        get_xy(my_x4, my_y4, my_x5, my_y5, t, &my_x6, &my_y6);
        gdImageLine(img, x+my_prev_x, y+my_prev_y, x+my_x6, y+my_y6, color);
        my_prev_x = my_x6;
        my_prev_y = my_y6;
    }
}

static uint draw_from_chinfo(gdImagePtr img, uint h, float A, uint x, uint y,
                             struct letter *ch, uint32_t color) {
    int i, T;
    for (i = 0; ch->h2s[i].type != (uint8_t)-1; i++) {
        if (ch->h2s[i].type == T_LINE) {
            gdImageLine(img,
                x+h*A*ch->h2s[i].instr[0], y+h*ch->h2s[i].instr[1],
                x+h*A*ch->h2s[i].instr[2], y+h*ch->h2s[i].instr[3],
                color);
        } else if (ch->h2s[i].type == T_CURVE) {
            T = 8;
            draw_bezier(img,
                ch->h2s[i].instr[0], ch->h2s[i].instr[1],
                ch->h2s[i].instr[2], ch->h2s[i].instr[3],
                ch->h2s[i].instr[4], ch->h2s[i].instr[5],
                ch->h2s[i].instr[6], ch->h2s[i].instr[7],
                x, y, h*A, h, color, _bezier_steps);
        }
    }
    return h*A;
}
static uint draw_letter(gdImagePtr img, uint h, uint x, uint y,
                        const char *code, uint32_t color, float dist) {
    struct letter my_l, *L = htable_find(letters, code);
    memcpy(&my_l, L, sizeof(struct letter));
    memcpy(my_l.h2s, L->h2s, 32*sizeof(struct howto));
    float A = my_l.widthFactor;
    int a,b,c;
    
    if (dist > 0.0) {
        int i, j, T;
        struct pt *tmp;
        float *ptr;
        char buf[32] = {0};
        struct hash_table *my_HT = malloc(sizeof(struct hash_table));
        htable_init(my_HT, 256);
        for (i = 0; my_l.h2s[i].type != (uint8_t)-1; i++) {
            if (my_l.h2s[i].type == T_LINE)
                T = 4;
            else if (my_l.h2s[i].type == T_CURVE)
                T = 8;
            for (j=0; j<T/2; j++) {
                snprintf(buf, 31, "%f %f",
                    my_l.h2s[i].instr[2*j], my_l.h2s[i].instr[2*j+1]);
                tmp = malloc(sizeof(struct pt));
                tmp->x = my_l.h2s[i].instr[2*j];
                tmp->x += dist * 2.0*rand()/RAND_MAX;
                tmp->y = my_l.h2s[i].instr[2*j+1];
                tmp->y += dist * 2.0*rand()/RAND_MAX;
                htable_set(my_HT, buf, tmp, sizeof(struct pt));
                free(tmp);
            }
        }
        for (i = 0; my_l.h2s[i].type != (uint8_t)-1; i++) {
            if (my_l.h2s[i].type == T_LINE)
                T = 4;
            else if (my_l.h2s[i].type == T_CURVE)
                T = 8;
            for (j=0; j<T/2; j++) {
                snprintf(buf, 31, "%f %f",
                    my_l.h2s[i].instr[2*j], my_l.h2s[i].instr[2*j+1]);
                tmp = htable_find(my_HT, buf);
                my_l.h2s[i].instr[2*j] = tmp->x;
                my_l.h2s[i].instr[2*j+1] = tmp->y;
            }
        }
        htable_kill(my_HT);
    }
    
    return draw_from_chinfo(img, h, A, x, y, &my_l, color);
}

static uint draw_string(gdImagePtr img, uint x, uint y, uint h,
                        const wchar_t *str,
                        uint32_t color, float dist) {
    uint w = 0;
    int i;
    char buf[5], z5[5] = {0};
    for(i=0; i < wcslen(str); i++) {
        memcpy(&buf, &z5, 5);
        wctomb(buf, *(str+i));
        w += draw_letter(img, h, x+w, y, buf, color, dist) + h/5;
    }
    return w;
}

static void draw_bline(gdImagePtr ximg, uint nCuts, float maxH, uint32_t color) {
    int w = gdImageSX(ximg);
    int h = gdImageSY(ximg)/2;
    int l = rand_i(-w/3, w/3);
    int R = rand_i(w/2, w*1.5);
    uint start = l;
    float x,A,B, *pts = malloc(sizeof(float)*8*nCuts);
    int i;
    uint my_R;
    for (i = 0; i < nCuts; i++) {
        my_R = R/nCuts * rand_i(128, 384)/250.;
        if(!i) {
            pts[0] = 0;
            pts[1] = rand_i(-100, 100)/100.;
            pts[2] = rand_i(0, 60)/100.;
            pts[3] = rand_i(-100, 100)/100.;
        } else {
            pts[8*i+0] = pts[8*(i-1) + 6] - 1;
            pts[8*i+1] = pts[8*(i-1) + 7];
            pts[8*i+2] = rand_i(0, 60)/100.;
            x = pts[8*i+2];
            A = pts[8*(i-1) + 7] - pts[8*(i-1) + 5];
            B = pts[8*(i-1) + 6] - pts[8*(i-1) + 4];
            pts[8*i+3] = pts[8*i+1] + A*x/B;
        }
        pts[8*i+4] = rand_i(40, 100)/100.;
        pts[8*i+5] = rand_i(-100, 100)/100.;
        pts[8*i+6] = 1.0;
        pts[8*i+7] = rand_i(-100, 100)/100.;
        draw_bezier(ximg,
                    pts[8*i + 0], pts[8*i + 1], pts[8*i + 2], pts[8*i + 3],
                    pts[8*i + 4], pts[8*i + 5], pts[8*i + 6], pts[8*i + 7],
                    start, h, my_R, h*maxH, color, _bezier_steps);
        start += my_R;
    }
    free(pts);
}

static gdImagePtr draw_captcha(const wchar_t *str) {
    uint iw = 10;
    uint ih = _captcha_sz * 1.9;
    gdImagePtr img = gdImageCreateTrueColor(_captcha_base_w, _captcha_base_h);
    gdImageFill(img, 0, 0, _captcha_bg);
    gdImageSetThickness(img, _captcha_lw);
    
    iw += draw_string(img, 5, 5, _captcha_sz, str, _captcha_fg, _captcha_dist);
    gdImagePtr img_ret = gdImageCreateTrueColor(iw, ih);
    gdImageSetThickness(img_ret, _captcha_lw);
    gdImageCopy(img_ret, img, 0, 0, 0, 0, iw+10, ih);
    int i;
    for (i = 0; i < rand_i(_captcha_nlines_min, _captcha_nlines_max); i++) {
        draw_bline(img_ret, rand_i(1, wcslen(str)/1.5), 1, _captcha_fg);
    }
    gdImageDestroy(img);
    return img_ret;
}

int main(int argc, char **argv) {
    _captcha_base_h = CAPTCHA_BASE_H;
    _captcha_base_w = CAPTCHA_BASE_W;
    _captcha_bg = CAPTCHA_BG;
    _captcha_fg = CAPTCHA_FG;
    _bezier_steps = BEZIER_STEPS;
    _captcha_dist = CAPTCHA_DIST;
    _captcha_nlines_max = CAPTCHA_NLINES_MAX;
    _captcha_nlines_min = CAPTCHA_NLINES_MIN;
    _captcha_sz = CAPTCHA_SZ;
    _captcha_lw = CAPTCHA_LW;
    _letters = LETTERS;
    int i;
    for (i = 0; i < argc; i++) {
        if(i+1 < argc && !strcmp(argv[i], "--captcha-safe-w"))
            _captcha_base_w = atoi(argv[i+1]);
        if(i+1 < argc && !strcmp(argv[i], "--captcha-safe-h"))
            _captcha_base_h = atoi(argv[i+1]);
        if(i+1 < argc && !strcmp(argv[i], "--captcha-bg"))
            _captcha_bg = strtol(argv[i+1], NULL, 16);
        if(i+1 < argc && !strcmp(argv[i], "--captcha-fg"))
            _captcha_fg = strtol(argv[i+1], NULL, 16);
        if(i+1 < argc && !strcmp(argv[i], "--captcha-sz"))
            _captcha_sz = atoi(argv[i+1]);
        if(i+1 < argc && !strcmp(argv[i], "--captcha-lw"))
            _captcha_lw = atoi(argv[i+1]);
        if(i+1 < argc && !strcmp(argv[i], "--captcha-nlines-min"))
            _captcha_nlines_min = atoi(argv[i+1]);
        if(i+1 < argc && !strcmp(argv[i], "--captcha-nlines-max"))
            _captcha_nlines_max = atoi(argv[i+1]);
        if(i+1 < argc && !strcmp(argv[i], "--captcha-dist"))
            _captcha_dist = atof(argv[i+1]);
        if(i+1 < argc && !strcmp(argv[i], "--bezier-steps"))
            _bezier_steps = atoi(argv[i+1]);
        if(i+1 < argc && !strcmp(argv[i], "--letters"))
            _letters = argv[i+1];
    }
    
    setlocale(LC_ALL, "C.UTF-8");
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec);
    
    letters = malloc(sizeof(struct hash_table));
    htable_init(letters, 100);
    readconf(_letters);
    
    char str[512] = {0};
    char empty[512] = {0};
    wchar_t buffer[128] = {0};
    char c;
    int tmp;
    i = 0;
    
    gdImagePtr img;
    while((c = fgetc(stdin)) != EOF) {
        if(c == '\n') {
            tmp = i;
            i = 0;
            swprintf(buffer, 127, L"%s", str);
            memcpy(str, empty, 512);
            img = draw_captcha(buffer);
            gdImagePng(img, stdout);
            gdImageDestroy(img);
            memcpy(buffer, empty, 512);
        } else {
            str[i++] = c;
        }
    }
    
    htable_kill(letters);
    
    return 0;
}
