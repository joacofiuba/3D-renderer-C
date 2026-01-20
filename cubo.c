#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "pinguino.h"

#define WIDTH 1000
#define HEIGHT 1000
#define INF INT_MAX
#define DIM 3
#define FPS 60
#define GREEN 0x39FF14
#define PI 3.1415

float vertex[8][3] = {
    { 0.5,  0.5,  0.5},
    {-0.5,  0.5,  0.5},
    {-0.5, -0.5,  0.5},
    { 0.5, -0.5,  0.5},


    { 0.5,  0.5, -0.5},
    {-0.5,  0.5, -0.5},
    {-0.5, -0.5, -0.5},
    { 0.5, -0.5, -0.5}

};

int faces[4][4] = {
    {0, 1, 2, 3},
    {4, 5, 6, 7},
    {0, 4, 7, 3},
    {1, 5, 6, 2}
};    


struct Segment {
    float a[2]; // posición xy inicio
    float b[2]; // posicion xy final
};


typedef struct heap_v{
    float *v; 
    int n; 
}vector_t; 

// invariante: dimension <= 3
vector_t *crear_v(int n){
    vector_t *v = malloc(sizeof(vector_t));
    if(v == NULL) return NULL;
    
    v->v = malloc(DIM * sizeof(float));
    if(v->v == NULL) {free(v); return NULL;}
    v->n = n; 

    return v; 
}

void destruir_v(vector_t *v){
    free(v->v);
    free(v);
}

// devuelve el rectangulo con origen de coordenadas centrado y normalizado a -1, 1
SDL_Rect screen(float *v){
    SDL_Rect rect;
    int s = 1;
    
    rect.x = (v[0] * WIDTH/2.0) + WIDTH/2 - s/2,
    rect.y = (-1) * (v[1] * HEIGHT/2.0) + HEIGHT/2 - s/2, 
    rect.w = s;
    rect.h = s;

    return rect;
}

// dim = 3 => dim = 2
float *project(float *v){
    int x = 0, y = 1, z = 2;
   
    if(z <= 0.01) return NULL;

    // proyecto los puntos sobre un plano (screen)
    v[0] = (v[x] / v[z]);
    v[1] = (v[y] / v[z]);
    v[z] = 0;

    return v; 
}


float *translate_z(float *v, float dz){
    int z = 2;
    v[z] += dz;
    
    return v;
}

// recibe un vector de dimension 3, retorna un vector de dimensión 3 rotado en el plano xz
float *rotate_xz(float *v, float angle){
    int x = 0, y = 1, z = 2;
    float vx = v[x]; 
    float vz = v[z];
    // los puntos en Y no sufren ninguna transformación    
    // x' = x*cos(angle) - z*sin(angle); 
    // z' = x*sin(angle) + y*cos(angle);
    float c = cos(angle);
    float s = sin(angle);
    v[x] = vx * c - vz * s;
    v[z] = vx * s + vz * c;

    return v;
}


void point(SDL_Surface *psurface, SDL_Rect p){
    SDL_FillRect(psurface, &p, GREEN);
}


void draw_segment(struct Segment *segment, SDL_Surface *psurface, uint32_t color){
    float *p = segment->a; // punto de paso

    float v_dir[2]; 
    v_dir[0] = segment->b[0] - segment->a[0]; // v_x
    v_dir[1] = segment->b[1] - segment->a[1]; // v_y

    for(float i = 0; i < 1; i += 0.001){ 
        float x = v_dir[0] * i + p[0];
        float y = v_dir[1] * i + p[1];

        float v_segment[] = {x, y};
        point(psurface, screen(v_segment));
    }
}

// produzco un frame trasladado y rotado
void frame(SDL_Surface *psurface, size_t nv, float vertex[nv][DIM], size_t nf, size_t cant_lados, int faces[nf][cant_lados], float dz, float angle){
    // DIBUJAR SOLO VERTICES:
    // for(size_t f = 0; f < nv; f++){
    //     vector_t *v = crear_v(DIM);            
    //     if(v == NULL) return;
    //     memcpy(v->v, &vertex[f][0], DIM * sizeof(float));
    //     point(psurface, screen(project(translate_z(rotate_xz(v->v, angle), dz))));
    //
    //     destruir_v(v);
    // }

    
    vector_t *a = crear_v(DIM);
    if(a == NULL) return;
    vector_t *b = crear_v(DIM);
    if(b == NULL){
        destruir_v(a);
        return;
    }

    for(size_t i = 0; i < nf; i++){
        for(size_t j = 0; j < cant_lados; j++){
            struct Segment segment;
            int indice0 = faces[i][j];
            int indice1 = faces[i][(j+1) % cant_lados];

            if (indice0 < 0 || indice0 >= nv) continue;
            if (indice1 < 0 || indice1 >= nv) continue;


            memcpy(a->v, &vertex[indice0][0], DIM * sizeof(float));
            memcpy(b->v, &vertex[indice1][0], DIM * sizeof(float));
            

            void * pa = project(translate_z(rotate_xz(a->v, angle), dz));
            void * pb = project(translate_z(rotate_xz(b->v, angle), dz));
            a->n = 2;
            b->n= 2;

            if(pa == NULL || pb == NULL) continue;
            
            segment.a[0] = a->v[0]; segment.a[1] = a->v[1];
            segment.b[0] = b->v[0]; segment.b[1] = b->v[1];
            
            draw_segment(&segment, psurface, GREEN);
        }
    }

    destruir_v(a); 
    destruir_v(b);
}


int main(){
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Window *pwindow = SDL_CreateWindow(
        "cubo", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI
    );    
    
    SDL_Surface *psurface = SDL_GetWindowSurface(pwindow);
    SDL_FillRect(psurface, NULL, 0x000000);
    SDL_UpdateWindowSurface(pwindow);
    
    bool quit_flag = false;
    while(1){
        SDL_Event event;
        
        
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT)
                quit_flag = true;
        }

        if(quit_flag == true)
            break;
        

        // rotation & traslation parameters
        float dt = 1.0/FPS; 
        float dz = 1;
        float angle = 0; 
       
        // pinguino
        int cant_lados = 3;
        int n_v = 323;
        int n_f = 623;

        for(size_t i = 0; i < 1000; i++){
            while(SDL_PollEvent(&event)){
                if(event.type == SDL_QUIT)
                    quit_flag = true;
            }

            if(quit_flag == true)
                break;

     
            SDL_FillRect(psurface, NULL, 0x000000);
            
            dz = 1.25; 
            angle += 0.25*PI*dt;
            
            // Pinguino
            frame(psurface, n_v, vs, n_f, cant_lados, fs, dz, angle);
            // cubo
            frame(psurface, 8, vertex, 4, 4, faces, dz, angle );
            SDL_UpdateWindowSurface(pwindow);
            //SDL_Delay(1000/FPS);
            
        }
        

        SDL_UpdateWindowSurface(pwindow);
        SDL_Delay(1);
    }

  

    SDL_Quit();
    return 0;
}







