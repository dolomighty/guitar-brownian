
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "dyn/kbhit.h"

#undef NDEBUG
#include <assert.h>



#define MAX_PERIOD 22050

typedef struct GEN {
    int period;
    float h[MAX_PERIOD];
    int index;
    float lp;
} GEN;

GEN gen[3];


//void synth_block( GEN *g ){
//    int i;
//    float *now;
//    float *pre = g->h[g->page];
//    g->page^=1;
//    now = g->h[g->page];
//
////    fprintf(stderr,"%d\n",g->period);
////    fprintf(stderr,"%d\n",g->page);
//
//    // random
//    for( i=0; i<g->period; i++ ){
//        pre[i] += 1.0-rand()*2.0/RAND_MAX;
//    }
//
////    // lowpass via fir w/wraparound + regressione a 0
////    for( i=0; i<g->period; i++ ){
////        now[i]= (
////            pre[(i-2)%g->period]+
////            pre[(i-1)%g->period]+
////            pre[i]+
////            pre[i]+
////            pre[(i+1)%g->period]+
////            pre[(i-2)%g->period]+
////        0)/6.06;
////    }
//
//    // lowpass via fir w/wraparound + regressione a 0
//    for( i=0; i<g->period; i++ ){
//        now[i]=(
//            pre[(i-5)%g->period]+
//            pre[(i-4)%g->period]+
//            pre[(i-3)%g->period]+
//            pre[(i-2)%g->period]+
//            pre[(i-1)%g->period]+
//            pre[i]+
//        0)/6.06;
//    }
//}



void synth_init( GEN *g, int period ){
    assert(period<=MAX_PERIOD);
    g->period = period;
    g->index = period;
    g->lp = 0;
    for( int i=0; i<period; i++ ){
        g->h[i] = 0;
    }
}





float synth( GEN *g ){
    if( ++g->index >= g->period ) g->index = 0;
    g->h[g->index] = g->h[g->index]+(1.0-rand()*2.0/RAND_MAX-g->h[g->index])*0.1;
//    return g->h[g->index];
    g->lp = g->lp+(g->h[g->index]-g->lp)*0.1;
    return g->lp;
}













//void audio_cb_F32_mono( void *userdata, Uint8 *stream, int len_bytes ){
//    float *frame = (float*)stream;
//    int len_frames = len_bytes / sizeof(*frame);
//    for(; len_frames > 0; len_frames--, frame++ ){
//        float v = (synth(&a)+synth(&b))*0.5;
//        if(v<-1)v=-1; else
//        if(v>+1)v=+1;
//        *frame = v;
//    }
//}




void audio_cb_F32_stereo( void *userdata, Uint8 *stream, int len_bytes ){
    struct LR {
        float L,R;
    } *frame = (struct LR *)stream;
    int len_frames = len_bytes / sizeof(*frame);
    for(; len_frames>0; len_frames--, frame++ ){
        float A = synth(&gen[0]);
        frame->L = A+synth(&gen[1]);
        frame->R = A+synth(&gen[2]);
    }
}




float key2freq( int key ){
    // da nota midi a frequenza in Hz
    return 6.875 * pow( 2, (3.0+key)/12 );
}




int main( int argc, char *argv[]){ 

    assert( SDL_Init( SDL_INIT_AUDIO ) >= 0 );
    atexit(SDL_Quit);

    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq     = 44100;
    want.format   = AUDIO_F32;
    want.samples  = 256;
//    want.channels = 1;
//    want.callback = audio_cb_F32_mono;
    want.channels = 2;
    want.callback = audio_cb_F32_stereo;

    assert( SDL_OpenAudio( &want, NULL ) >= 0 );
  
    synth_init( &gen[0], want.freq*4/key2freq(60+0));
    synth_init( &gen[1], want.freq*4/key2freq(60+7));
    synth_init( &gen[2], want.freq*4/key2freq(60+12));

    SDL_PauseAudio(0);  // start audio playing.

    bool esci = false;
    while(!esci){
        SDL_Delay(10);
        if(kbhit()){
            int c = getchar();
            switch(c){
                case 'q':
                    esci=true;
                    break; 

#define C(K,N) \
                case K: \
                    synth_init( &gen[0], want.freq*4/key2freq(N+0)); \
                    synth_init( &gen[1], want.freq*4/key2freq(N+7)); \
                    synth_init( &gen[2], want.freq*4/key2freq(N+12)); \
                    break;

                C('z',60);
                C('s',61);
                C('x',62);
                C('d',63);
                C('c',64);
                C('v',65);
                C('g',66);
                C('b',67);
                C('h',68);
                C('n',69);
                C('j',70);
                C('m',71);
#undef C
            }
        }
    }

    SDL_CloseAudio();

    return 0;
}


